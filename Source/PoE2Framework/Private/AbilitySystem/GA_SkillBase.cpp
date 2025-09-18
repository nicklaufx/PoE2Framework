#include "AbilitySystem/GA_SkillBase.h"
#include "AbilitySystem/PoE2_AbilitySystemComponent.h" // 包含新组件的头文件
#include "Data/SkillDataAsset.h"
#include "Spec/Patch.h"
#include "Spec/SkillSpec.h"
#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "AbilitySystem/Actors/PoE2AreaEffectBase.h"
#include "AbilitySystem/Actors/PoE2MinionBase.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "AbilitySystem/Handlers/MechanicHandlerBase.h"
#include "CueSystem/PoE2CueManager.h"
#include "CueSystem/CueParams.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Core/PoE2Log.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "UObject/Class.h"

UGA_SkillBase::UGA_SkillBase()
    : bAutoExecuteSkillEffects(true)
{
}

void UGA_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 1. 获取数据源
    const USkillDataAsset* SkillDA = nullptr;
    TArray<FPatch> Patches; 
    
    // 多种方式提取 SkillDataAsset
    if (TriggerEventData)
    {
        // 方式1: 从 OptionalObject 获取
        if (TriggerEventData->OptionalObject && TriggerEventData->OptionalObject->IsA<USkillDataAsset>())
        {
            SkillDA = Cast<USkillDataAsset>(TriggerEventData->OptionalObject);
        }
        
        // 方式2: 从 TargetData 获取（如果需要的话）
        if (!SkillDA && TriggerEventData->TargetData.IsValid(0))
        {
            // TODO: 实现从 TargetData 中提取 SkillDataAsset 的逻辑
        }
    }
    
    // 方式3: 从 AbilitySpec 的 SourceObject 获取
    if (!SkillDA)
    {
        if(const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
        {
            SkillDA = Cast<USkillDataAsset>(Spec->SourceObject.Get());
        }
    }
    
    // 验证必要数据
    if (!SkillDA)
    {
        UE_LOG(LogPoE2Framework, Error, TEXT("UGA_SkillBase::ActivateAbility: No SkillDataAsset found. Ability: %s"), *GetClass()->GetName());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // ====================================================================
    // 核心修改在这里：填上之前 TODO 的部分
    // ====================================================================

    // 1. 从 ActorInfo 获取我们的自定义 AbilitySystemComponent
    UPoE2_AbilitySystemComponent* PoE2_ASC = Cast<UPoE2_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

    // 2. 如果成功获取，就调用核心函数来搜集 Patches
    if (PoE2_ASC)
    {
        Patches = PoE2_ASC->GetPatchesForSkill(SkillDA);
    }
    // ====================================================================
    
    // 2. 构建局部 SkillSpec
    FSkillSpec LocalSkillSpec;
    BuildSkillSpec(SkillDA, Patches, LocalSkillSpec);
    
    // 验证 SkillSpec 构建结果
    if (LocalSkillSpec.SkillId == NAME_None)
    {
        UE_LOG(LogPoE2Framework, Error, TEXT("UGA_SkillBase::ActivateAbility: Failed to build SkillSpec from DataAsset: %s"), *SkillDA->GetName());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // 3. 调用蓝图事件，允许蓝图层进行预处理
    K2_OnSkillSpecReady(LocalSkillSpec);
    
    // 4. 检查资源与冷却 (GAS 标准流程)
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogPoE2Framework, Warning, TEXT("UGA_SkillBase::ActivateAbility: CommitAbility failed for skill: %s"), *LocalSkillSpec.SkillId.ToString());
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // 5. 播放施法表现
    // 5.1 播放施法动画
    if (SkillDA->CastMontage)
    {
        UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, 
            NAME_None, 
            SkillDA->CastMontage, 
            1.0f, // PlayRate
            NAME_None, // StartSection
            true, // bStopWhenAbilityEnds
            1.0f, // AnimRootMotionTranslationScale
            0.0f  // StartTimeSeconds
        );
        
        if (MontageTask)
        {
            // 绑定动画完成回调
            MontageTask->OnCompleted.AddDynamic(this, &UGA_SkillBase::OnCastMontageCompleted);
            MontageTask->OnInterrupted.AddDynamic(this, &UGA_SkillBase::OnCastMontageInterrupted);
            MontageTask->ReadyForActivation();
        }
    }
    
    // 5.2 播放施法 Cue
    if (SkillDA->CueOnCast.IsValid())
    {
        FGameplayCueParameters LocalCueParams;
        AActor* Avatar = GetAvatarActorFromActorInfo();
        LocalCueParams.Location = Avatar ? Avatar->GetActorLocation() : FVector::ZeroVector;
        LocalCueParams.RawMagnitude = LocalSkillSpec.FinalDamage;

        UPoE2CueManager::PlayLocalCue(Avatar, SkillDA->CueOnCast, LocalCueParams);
    }
    
    // 6. 触发蓝图的 PerformSpawn 钩子，如果蓝图没有覆写，则自动执行默认逻辑
    const bool bBlueprintOverridesPerformSpawn = GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UGA_SkillBase, K2_PerformSpawn));
    K2_PerformSpawn(LocalSkillSpec);

    if (bAutoExecuteSkillEffects && !bBlueprintOverridesPerformSpawn)
    {
        ExecuteSkillEffects(LocalSkillSpec);
    }
}

