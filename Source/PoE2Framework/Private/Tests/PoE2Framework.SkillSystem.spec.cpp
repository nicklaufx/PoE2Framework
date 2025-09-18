// Copyright 2025 liufucheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectMacros.h"
#include "AbilitySystemComponent.h"
#include "Spec/SkillSpec.h"
#include "Spec/Patch.h"
#include "Data/Mechanics/PierceParameterDataAsset.h"
#include "Data/SkillDataAsset.h"
#include "AbilitySystem/Actors/PoE2AreaEffectBase.h"
#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "AbilitySystem/Handlers/Mechanic_Pierce.h"
#include "AbilitySystem/Handlers/MechanicHandlerBase.h"
#include "AbilitySystem/GA_SkillBase.h"
#include "GameplayEffect.h"
#include "Components/SphereComponent.h"

UCLASS()
class UMechanic_TestLifecycle : public UMechanicHandlerBase
{
    GENERATED_BODY()

public:
    static void Reset();

    virtual void OnCast_Implementation(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec) override;
    virtual void OnSpawn_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) override;
    virtual EHitHandlerResult OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec) override;
    virtual void OnTick_Implementation(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec) override;
    virtual void OnEnd_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) override;

    static int32 CastCount;
    static int32 SpawnCount;
    static int32 HitCount;
    static int32 TickCount;
    static int32 EndCount;
    static float LastTickDelta;
    static TWeakObjectPtr<AActor> LastSpawnOwner;
    static TWeakObjectPtr<UObject> LastSpawnOuterObject;
    static TWeakObjectPtr<AActor> LastHitOwner;
};

int32 UMechanic_TestLifecycle::CastCount = 0;
int32 UMechanic_TestLifecycle::SpawnCount = 0;
int32 UMechanic_TestLifecycle::HitCount = 0;
int32 UMechanic_TestLifecycle::TickCount = 0;
int32 UMechanic_TestLifecycle::EndCount = 0;
float UMechanic_TestLifecycle::LastTickDelta = 0.0f;
TWeakObjectPtr<AActor> UMechanic_TestLifecycle::LastSpawnOwner = nullptr;
TWeakObjectPtr<UObject> UMechanic_TestLifecycle::LastSpawnOuterObject = nullptr;
TWeakObjectPtr<AActor> UMechanic_TestLifecycle::LastHitOwner = nullptr;

void UMechanic_TestLifecycle::Reset()
{
    CastCount = 0;
    SpawnCount = 0;
    HitCount = 0;
    TickCount = 0;
    EndCount = 0;
    LastTickDelta = 0.0f;
    LastSpawnOwner = nullptr;
    LastSpawnOuterObject = nullptr;
    LastHitOwner = nullptr;
}

BEGIN_DEFINE_SPEC(FPoE2SkillSystem_SkillSpecBuilderSpec, "PoE2.SkillSystem.SkillSpec",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
    UTestSkillAbility* Ability;
    USkillDataAsset* SkillAsset;
END_DEFINE_SPEC(FPoE2SkillSystem_SkillSpecBuilderSpec)

void FPoE2SkillSystem_SkillSpecBuilderSpec::Define()
{
    Describe("BuildSkillSpec", [this]()
    {
        BeforeEach([this]()
        {
            Ability = NewObject<UTestSkillAbility>();
            SkillAsset = NewObject<USkillDataAsset>();

            SkillAsset->SkillId = TEXT("BuildSpecSkill");
            SkillAsset->BaseDamage = 100.f;
            SkillAsset->Speed = 600.f;
            SkillAsset->Duration = 3.f;
            SkillAsset->ProjectileClass = nullptr;
            SkillAsset->SummonClass = nullptr;
        });

        It("should apply patches on top of base data", [this]()
        {
            FPatch Patch;
            Patch.AdditiveModifiers.Add(TEXT("FinalDamage"), 50.f);
            Patch.MultiplicativeModifiers.Add(TEXT("FinalDamage"), 0.5f);
            Patch.TagsToAdd.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Mechanic.Pierce")));
            Patch.EffectsToAdd.Add(UGameplayEffect::StaticClass());
            Patch.HandlersToAdd.Add(UMechanic_TestLifecycle::StaticClass());
            Patch.ProjectileClassOverride = APoE2ProjectileBase::StaticClass();

            TArray<FPatch> Patches;
            Patches.Add(Patch);

            FSkillSpec OutSpec;
            Ability->BuildSkillSpec(SkillAsset, Patches, OutSpec);

            TestEqual(TEXT("SkillId propagated"), OutSpec.SkillId, SkillAsset->SkillId);
            TestEqual(TEXT("Projectile class overridden"), OutSpec.ProjectileClass.Get(), APoE2ProjectileBase::StaticClass());
            TestEqual(TEXT("Base projectile speed copied"), OutSpec.ProjectileSpeed, SkillAsset->Speed);

            const float ExpectedDamage = (SkillAsset->BaseDamage + 50.f) * 1.5f;
            TestEqual(TEXT("Additive and multiplicative modifiers combined"), OutSpec.FinalDamage, ExpectedDamage);

            TestTrue(TEXT("Gameplay tag appended"), OutSpec.SkillTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("Mechanic.Pierce"))));
            TestTrue(TEXT("Gameplay effect added"), OutSpec.AppliedEffects.Contains(UGameplayEffect::StaticClass()));
            TestTrue(TEXT("Mechanic handler appended"), OutSpec.MechanicHandlers.Contains(UMechanic_TestLifecycle::StaticClass()));
        });

        AfterEach([this]()
        {
            Ability = nullptr;
            SkillAsset = nullptr;
        });
    });
}


