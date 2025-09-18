#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class POE2FRAMEWORK_API FPoE2Tags
{
public:
    static void InitializeNativeTags();
    static const FPoE2Tags& Get() { return GameplayTags; }

    // Attribute Tags
    FGameplayTag Attributes_Core_Health;
    FGameplayTag Attributes_Core_MaxHealth;

    // Damage Tags
    FGameplayTag Damage_Base;
    FGameplayTag Damage_Type_Fire; // Example for future use
    FGameplayTag Damage_Type_Cold; // Example for future use

    // Data Tags
    FGameplayTag Data_Damage;

private:
    static FPoE2Tags GameplayTags;
};