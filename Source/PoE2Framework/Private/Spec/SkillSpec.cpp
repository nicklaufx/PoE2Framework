#include "Spec/SkillSpec.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"
#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "AbilitySystem/Actors/PoE2AreaEffectBase.h"
#include "AbilitySystem/Actors/PoE2MinionBase.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"

bool FSkillSpec::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    bOutSuccess = true;
    static constexpr int32 MaxArrayElements = 64;

    // 序列化版本号
    uint8 Version = SKILLSPEC_VERSION;
    Ar << Version;
    
    if (Ar.IsLoading() && Version != SKILLSPEC_VERSION)
    {
        // 版本不匹配，但可以考虑向后兼容
        // 当前简单处理：版本不匹配直接失败
        bOutSuccess = false;
        return false;
    }

    // 序列化基础数据
    Ar << SkillId;
    
    // 使用 UObject* 临时变量序列化类引用
    UObject* TempAbilityClass = AbilityClass;
    if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempAbilityClass))
    {
        bOutSuccess = false;
        return false;
    }
    
    // 验证 AbilityClass 类型并回填
    if (Ar.IsLoading())
    {
        UClass* LoadedClass = Cast<UClass>(TempAbilityClass);
        if (LoadedClass && LoadedClass->IsChildOf(UGameplayAbility::StaticClass()))
        {
            AbilityClass = LoadedClass;
        }
        else
        {
            AbilityClass = nullptr;
            if (TempAbilityClass != nullptr)
            {
                bOutSuccess = false;
                return false;
            }
        }
    }
    
    // 序列化承载体类引用
    UObject* TempProjectileClass = ProjectileClass;
    if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempProjectileClass))
    {
        bOutSuccess = false;
        return false;
    }
    if (Ar.IsLoading())
    {
        UClass* LoadedClass = Cast<UClass>(TempProjectileClass);
        if (LoadedClass && LoadedClass->IsChildOf(APoE2ProjectileBase::StaticClass()))
        {
            ProjectileClass = LoadedClass;
        }
        else
        {
            ProjectileClass = nullptr;
            if (TempProjectileClass != nullptr)
            {
                bOutSuccess = false;
                return false;
            }
        }
    }
    
    UObject* TempAreaClass = AreaClass;
    if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempAreaClass))
    {
        bOutSuccess = false;
        return false;
    }
    if (Ar.IsLoading())
    {
        UClass* LoadedClass = Cast<UClass>(TempAreaClass);
        if (LoadedClass && LoadedClass->IsChildOf(APoE2AreaEffectBase::StaticClass()))
        {
            AreaClass = LoadedClass;
        }
        else
        {
            AreaClass = nullptr;
            if (TempAreaClass != nullptr)
            {
                bOutSuccess = false;
                return false;
            }
        }
    }
    
    UObject* TempSummonClass = SummonClass;
    if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempSummonClass))
    {
        bOutSuccess = false;
        return false;
    }
    if (Ar.IsLoading())
    {
        UClass* LoadedClass = Cast<UClass>(TempSummonClass);
        if (LoadedClass && LoadedClass->IsChildOf(APoE2MinionBase::StaticClass()))
        {
            SummonClass = LoadedClass;
        }
        else
        {
            SummonClass = nullptr;
            if (TempSummonClass != nullptr)
            {
                bOutSuccess = false;
                return false;
            }
        }
    }

    // 序列化数值
    Ar << FinalDamage;
    Ar << Cooldown;
    Ar << ResourceCost;
    Ar << CastTime;
    Ar << AreaRadius;
    Ar << ProjectileSpeed;
    Ar << MaxRange;
    Ar << Lifetime;
    Ar << SummonCount;
    
    // 验证 SummonCount 上限
    if (Ar.IsLoading() && (SummonCount < 1 || SummonCount > 10))
    {
        SummonCount = FMath::Clamp(SummonCount, 1, 10);
    }

    // 序列化标签
    if (!SkillTags.NetSerialize(Ar, Map, bOutSuccess) || !bOutSuccess)
    {
        bOutSuccess = false;
        return false;
    }

    // 序列化效果数组
    uint32 EffectsNum = AppliedEffects.Num();
    
    // 在保存时也进行 clamp，防止超限数据导致复制失败
    if (!Ar.IsLoading())
    {
        EffectsNum = FMath::Min(EffectsNum, static_cast<uint32>(MaxArrayElements));
    }
    
    Ar.SerializeIntPacked(EffectsNum);
    
    if (EffectsNum > MaxArrayElements)
    {
        bOutSuccess = false;
        return false;
    }
    
    if (Ar.IsLoading())
    {
        AppliedEffects.SetNum(EffectsNum);
    }
    
    for (uint32 i = 0; i < EffectsNum; i++)
    {
        UObject* TempEffectClass = AppliedEffects[i];
        if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempEffectClass))
        {
            bOutSuccess = false;
            return false;
        }
        
        if (Ar.IsLoading())
        {
            UClass* LoadedClass = Cast<UClass>(TempEffectClass);
            if (LoadedClass && LoadedClass->IsChildOf(UGameplayEffect::StaticClass()))
            {
                AppliedEffects[i] = LoadedClass;
            }
            else
            {
                AppliedEffects[i] = nullptr;
                if (TempEffectClass != nullptr)
                {
                    bOutSuccess = false;
                    return false;
                }
            }
        }
    }

    // 序列化机制处理器数组
    uint32 HandlersNum = MechanicHandlers.Num();
    
    // 在保存时也进行 clamp
    if (!Ar.IsLoading())
    {
        HandlersNum = FMath::Min(HandlersNum, static_cast<uint32>(MaxArrayElements));
    }
    
    Ar.SerializeIntPacked(HandlersNum);
    
    if (HandlersNum > MaxArrayElements)
    {
        bOutSuccess = false;
        return false;
    }
    
    if (Ar.IsLoading())
    {
        MechanicHandlers.SetNum(HandlersNum);
    }
    
    for (uint32 i = 0; i < HandlersNum; i++)
    {
        UObject* TempHandlerClass = MechanicHandlers[i];
        if (!Map->SerializeObject(Ar, UClass::StaticClass(), TempHandlerClass))
        {
            bOutSuccess = false;
            return false;
        }
        
        if (Ar.IsLoading())
        {
            UClass* LoadedClass = Cast<UClass>(TempHandlerClass);
            if (LoadedClass && LoadedClass->ImplementsInterface(UMechanicHandler::StaticClass()))
            {
                MechanicHandlers[i] = LoadedClass;
            }
            else
            {
                MechanicHandlers[i] = nullptr;
                if (TempHandlerClass != nullptr)
                {
                    bOutSuccess = false;
                    return false;
                }
            }
        }
    }

    // 序列化自定义参数（按 Key 排序确保一致性）
    uint32 ParamsNum = CustomParams.Num();
    
    // 在保存时也进行 clamp
    if (!Ar.IsLoading())
    {
        ParamsNum = FMath::Min(ParamsNum, static_cast<uint32>(MaxArrayElements));
    }
    
    Ar.SerializeIntPacked(ParamsNum);
    
    if (ParamsNum > MaxArrayElements)
    {
        bOutSuccess = false;
        return false;
    }
    
    if (Ar.IsLoading())
    {
        CustomParams.Empty();
        for (uint32 i = 0; i < ParamsNum; i++)
        {
            FName Key;
            float Value;
            Ar << Key;
            Ar << Value;
            CustomParams.Add(FCustomParam(Key, Value));
        }
    }
    else
    {
        // 按 Key 排序确保网络一致性（使用 LexicalLess）
        TArray<TPair<FName, float>> SortedParams;
        for (const FCustomParam& Param : CustomParams)
        {
            SortedParams.Add(TPair<FName, float>(Param.Key, Param.Value));
        }
        SortedParams.Sort([](const TPair<FName, float>& A, const TPair<FName, float>& B)
        {
            return A.Key.ToString().Compare(B.Key.ToString(), ESearchCase::CaseSensitive) < 0;
        });
        
        for (const auto& Pair : SortedParams)
        {
            FName Key = Pair.Key;
            float Value = Pair.Value;
            Ar << Key;
            Ar << Value;
        }
    }

    return bOutSuccess;
}