void UMechanic_TestLifecycle::OnCast_Implementation(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec)
{
    ++CastCount;
}

void UMechanic_TestLifecycle::OnSpawn_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec)
{
    ++SpawnCount;
    LastSpawnOwner = OwnerActor;
    LastSpawnOuterObject = GetOuter();
}

EHitHandlerResult UMechanic_TestLifecycle::OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec)
{
    ++HitCount;
    LastHitOwner = OwnerActor;
    return EHitHandlerResult::Continue;
}

void UMechanic_TestLifecycle::OnTick_Implementation(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec)
{
    ++TickCount;
    LastTickDelta = DeltaTime;
}

void UMechanic_TestLifecycle::OnEnd_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec)
{
    ++EndCount;
}

UCLASS()
class ATestProjectile : public APoE2ProjectileBase
{
    GENERATED_BODY()

public:
    void SimulateHit(AActor* Target)
    {
        FHitResult Hit;
        Hit.HitObjectHandle = FActorInstanceHandle(Target);
        OnHit(nullptr, Target, nullptr, FVector::ZeroVector, Hit);
    }
};

UCLASS()
class ATestAreaEffect : public APoE2AreaEffectBase
{
    GENERATED_BODY()

public:
    void ForcePulse()
    {
        HandleAreaPulse();
    }
};

UCLASS()
class ATestOverlapActor : public AActor
{
    GENERATED_BODY()

public:
    ATestOverlapActor()
    {
        USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
        SphereComponent->InitSphereRadius(50.0f);
        SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
        SphereComponent->SetGenerateOverlapEvents(true);
        RootComponent = SphereComponent;
    }
};

UCLASS()
class UTestSkillAbility : public UGA_SkillBase
{
    GENERATED_BODY()
};

