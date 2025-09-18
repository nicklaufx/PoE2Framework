// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Patch.generated.h"

// Forward Declarations
class UGameplayEffect;
class APoE2ProjectileBase;

/**
 * A data structure that defines a modification ("Patch") to a FSkillSpec.
 * This is the core of the Support Gem system. Each SupportDataAsset contains one of these
 * to describe how it alters a base skill.
 */
USTRUCT(BlueprintType)
struct POE2FRAMEWORK_API FPatch
{
    GENERATED_BODY()

public:
    //================================================================================
    // Fields
    //================================================================================

    // Category: Numerical Modifiers
    // Maps a target FName (matching a field in FSkillSpec) to a value.
    //--------------------------------------------------------------------------------

    /** 
     * Additive modifiers. E.g., {"FinalDamage", 10.f} adds 10 flat damage.
     * The FName key must match a float property name in FSkillSpec.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Numerical")
    TMap<FName, float> AdditiveModifiers;

    /**
     * Multiplicative modifiers ("Increased/Reduced"). E.g., {"ProjectileSpeed", 0.2f} increases speed by 20%.
     * The FName key must match a float property name in FSkillSpec.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Numerical")
    TMap<FName, float> MultiplicativeModifiers;

    // Category: Tag Modifiers
    //--------------------------------------------------------------------------------

    /** Gameplay Tags to add to the skill's spec. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Tags")
    FGameplayTagContainer TagsToAdd;

    /** Gameplay Tags to remove from the skill's spec. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Tags")
    FGameplayTagContainer TagsToRemove;

    // Category: Asset & Handler Additions
    //--------------------------------------------------------------------------------

    /** Gameplay Effects to add to the skill's application list. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Assets")
    TArray<TSubclassOf<UGameplayEffect>> EffectsToAdd;

    /** Mechanic Handlers to add to the skill's logic. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Assets", meta=(MustImplement="/Script/PoE2Framework.MechanicHandler"))
    TArray<TSubclassOf<UObject>> HandlersToAdd;
    
    // Category: Overrides
    // Use these to completely replace a property on the skill.
    //--------------------------------------------------------------------------------

    /** If specified, replaces the skill's projectile actor class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patch|Overrides")
    TSubclassOf<APoE2ProjectileBase> ProjectileClassOverride;
};