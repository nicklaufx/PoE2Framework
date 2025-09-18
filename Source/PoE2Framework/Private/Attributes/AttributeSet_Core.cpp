// Copyright 2025 liu f cheng. All rights reserved.
// Use of this source code is governed by a MIT license that can be
// found in the LICENSE file.

#include "Attributes/AttributeSet_Core.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "Core/PoE2Log.h"

UAttributeSet_Core::UAttributeSet_Core()
{
    Health = 100.0f;
    MaxHealth = 100.0f;
}

void UAttributeSet_Core::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Core, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAttributeSet_Core, MaxHealth, COND_None, REPNOTIFY_Always);
}

void UAttributeSet_Core::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // Clamp Health to valid range
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetMaxHealthAttribute())
    {
        NewValue = FMath::Max(NewValue, 1.0f); // MinHealth of 1
    }
}

void UAttributeSet_Core::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // Clamp Health after effects are applied
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
    }
}

void UAttributeSet_Core::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Core, Health, OldHealth);

    UE_LOG(LogPoE2Framework, Log, TEXT("Health changed from %.2f to %.2f"), OldHealth.GetCurrentValue(), GetHealth());
}

void UAttributeSet_Core::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAttributeSet_Core, MaxHealth, OldMaxHealth);

    // Ensure current health doesn't exceed new max health
    if (GetHealth() > GetMaxHealth())
    {
        SetHealth(GetMaxHealth());
    }

    UE_LOG(LogPoE2Framework, Log, TEXT("MaxHealth changed from %.2f to %.2f"), OldMaxHealth.GetCurrentValue(), GetMaxHealth());
}