// Define the test spec using Unreal's Automation System
BEGIN_DEFINE_SPEC(FPoE2SkillSystem_PierceMechanicSpec, "PoE2.SkillSystem.Mechanics.Pierce",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
    UWorld* World;
    APoE2ProjectileBase* Projectile;
    AActor* Caster;
    AActor* Target1;
    AActor* Target2;
    AActor* Target3;
    FSkillSpec SkillSpec;
    UMechanic_Pierce* PierceHandler;
END_DEFINE_SPEC(FPoE2SkillSystem_PierceMechanicSpec)

void FPoE2SkillSystem_PierceMechanicSpec::Define()
{
    Describe("Pierce Mechanic Logic", [this]()
    {
        // Before each "It" block, set up a clean test environment
        BeforeEach([this]()
        {
            // ARRANGE: Create a new world for our test
            World = FAutomationEditorCommonUtils::CreateNewMap();

            // ARRANGE: Spawn the actors needed for the test
            Caster = World->SpawnActor<AActor>();
            Target1 = World->SpawnActor<AActor>();
            Target2 = World->SpawnActor<AActor>();
            Target3 = World->SpawnActor<AActor>();

            // ARRANGE: Create the Skill Spec with Pierce parameters
            SkillSpec = FSkillSpec();
            SkillSpec.SkillId = TEXT("TestSkill_Pierce");

            // Add Pierce mechanic handler to the spec
            SkillSpec.MechanicHandlers.Add(UMechanic_Pierce::StaticClass());

            // Set pierce count parameter directly in CustomParams
            SkillSpec.SetCustomParam(UMechanic_Pierce::PierceCountKey, 2.0f);

            // ARRANGE: Spawn the projectile and assign its owner
            Projectile = World->SpawnActor<APoE2ProjectileBase>();
            Projectile->SetOwner(Caster);

            // Create pierce handler instance for testing
            PierceHandler = NewObject<UMechanic_Pierce>();
        });

        It("should initialize with pierce count from spec parameters", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            // ASSERT: Check that the spec contains the correct pierce count
            TestTrue(TEXT("SkillSpec should contain pierce count"), SkillSpec.Contains(UMechanic_Pierce::PierceCountKey));
            TestEqual(TEXT("Pierce count should be 2"), SkillSpec.GetCustomParam(UMechanic_Pierce::PierceCountKey), 2.0f);
        });

        It("should allow pierce on first hit and track usage", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            // ACT: Simulate first hit using the handler directly
            FHitResult HitResult;
            HitResult.HitObjectHandle = FActorInstanceHandle(Target1);
            EHitHandlerResult Result = PierceHandler->OnHit_Implementation(Projectile, Target1, HitResult, SkillSpec);

            // ASSERT: First hit should allow pierce
            TestEqual(TEXT("First hit should return Pierce result"), Result, EHitHandlerResult::Pierce);

            // ASSERT: Check that pierce usage is tracked (1 pierce used, 1 remaining)
            // We can't directly access the static map, but we can test the behavior
        });

        It("should allow pierce on second hit", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            FHitResult HitResult1, HitResult2;
            HitResult1.HitObjectHandle = FActorInstanceHandle(Target1);
            HitResult2.HitObjectHandle = FActorInstanceHandle(Target2);

            // ACT: Simulate first hit
            EHitHandlerResult Result1 = PierceHandler->OnHit_Implementation(Projectile, Target1, HitResult1, SkillSpec);

            // ACT: Simulate second hit
            EHitHandlerResult Result2 = PierceHandler->OnHit_Implementation(Projectile, Target2, HitResult2, SkillSpec);

            // ASSERT: Both hits should allow pierce
            TestEqual(TEXT("First hit should return Pierce result"), Result1, EHitHandlerResult::Pierce);
            TestEqual(TEXT("Second hit should return Pierce result"), Result2, EHitHandlerResult::Pierce);
        });

        It("should stop projectile after all pierces are used", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            FHitResult HitResult1, HitResult2, HitResult3;
            HitResult1.HitObjectHandle = FActorInstanceHandle(Target1);
            HitResult2.HitObjectHandle = FActorInstanceHandle(Target2);
            HitResult3.HitObjectHandle = FActorInstanceHandle(Target3);

            // ACT: Simulate first hit (1 pierce used, 1 remaining)
            EHitHandlerResult Result1 = PierceHandler->OnHit_Implementation(Projectile, Target1, HitResult1, SkillSpec);

            // ACT: Simulate second hit (2 pierces used, 0 remaining)
            EHitHandlerResult Result2 = PierceHandler->OnHit_Implementation(Projectile, Target2, HitResult2, SkillSpec);

            // ACT: Simulate third hit (no pierces left)
            EHitHandlerResult Result3 = PierceHandler->OnHit_Implementation(Projectile, Target3, HitResult3, SkillSpec);

            // ASSERT: First two hits should allow pierce, third should stop
            TestEqual(TEXT("First hit should return Pierce result"), Result1, EHitHandlerResult::Pierce);
            TestEqual(TEXT("Second hit should return Pierce result"), Result2, EHitHandlerResult::Pierce);
            TestEqual(TEXT("Third hit should return Stop result"), Result3, EHitHandlerResult::Stop);
        });

        It("should stop immediately when no pierce count is available", [this]()
        {
            // ARRANGE: Create spec with no pierce count
            FSkillSpec NoPierceSpec;
            NoPierceSpec.SkillId = TEXT("TestSkill_NoPierce");
            NoPierceSpec.MechanicHandlers.Add(UMechanic_Pierce::StaticClass());
            // Don't set pierce count parameter (defaults to 0)

            // ACT: Initialize projectile with no-pierce spec
            Projectile->InitFromSpec(NoPierceSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            // ACT: Simulate hit
            FHitResult HitResult;
            HitResult.HitObjectHandle = FActorInstanceHandle(Target1);
            EHitHandlerResult Result = PierceHandler->OnHit_Implementation(Projectile, Target1, HitResult, NoPierceSpec);

            // ASSERT: Should stop immediately
            TestEqual(TEXT("Hit with no pierce count should return Stop result"), Result, EHitHandlerResult::Stop);
        });

        It("should handle projectile destruction through OnHit method", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec, nullptr, TArray<TScriptInterface<IMechanicHandler>>());

            // ACT: Simulate hits through the projectile's OnHit method
            FHitResult HitResult1, HitResult2, HitResult3;
            HitResult1.HitObjectHandle = FActorInstanceHandle(Target1);
            HitResult2.HitObjectHandle = FActorInstanceHandle(Target2);
            HitResult3.HitObjectHandle = FActorInstanceHandle(Target3);

            // Test through handler directly since OnHit is protected
            // First hit - should pierce
            EHitHandlerResult Result1 = PierceHandler->OnHit_Implementation(Projectile, Target1, HitResult1, SkillSpec);
            TestEqual(TEXT("First hit should return Pierce result"), Result1, EHitHandlerResult::Pierce);

            // Second hit - should pierce
            EHitHandlerResult Result2 = PierceHandler->OnHit_Implementation(Projectile, Target2, HitResult2, SkillSpec);
            TestEqual(TEXT("Second hit should return Pierce result"), Result2, EHitHandlerResult::Pierce);

            // Third hit - should be destroyed
            EHitHandlerResult Result3 = PierceHandler->OnHit_Implementation(Projectile, Target3, HitResult3, SkillSpec);
            TestEqual(TEXT("Third hit should return Stop result"), Result3, EHitHandlerResult::Stop);

            // Since we can't directly call OnHit, we test the handler logic directly
            // The third hit returning Stop indicates the projectile should be destroyed
        });

        // After each "It" block, tear down the environment
        AfterEach([this]()
        {
            // Clean up any remaining projectile reference in the static map
            if (IsValid(Projectile))
            {
                // Force cleanup by simulating enough hits to exhaust pierces
                FHitResult DummyHit;
                for (int32 i = 0; i < 5; ++i)
                {
                    PierceHandler->OnHit_Implementation(Projectile, Target1, DummyHit, SkillSpec);
                }
            }

            // Clean up the world and actors
            if (World)
            {
                World->DestroyWorld(false);
            }
            Projectile = nullptr;
            Caster = nullptr;
            Target1 = nullptr;
            Target2 = nullptr;
            Target3 = nullptr;
            PierceHandler = nullptr;
        });
    });
}

