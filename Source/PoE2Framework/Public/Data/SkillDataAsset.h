// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Data/ParameterDataAsset.h"
#include "SkillDataAsset.generated.h"

// Forward Declarations
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UMechanicHandler;
class APoE2ProjectileBase;
class APoE2MinionBase;
struct FSkillSpec;

/**
 * Defines a single skill in a data-driven way.
 * This asset contains all the base stats, effects, cues, and logic for a skill
 * before any modifications from support gems, items, or talents are applied.
 * It serves as the source data to create a FSkillSpec.
 */
UCLASS(BlueprintType, meta=(DisplayName="PoE2 Skill DataAsset"))
class POE2FRAMEWORK_API USkillDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /**
     * File Path: Plugins/PoE2Framework/Source/PoE2Framework/Public/Data/SkillDataAsset.h
     * Class Name: USkillDataAsset
     */
    USkillDataAsset();

    //~ Begin UPrimaryDataAsset Interface
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    //~ End UPrimaryDataAsset Interface

    /**
     * Creates a base FSkillSpec snapshot from this DataAsset.
     * This represents the initial state of a skill before any patches are applied.
     * @return A FSkillSpec struct populated with the base data from this asset.
     */
    virtual FSkillSpec CreateBaseSkillSpec() const;

public:
    //================================================================================
    // Fields
    //================================================================================

    // Category: Identity & Classification
    //--------------------------------------------------------------------------------
    
    /** The unique identifier for this skill. Should be unique across all skills. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FName SkillId;

    /** The name of the skill displayed in the UI. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;

    /** Tags that describe the skill (e.g., Skill.Fire, Skill.Projectile). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FGameplayTagContainer SkillTags;

    // Category: Core Logic & Bindings
    //--------------------------------------------------------------------------------

    /** The Gameplay Ability class that executes the skill's logic. Should be a subclass of GA_SkillBase. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    TSubclassOf<UGameplayAbility> AbilityClass;
    
    /** The animation montage to play when casting this skill. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
    TObjectPtr<UAnimMontage> CastMontage;
    
    // Category: Base Numerical Stats
    //--------------------------------------------------------------------------------

    /** The Gameplay Tag defining the type of damage (e.g., Damage.Type.Fire). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (Categories = "Damage"))
    FGameplayTag DamageType;

    /** The base damage value. This is typically passed to a GameplayEffect via SetByCallerMagnitude with the 'Data.Damage' tag. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float BaseDamage = 0.f;

    /** The Gameplay Effect class to use for applying damage. This should use an ExecutionCalculation like Exec_Damage. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float Cooldown = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta=(DisplayName="Resource Cost"))
    float Cost = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    float CastTime = 0.f;

    // Category: Spatial Properties
    //--------------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial")
    float Radius = 0.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial", meta=(DisplayName="Projectile Speed"))
    float Speed = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial")
    float MaxRange = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spatial", meta=(DisplayName="Lifetime / Duration"))
    float Duration = 0.f;
    
    // Category: Spawnable Actors
    //--------------------------------------------------------------------------------

    /** The projectile actor to spawn, if this is a projectile skill. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors")
    TSubclassOf<APoE2ProjectileBase> ProjectileClass;
    
    /** The minion actor to spawn, if this is a summoning skill. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors")
    TSubclassOf<APoE2MinionBase> SummonClass;
    
    /** The base number of minions to summon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actors")
    int32 SummonCount = 1;

    // Category: Visual & Audio Cues
    //--------------------------------------------------------------------------------

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues", meta=(Categories="Cue"))
    FGameplayTag CueOnCast;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues", meta=(Categories="Cue"))
    FGameplayTag CueOnProjectile;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues", meta=(Categories="Cue"))
    FGameplayTag CueOnImpact;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues", meta=(Categories="Cue"))
    FGameplayTag CueOnResidual;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cues", meta=(Categories="Cue"))
    FGameplayTag CueOnAura;

    // Category: Default Effects & Mechanics
    //--------------------------------------------------------------------------------

    /** List of Gameplay Effects to apply on hit by default. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults")
    TArray<TSubclassOf<UGameplayEffect>> DefaultEffects;

    /** List of default mechanic handlers for this skill (e.g., Pierce, Chain). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defaults", meta=(MustImplement="/Script/PoE2Framework.MechanicHandler"))
    TArray<TSubclassOf<UObject>> DefaultHandlers;

    /**
     * A list of data assets that define custom parameters for the skill's mechanics.
     * This provides a structured, UI-friendly way to configure complex mechanics.
     */
    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = "Defaults", meta=(DisplayName="Custom Mechanic Parameters"))
    TArray<TObjectPtr<UParameterDataAsset>> CustomMechanicParameters;
};