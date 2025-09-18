// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Spec/Patch.h"
#include "SupportDataAsset.generated.h"

/**
 * Defines a Support Gem in a data-driven way.
 * A support gem's primary purpose is to provide an FPatch that modifies a base skill.
 */
UCLASS(BlueprintType, meta=(DisplayName="PoE2 Support DataAsset"))
class POE2FRAMEWORK_API USupportDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /**
     * File Path: Plugins/PoE2Framework/Source/PoE2Framework/Public/Data/SupportDataAsset.h
     * Class Name: USupportDataAsset
     */
    USupportDataAsset();

    //~ Begin UPrimaryDataAsset Interface
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
    //~ End UPrimaryDataAsset Interface

public:
    //================================================================================
    // Fields
    //================================================================================

    // Category: Identity
    //--------------------------------------------------------------------------------

    /** The unique identifier for this support gem. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FName SupportId;

    /** The name of the support gem displayed in the UI. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
    FText DisplayName;
    
    // Category: Patch Data
    //--------------------------------------------------------------------------------

    /** The actual modification this support gem applies to a skill. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Patch")
    FPatch SkillPatch;

    //================================================================================
    // TODO for Claude
    //================================================================================
    
    // TODO: [Claude] Implement the GetPrimaryAssetId() override in the .cpp file.
    // - It should return a FPrimaryAssetId with a type of 'Support' and a name of this asset's FName.
    // - This allows the Asset Manager to discover and manage support gems.
};