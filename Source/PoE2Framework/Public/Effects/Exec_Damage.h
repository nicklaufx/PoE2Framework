// Copyright 2025 liu f cheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "Exec_Damage.generated.h"

/**
 * @class UExec_Damage
 * @brief A GameplayEffectExecutionCalculation for applying damage.
 * This class calculates the final damage amount based on various factors and applies it to the target's Health attribute.
 */
UCLASS()
class POE2FRAMEWORK_API UExec_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExec_Damage();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	// TODO:
	// 1. Critical Strike Calculation: Implement logic to determine if the damage is a critical strike and apply a damage multiplier accordingly.
	// 2. Resistance and Mitigation: Factor in target's resistances (e.g., armor, elemental resistances) to reduce the incoming damage.
	// 3. Damage Conversion: Handle cases where damage type gets converted (e.g., Physical to Fire).
};