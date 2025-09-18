// Copyright 2025 liufucheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ParameterDataAsset.generated.h"

/**
 * An abstract base class for data assets that define parameters for skill mechanics.
 * Each subclass is responsible for contributing its parameters to a flattened TMap
 * which is then used by the skill's runtime spec.
 */
UCLASS(Blueprintable, Abstract)
class POE2FRAMEWORK_API UParameterDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /**
     * Takes the parameters defined in this data asset and adds them to the provided TMap.
     * @param InOutMap The map to contribute parameters to. Keys should be unique and descriptive (e.g., "Chain.Count").
     */
    virtual void ContributeToParameterMap(TMap<FName, float>& InOutMap) const PURE_VIRTUAL(UParameterDataAsset::ContributeToParameterMap, );
};