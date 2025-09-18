// Copyright 2025 liufucheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spec/SkillSpec.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "PoE2ProjectileBase.generated.h"

class UProjectileMovementComponent;
class UAbilitySystemComponent;

/**
 * @brief Base class for all projectiles in the game.
 * It handles movement, collision, and basic replication.
 */
UCLASS()
class POE2FRAMEWORK_API APoE2ProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	APoE2ProjectileBase();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
        /**
         * @brief Initializes the projectile from a skill specification.
         * Called on the server after the projectile is spawned.
         * @param InSpec The skill spec to initialize from.
         * @param InOwnerASC Ability system component that spawned the projectile.
         * @param HandlerPrototypes Mechanic handler instances cloned from the ability.
         */
        virtual void InitFromSpec(const FSkillSpec& InSpec, UAbilitySystemComponent* InOwnerASC, const TArray<TScriptInterface<IMechanicHandler>>& HandlerPrototypes);

        virtual void Tick(float DeltaSeconds) override;

protected:
        /**
         * @brief Server-only logic to handle projectile hitting another actor.
         * @param OtherActor The actor that was hit.
         * @param HitResult The hit result from the collision.
         */
        UFUNCTION()
        virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
        UFUNCTION(BlueprintPure, Category = "Projectile|Mechanics")
        int32 GetActiveHandlerCount() const;

public:
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
        TObjectPtr<UProjectileMovementComponent> MovementComponent;

	/** The Ability System Component of the owner of this projectile. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
        /** The skill spec that this projectile was created from. */
        UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Projectile")
        FSkillSpec CurrentSpec;

        /** Runtime mechanic handler instances bound to this projectile. */
        UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile")
        TArray<TScriptInterface<IMechanicHandler>> ActiveHandlers;

	// TODO:
	// 1. Replication Strategy: Determine if custom replication is needed for smoother movement, especially for networked games.
	//    Consider using a struct to pack frequently updated properties for more efficient replication.
	// 2. Interpolation Strategy: Implement client-side interpolation/prediction to reduce perceived latency.
	// 3. Hit Filtering: Implement logic to ensure a projectile doesn't hit the same target multiple times, or to filter hits based on alignment (enemy/ally).
};
