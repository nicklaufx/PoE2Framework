// Copyright 2025 liu f cheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "AbilitySystem/Handlers/Mechanic_Pierce.h"
#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "Spec/SkillSpec.h"
#include "Core/PoE2Log.h"

// Define the static key. The string literal now only exists in one place.
const FName UMechanic_Pierce::PierceCountKey = FName(TEXT("Mechanic.Pierce.Count"));

// Static map to track pierce usage per projectile instance
TMap<TWeakObjectPtr<AActor>, int32> UMechanic_Pierce::ProjectilePierceUsage;

EHitHandlerResult UMechanic_Pierce::OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec)
{
    APoE2ProjectileBase* Projectile = Cast<APoE2ProjectileBase>(OwnerActor);
    if (!Projectile)
    {
        UE_LOG(LogPoE2Framework, Warning, TEXT("Mechanic_Pierce: OwnerActor is not a projectile"));
        return EHitHandlerResult::Continue;
    }

    // Check for pierce count from the CustomParams using our safe, static key.
    int32 TotalPierceCount = 0;
    if (SkillSpec.Contains(UMechanic_Pierce::PierceCountKey))
    {
        TotalPierceCount = FMath::FloorToInt(SkillSpec[UMechanic_Pierce::PierceCountKey]);
    }

    // If no pierce available, stop
    if (TotalPierceCount <= 0)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("Mechanic_Pierce: No pierce count in SkillSpec, stopping projectile"));
        return EHitHandlerResult::Stop;
    }

    // Track how many times this projectile has pierced
    TWeakObjectPtr<AActor> ProjectilePtr = Projectile;
    int32 UsedPierces = ProjectilePierceUsage.FindRef(ProjectilePtr);

    if (UsedPierces < TotalPierceCount)
    {
        // Still has pierces available
        ProjectilePierceUsage.Add(ProjectilePtr, UsedPierces + 1);

        int32 RemainingPierces = TotalPierceCount - (UsedPierces + 1);
        UE_LOG(LogPoE2Framework, Log, TEXT("Mechanic_Pierce: Projectile pierced through target, remaining pierces: %d"), RemainingPierces);

        return EHitHandlerResult::Pierce;
    }

    // No more pierces available - clean up tracking
    ProjectilePierceUsage.Remove(ProjectilePtr);
    UE_LOG(LogPoE2Framework, Log, TEXT("Mechanic_Pierce: No pierce remaining, stopping projectile"));
    return EHitHandlerResult::Stop;
}