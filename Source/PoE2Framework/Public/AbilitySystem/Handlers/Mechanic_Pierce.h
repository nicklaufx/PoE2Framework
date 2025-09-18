// Copyright 2025 liu f cheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "Mechanic_Pierce.generated.h"

class APoE2ProjectileBase;

/**
 * @class UMechanic_Pierce
 * @brief A mechanic handler that implements pierce functionality for projectiles.
 * Uses a static map to track pierce usage per projectile instance.
 */
UCLASS(BlueprintType, Blueprintable)
class POE2FRAMEWORK_API UMechanic_Pierce : public UObject, public IMechanicHandler
{
	GENERATED_BODY()

public:
	// Defines the parameter key for the number of pierces.
	static const FName PierceCountKey;

	// IMechanicHandler interface
	virtual EHitHandlerResult OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec) override;

private:
	// Static map to track how many pierces each projectile has used
	// Using TWeakObjectPtr to avoid holding strong references to projectiles
	static TMap<TWeakObjectPtr<AActor>, int32> ProjectilePierceUsage;
};