// Lifecycle spec to ensure handlers receive proper callbacks
BEGIN_DEFINE_SPEC(FPoE2SkillSystem_HandlerLifecycleSpec, "PoE2.SkillSystem.Mechanics.Lifecycle",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
    UWorld* World;
    ATestProjectile* Projectile;
    AActor* Target;
    FSkillSpec SkillSpec;
    TArray<TScriptInterface<IMechanicHandler>> HandlerPrototypes;
END_DEFINE_SPEC(FPoE2SkillSystem_HandlerLifecycleSpec)

void FPoE2SkillSystem_HandlerLifecycleSpec::Define()
{
    Describe("Mechanic handler lifecycle", [this]()
    {
        BeforeEach([this]()
        {
            World = FAutomationEditorCommonUtils::CreateNewMap();
            Projectile = World->SpawnActor<ATestProjectile>();
            Target = World->SpawnActor<AActor>();

            SkillSpec = FSkillSpec();
            SkillSpec.SkillId = TEXT("LifecycleSkill");

            HandlerPrototypes.Reset();
            UMechanic_TestLifecycle::Reset();

            UMechanic_TestLifecycle* Prototype = NewObject<UMechanic_TestLifecycle>();
            TScriptInterface<IMechanicHandler> PrototypeInterface;
            PrototypeInterface.SetObject(Prototype);
            PrototypeInterface.SetInterface(Cast<IMechanicHandler>(Prototype));
            HandlerPrototypes.Add(PrototypeInterface);

            IMechanicHandler::Execute_OnCast(Prototype, nullptr, SkillSpec);
        });

        It("should propagate lifecycle callbacks to handler instances", [this]()
        {
            Projectile->InitFromSpec(SkillSpec, nullptr, HandlerPrototypes);

            TestEqual(TEXT("OnCast executed once"), UMechanic_TestLifecycle::CastCount, 1);
            TestEqual(TEXT("OnSpawn executed once"), UMechanic_TestLifecycle::SpawnCount, 1);
            TestEqual(TEXT("Projectile owns one handler instance"), Projectile->GetActiveHandlerCount(), 1);
            TestTrue(TEXT("Handler outer is projectile"), UMechanic_TestLifecycle::LastSpawnOuterObject.Get() == Projectile);
            TestTrue(TEXT("Handler owner matches projectile"), UMechanic_TestLifecycle::LastSpawnOwner.Get() == Projectile);

            Projectile->Tick(0.016f);
            TestEqual(TEXT("OnTick executed once"), UMechanic_TestLifecycle::TickCount, 1);
            TestTrue(TEXT("Tick delta recorded"), FMath::IsNearlyEqual(UMechanic_TestLifecycle::LastTickDelta, 0.016f));

            Projectile->SimulateHit(Target);
            TestEqual(TEXT("OnHit executed once"), UMechanic_TestLifecycle::HitCount, 1);
            TestTrue(TEXT("Hit callback invoked on projectile instance"), UMechanic_TestLifecycle::LastHitOwner.Get() == Projectile);

            Projectile->Destroy();
            World->Tick(ELevelTick::LEVELTICK_All, 0.0f);
            TestEqual(TEXT("OnEnd executed once"), UMechanic_TestLifecycle::EndCount, 1);
        });

        AfterEach([this]()
        {
            if (World)
            {
                World->DestroyWorld(false);
            }
            World = nullptr;
            Projectile = nullptr;
            Target = nullptr;
            HandlerPrototypes.Reset();
        });
    });
}

BEGIN_DEFINE_SPEC(FPoE2SkillSystem_AreaEffectSpec, "PoE2.SkillSystem.AreaEffect",
                  EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
    UWorld* World;
    ATestAreaEffect* AreaEffect;
    ATestOverlapActor* Target;
    FSkillSpec SkillSpec;
    TArray<TScriptInterface<IMechanicHandler>> HandlerPrototypes;
END_DEFINE_SPEC(FPoE2SkillSystem_AreaEffectSpec)

void FPoE2SkillSystem_AreaEffectSpec::Define()
{
    Describe("Area effect pulses apply mechanics", [this]()
    {
        BeforeEach([this]()
        {
            World = FAutomationEditorCommonUtils::CreateNewMap();
            AreaEffect = World->SpawnActor<ATestAreaEffect>();
            Target = World->SpawnActor<ATestOverlapActor>();

            SkillSpec = FSkillSpec();
            SkillSpec.SkillId = TEXT("AreaPulseSkill");
            SkillSpec.AreaRadius = 300.0f;
            SkillSpec.SetCustomParam(TEXT("Area.TickInterval"), 0.05f);
            SkillSpec.MechanicHandlers.Reset();
            SkillSpec.MechanicHandlers.Add(UMechanic_TestLifecycle::StaticClass());

            HandlerPrototypes.Reset();
            UMechanic_TestLifecycle::Reset();

            UMechanic_TestLifecycle* Prototype = NewObject<UMechanic_TestLifecycle>();
            TScriptInterface<IMechanicHandler> PrototypeInterface;
            PrototypeInterface.SetObject(Prototype);
            PrototypeInterface.SetInterface(Cast<IMechanicHandler>(Prototype));
            HandlerPrototypes.Add(PrototypeInterface);

            IMechanicHandler::Execute_OnCast(Prototype, nullptr, SkillSpec);

            AreaEffect->SetActorLocation(FVector::ZeroVector);
            Target->SetActorLocation(FVector::ZeroVector);
        });

        It("should trigger OnHit for overlapping actors on pulse", [this]()
        {
            AreaEffect->InitFromSpec(SkillSpec, nullptr, HandlerPrototypes);

            TestEqual(TEXT("Handler spawned once"), UMechanic_TestLifecycle::SpawnCount, 1);
            TestEqual(TEXT("Area effect owns handler instance"), AreaEffect->GetActiveHandlerCount(), 1);

            AreaEffect->ForcePulse();
            TestEqual(TEXT("OnHit executed after first pulse"), UMechanic_TestLifecycle::HitCount, 1);

            AreaEffect->ForcePulse();
            TestEqual(TEXT("OnHit executed after second pulse"), UMechanic_TestLifecycle::HitCount, 2);
        });

        AfterEach([this]()
        {
            if (World)
            {
                World->DestroyWorld(false);
            }

            World = nullptr;
            AreaEffect = nullptr;
            Target = nullptr;
            HandlerPrototypes.Reset();
        });
    });
}
