// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "PoE2CueManager.generated.h"

/**
 * @class UPoE2CueManager
 * @brief Overrides the default GameplayCueManager to provide custom logic for loading cues.
 * This class is intended to be set as the default GameplayCueManager in DefaultEngine.ini.
 * It primarily serves as a wrapper and does not implement specific VFX logic itself,
 * but rather delegates to the GameplayCue system.
 * 
 * Example usage in DefaultEngine.ini:
 * [/Script/GameplayAbilities.AbilitySystemGlobals]
 * GlobalGameplayCueManagerClass="/Script/PoE2Framework.PoE2CueManager"
 */
UCLASS()
class POE2FRAMEWORK_API UPoE2CueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	/**
	 * @brief Plays a gameplay cue that is only visible locally.
	 * Does not replicate. Useful for UI feedback or effects only the local player should see.
	 * @param Target The target actor on which to play the cue.
	 * @param CueTag The tag identifying the gameplay cue.
	 * @param Parameters Additional parameters for the gameplay cue.
	 */
	static void PlayLocalCue(AActor* Target, FGameplayTag CueTag, const FGameplayCueParameters& Parameters);

	/**
	 * @brief Plays a gameplay cue that replicates to all clients.
	 * @param Target The target actor on which to play the cue.
	 * @param CueTag The tag identifying the gameplay cue.
	 * @param Parameters Additional parameters for the gameplay cue.
	 */
	static void PlayNetCue(AActor* Target, FGameplayTag CueTag, const FGameplayCueParameters& Parameters);
};