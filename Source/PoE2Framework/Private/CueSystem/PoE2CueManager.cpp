#include "CueSystem/PoE2CueManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/PoE2Log.h"

void UPoE2CueManager::PlayLocalCue(AActor* Target, FGameplayTag CueTag, const FGameplayCueParameters& Parameters)
{
    if (!Target)
    {
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!ASC)
    {
        return;
    }

    // Execute locally only (no replication)
    ASC->ExecuteGameplayCue(CueTag, Parameters);
}

void UPoE2CueManager::PlayNetCue(AActor* Target, FGameplayTag CueTag, const FGameplayCueParameters& Parameters)
{
    if (!Target)
    {
        return;
    }

    // Add a check to ensure this is called on the server
    if (Target->GetLocalRole() < ROLE_Authority)
    {
        UE_LOG(LogPoE2Framework, Warning, TEXT("PlayNetCue should only be called on the server."));
        return;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!ASC)
    {
        return;
    }

    // Create a context handle with the parameters
    FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
    if (FGameplayEffectContext* Context = ContextHandle.Get())
    {
        Context->AddHitResult(FHitResult(), true);
    }

    // Server calls multicast to all clients with the context handle
    ASC->NetMulticast_InvokeGameplayCueExecuted(CueTag, ASC->GetPredictionKeyForNewAction(), ContextHandle);
}