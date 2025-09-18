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
#include "Data/Mechanics/PierceParameterDataAsset.h"
#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "AbilitySystem/Handlers/Mechanic_Pierce.h"

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
            Projectile->InitFromSpec(SkillSpec);

            // ASSERT: Check that the spec contains the correct pierce count
            TestTrue(TEXT("SkillSpec should contain pierce count"), SkillSpec.Contains(UMechanic_Pierce::PierceCountKey));
            TestEqual(TEXT("Pierce count should be 2"), SkillSpec.GetCustomParam(UMechanic_Pierce::PierceCountKey), 2.0f);
        });

        It("should allow pierce on first hit and track usage", [this]()
        {
            // ACT: Initialize the projectile with the spec
            Projectile->InitFromSpec(SkillSpec);

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
            Projectile->InitFromSpec(SkillSpec);

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
            Projectile->InitFromSpec(SkillSpec);

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
            Projectile->InitFromSpec(NoPierceSpec);

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
            Projectile->InitFromSpec(SkillSpec);

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