void UGA_SkillBase::ExecuteSkillEffects(const FSkillSpec& LocalSkillSpec)
{
    TArray<TScriptInterface<IMechanicHandler>> HandlerInstances;
    HandlerInstances.Reserve(LocalSkillSpec.MechanicHandlers.Num());

    UAbilitySystemComponent* CasterASC = GetAbilitySystemComponentFromActorInfo();

    for (const TSubclassOf<UObject>& HandlerClass : LocalSkillSpec.MechanicHandlers)
    {
        if (!HandlerClass)
        {
            continue;
        }

        UMechanicHandlerBase* NewHandler = NewObject<UMechanicHandlerBase>(this, HandlerClass);
        if (!NewHandler)
        {
            continue;
        }

        TScriptInterface<IMechanicHandler> HandlerInterface;
        HandlerInterface.SetObject(NewHandler);
        HandlerInterface.SetInterface(Cast<IMechanicHandler>(NewHandler));
        if (!HandlerInterface.GetInterface())
        {
            UE_LOG(LogPoE2Framework, Warning, TEXT("UGA_SkillBase::ExecuteSkillEffects: Handler %s does not implement interface"), *HandlerClass->GetName());
            continue;
        }

        IMechanicHandler::Execute_OnCast(NewHandler, CasterASC, LocalSkillSpec);
        HandlerInstances.Add(HandlerInterface);
    }

    // 生成投掷物
    if (LocalSkillSpec.ProjectileClass)
    {
        if (APoE2ProjectileBase* Projectile = SpawnProjectile(LocalSkillSpec))
        {
            Projectile->InitFromSpec(LocalSkillSpec, CasterASC, HandlerInstances);
        }
    }

    // 生成区域效果
    if (LocalSkillSpec.AreaClass)
    {
        if (APoE2AreaEffectBase* AreaEffect = SpawnArea(LocalSkillSpec))
        {
            AreaEffect->InitFromSpec(LocalSkillSpec, CasterASC, HandlerInstances);
        }
    }

    // 生成召唤物
    if (LocalSkillSpec.SummonClass)
    {
        AActor* Avatar = GetAvatarActorFromActorInfo();
        if (Avatar)
        {
            const FVector RightVector = Avatar->GetActorRightVector();

            for (int32 Index = 0; Index < LocalSkillSpec.SummonCount; ++Index)
            {
                if (APoE2MinionBase* Summon = SpawnSummon(LocalSkillSpec))
                {
                    const float Spread = 100.f;
                    const float OffsetIndex = static_cast<float>(Index) - (static_cast<float>(LocalSkillSpec.SummonCount - 1) * 0.5f);
                    const FVector SpawnLocation = Avatar->GetActorLocation() + (RightVector * OffsetIndex * Spread);
                    Summon->SetActorLocation(SpawnLocation);
                    Summon->InitFromSpec(LocalSkillSpec, CasterASC, HandlerInstances);
                }
            }
        }
    }
}

void UGA_SkillBase::OnCastMontageCompleted()
{
    // 动画播放完成，技能执行结束
    UE_LOG(LogPoE2Framework, Log, TEXT("UGA_SkillBase::OnCastMontageCompleted: Cast animation completed"));
    
    // 可以在这里添加动画完成后的逻辑
    // 例如触发额外效果、结束能力等
}

