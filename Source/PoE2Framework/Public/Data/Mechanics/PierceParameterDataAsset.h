// Copyright 2025 liufucheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#pragma once

#include "CoreMinimal.h"
#include "Data/ParameterDataAsset.h"
#include "PierceParameterDataAsset.generated.h"

class UMechanic_Pierce;

/**
 * Defines the parameters for the Pierce mechanic.
 */
UCLASS(BlueprintType, meta=(DisplayName="Params: Pierce"))
class POE2FRAMEWORK_API UPierceParameterDataAsset : public UParameterDataAsset
{
    GENERATED_BODY()

public:
    /** The number of times a projectile with this mechanic can pierce through targets. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pierce")
    int32 PierceCount = 1;

    virtual void ContributeToParameterMap(TMap<FName, float>& InOutMap) const override;
};