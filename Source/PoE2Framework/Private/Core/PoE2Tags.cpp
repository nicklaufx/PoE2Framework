#include "Core/PoE2Tags.h"
#include "GameplayTagsManager.h"
#include "Core/PoE2Log.h"

FPoE2Tags FPoE2Tags::GameplayTags;

void FPoE2Tags::InitializeNativeTags()
{
    UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

    GameplayTags.Attributes_Core_Health = Manager.AddNativeGameplayTag(
        TEXT("Attributes.Core.Health"),
        TEXT("The current health of an actor"));

    GameplayTags.Attributes_Core_MaxHealth = Manager.AddNativeGameplayTag(
        TEXT("Attributes.Core.MaxHealth"),
        TEXT("The maximum health of an actor"));

    GameplayTags.Damage_Base = Manager.AddNativeGameplayTag(
        TEXT("Damage.Base"),
        TEXT("Base damage tag for all damage types"));

    GameplayTags.Damage_Type_Fire = Manager.AddNativeGameplayTag(
        TEXT("Damage.Type.Fire"),
        TEXT("Fire damage type"));

    GameplayTags.Damage_Type_Cold = Manager.AddNativeGameplayTag(
        TEXT("Damage.Type.Cold"),
        TEXT("Cold damage type"));

    GameplayTags.Data_Damage = Manager.AddNativeGameplayTag(
        TEXT("Data.Damage"),
        TEXT("Tag used for SetByCaller damage values"));

    UE_LOG(LogPoE2Framework, Log, TEXT("PoE2 Native Gameplay Tags Initialized"));
}