void UGA_SkillBase::OnCastMontageInterrupted()
{
    // 动画被打断，可能需要取消技能
    UE_LOG(LogPoE2Framework, Warning, TEXT("UGA_SkillBase::OnCastMontageInterrupted: Cast animation was interrupted"));
    
    // 可以选择结束能力或者其他处理
    // EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_SkillBase::EndAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    // 由于重构为无状态设计，不再需要清理成员变量
    // 所有状态都由承载体（Projectile/Area/Minion）管理
    // 这里只需要调用父类的清理逻辑
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_SkillBase::BuildSkillSpec(const USkillDataAsset* SkillDA,
    const TArray<FPatch>& Patches,
    FSkillSpec& OutSpec) const
{
    if (!SkillDA)
    {
        return;
    }
    
    // 1. 从 SkillDA 读取基础数值
    OutSpec = SkillDA->CreateBaseSkillSpec();
    
    // 2. 应用所有 Patch 修改
    for (const FPatch& Patch : Patches)
    {
        UStruct* SkillSpecStruct = FSkillSpec::StaticStruct();

        // 应用加法修改
        for (const auto& Elem : Patch.AdditiveModifiers)
        {
            if (FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(SkillSpecStruct, Elem.Key))
            {
                float& CurrentValue = *FloatProp->ContainerPtrToValuePtr<float>(&OutSpec);
                CurrentValue += Elem.Value;
            }
        }

        // 应用乘法修改 ("Increased/Reduced")
        for (const auto& Elem : Patch.MultiplicativeModifiers)
        {
            if (FFloatProperty* FloatProp = FindFProperty<FFloatProperty>(SkillSpecStruct, Elem.Key))
            {
                float& CurrentValue = *FloatProp->ContainerPtrToValuePtr<float>(&OutSpec);
                CurrentValue *= (1.0f + Elem.Value);
            }
        }
        
        // 应用标签修改
        OutSpec.SkillTags.AppendTags(Patch.TagsToAdd);
        OutSpec.SkillTags.RemoveTags(Patch.TagsToRemove);
        
        // 添加效果
        OutSpec.AppliedEffects.Append(Patch.EffectsToAdd);
        
        // 添加机制处理器
        OutSpec.MechanicHandlers.Append(Patch.HandlersToAdd);
        
        // 应用投掷物类覆盖
        if (Patch.ProjectileClassOverride)
        {
            OutSpec.ProjectileClass = Patch.ProjectileClassOverride;
        }
    }
    
    // 3. OutSpec 现在包含了完整的合成结果
}

APoE2ProjectileBase* UGA_SkillBase::SpawnProjectile(const FSkillSpec& SkillSpec)
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!SkillSpec.ProjectileClass || !Avatar)
    {
        return nullptr;
    }
    
    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    FTransform SpawnTransform = Avatar->GetActorTransform();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    APoE2ProjectileBase* Projectile = World->SpawnActor<APoE2ProjectileBase>(
        SkillSpec.ProjectileClass,
        SpawnTransform,
        SpawnParams
    );
    
    return Projectile;
}

APoE2AreaEffectBase* UGA_SkillBase::SpawnArea(const FSkillSpec& SkillSpec)
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!SkillSpec.AreaClass || !Avatar)
    {
        return nullptr;
    }
    
    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    FTransform SpawnTransform = Avatar->GetActorTransform();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    APoE2AreaEffectBase* AreaEffect = World->SpawnActor<APoE2AreaEffectBase>(
        SkillSpec.AreaClass,
        SpawnTransform,
        SpawnParams
    );
    
    return AreaEffect;
}

APoE2MinionBase* UGA_SkillBase::SpawnSummon(const FSkillSpec& SkillSpec)
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!SkillSpec.SummonClass || !Avatar)
    {
        return nullptr;
    }
    
    UWorld* World = Avatar->GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    FTransform SpawnTransform = Avatar->GetActorTransform();
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Avatar;
    SpawnParams.Instigator = Cast<APawn>(Avatar);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    APoE2MinionBase* Summon = World->SpawnActor<APoE2MinionBase>(
        SkillSpec.SummonClass,
        SpawnTransform,
        SpawnParams
    );
    
    return Summon;
}

// ApplySkillEffects 和 TriggerMechanicHandlers 函数已移除
// 这些逻辑现在由承载体（Projectile/Area/Minion）在各自的事件中处理