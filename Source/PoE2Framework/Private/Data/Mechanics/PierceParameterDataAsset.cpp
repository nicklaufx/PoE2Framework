// Copyright 2025 liufucheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#include "Data/Mechanics/PierceParameterDataAsset.h"
#include "AbilitySystem/Handlers/Mechanic_Pierce.h"

void UPierceParameterDataAsset::ContributeToParameterMap(TMap<FName, float>& InOutMap) const
{
    // Use the static key from the mechanic handler to ensure type safety.
    InOutMap.Add(UMechanic_Pierce::PierceCountKey, static_cast<float>(PierceCount));
}