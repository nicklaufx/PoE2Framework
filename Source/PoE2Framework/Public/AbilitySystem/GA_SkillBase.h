// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_SkillBase.generated.h"

// Forward Declarations
class USkillDataAsset;
class APoE2ProjectileBase;
class APoE2AreaEffectBase;
class APoE2MinionBase;
struct FSkillSpec;
struct FPatch;
struct FPoE2CueParams;

/**
 * Base Gameplay Ability for all skills in PoE2Framework.
 * This class is a STATELESS process executor. Its primary job is to:
 * 1. Receive skill data via GameplayEventData.
 * 2. Perform checks (cost, cooldown).
 * 3. Play casting animations and cues.
 * 4. Spawn the appropriate actor (Projectile, Area, Minion) that will carry the skill's logic.
 * 5. Pass the final FSkillSpec to the spawned actor.
 *
 * It SHOULD NOT store any stateful information like FSkillSpec as member variables.
 */
UCLASS(Abstract)
class POE2FRAMEWORK_API UGA_SkillBase : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_SkillBase();

    /**
     * The main entry point for skill execution.
     * Expects TriggerEventData to contain the necessary context, such as the SkillDataAsset.
     */
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

protected:
    //================================================================================
    // Helper Functions
    //================================================================================

    /**
     * Constructs the final FSkillSpec from a base DataAsset and a series of modifications (Patches).
     * This is a pure function.
     * @param SkillDA The source Skill Data Asset.
     * @param Patches An array of patches from support gems, items, etc.
     * @param OutSpec The resulting final skill specification.
     */
    UFUNCTION(BlueprintPure, Category = "Skill")
    void BuildSkillSpec(const USkillDataAsset* SkillDA, const TArray<FPatch>& Patches, FSkillSpec& OutSpec) const;

    /**
     * Spawns a projectile actor.
     * @param SkillSpec The final skill spec containing spawn info (e.g., ProjectileClass).
     * @return The spawned projectile actor, ready for initialization.
     */
    UFUNCTION(BlueprintCallable, Category = "Skill|Spawning")
    APoE2ProjectileBase* SpawnProjectile(const FSkillSpec& SkillSpec);

    /**
     * Spawns an area effect actor.
     * @param SkillSpec The final skill spec containing spawn info (e.g., AreaClass).
     * @return The spawned area effect actor, ready for initialization.
     */
    UFUNCTION(BlueprintCallable, Category = "Skill|Spawning")
    APoE2AreaEffectBase* SpawnArea(const FSkillSpec& SkillSpec);

    /**
     * Spawns a minion actor.
     * @param SkillSpec The final skill spec containing spawn info (e.g., SummonClass).
     * @return The spawned minion actor, ready for initialization.
     */
    UFUNCTION(BlueprintCallable, Category = "Skill|Spawning")
    APoE2MinionBase* SpawnSummon(const FSkillSpec& SkillSpec);

    /**
     * Spawns and initializes all skill actors (Projectiles, Areas, Minions).
     * This is the core C++ logic that can be called by Blueprints.
     */
    UFUNCTION(BlueprintCallable, Category = "Skill|Execution")
    void ExecuteSkillEffects(const FSkillSpec& LocalSkillSpec);

public:
    /** Override to clean up ability state when ending. */
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

    //================================================================================
    // Blueprint Implementable Events
    //================================================================================

    /**
     * Blueprint-implementable event that runs after the skill spec is built but before any other logic.
     * Use this for any pre-flight logic or validation.
     * @param SkillSpec The final, constructed skill specification.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Skill", meta = (DisplayName = "OnSkillSpecReady"))
    void K2_OnSkillSpecReady(const FSkillSpec& SkillSpec);
    
    /**
     * Blueprint-implementable event for spawning and initializing the skill's actor carrier.
     * This is where the core "action" of the ability happens.
     * @param SkillSpec The final, constructed skill specification.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Skill", meta = (DisplayName = "PerformSpawn"))
    void K2_PerformSpawn(const FSkillSpec& SkillSpec);

protected:
    /** When true (default), ExecuteSkillEffects will be invoked automatically if no Blueprint overrides PerformSpawn. */
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    bool bAutoExecuteSkillEffects;

private:
    /** Animation callback for when the cast montage completes successfully. */
    UFUNCTION()
    void OnCastMontageCompleted();

    /** Animation callback for when the cast montage is interrupted. */
    UFUNCTION()
    void OnCastMontageInterrupted();
};