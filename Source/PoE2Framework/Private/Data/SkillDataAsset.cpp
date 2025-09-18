#include "Data/SkillDataAsset.h"
#include "Spec/SkillSpec.h"
#include "Engine/AssetManager.h"

USkillDataAsset::USkillDataAsset()
{
    // Set default values
    SkillId = NAME_None;
    DisplayName = FText::GetEmpty();
    BaseDamage = 0.f;
    Cooldown = 0.f;
    Cost = 0.f;
    CastTime = 0.f;
    Radius = 0.f;
    Speed = 0.f;
    MaxRange = 0.f;
    Duration = 0.f;
    SummonCount = 1;
}

FPrimaryAssetId USkillDataAsset::GetPrimaryAssetId() const
{
    // Return a FPrimaryAssetId with a type of 'Skill' and a name of this asset's FName
    // This is important for the Asset Manager to discover and manage skill assets
    return FPrimaryAssetId(TEXT("Skill"), GetFName());
}

FSkillSpec USkillDataAsset::CreateBaseSkillSpec() const
{
    // Create a new SkillSpec and populate it with base data from this asset
    FSkillSpec NewSpec;
    
    // Identity & Basic Properties
    NewSpec.SkillId = this->SkillId;
    NewSpec.AbilityClass = this->AbilityClass;
    
    // Actor Classes
    NewSpec.ProjectileClass = this->ProjectileClass;
    NewSpec.AreaClass = nullptr; // Not defined in this DataAsset, would come from patches or other sources
    NewSpec.SummonClass = this->SummonClass;
    NewSpec.SummonCount = this->SummonCount;
    
    // Numerical Stats - Map from DataAsset naming to SkillSpec naming
    NewSpec.FinalDamage = this->BaseDamage;
    NewSpec.DamageEffectClass = this->DamageEffectClass;
    NewSpec.Cooldown = this->Cooldown;
    NewSpec.ResourceCost = this->Cost;  // Cost -> ResourceCost
    NewSpec.CastTime = this->CastTime;
    NewSpec.AreaRadius = this->Radius;  // Radius -> AreaRadius
    NewSpec.ProjectileSpeed = this->Speed;  // Speed -> ProjectileSpeed
    NewSpec.MaxRange = this->MaxRange;
    NewSpec.Lifetime = this->Duration;  // Duration -> Lifetime
    
    // Tags
    NewSpec.SkillTags = this->SkillTags;
    
    // Default Effects
    NewSpec.AppliedEffects = this->DefaultEffects;
    
    // Default Mechanic Handlers
    NewSpec.MechanicHandlers = this->DefaultHandlers;

    // Custom Parameters (empty by default, can be populated by patches)
    NewSpec.CustomParams.Empty();

    // Flatten the parameter data assets into the final TMap
    TMap<FName, float> FlattenedParams;
    for (const UParameterDataAsset* ParamDA : CustomMechanicParameters)
    {
        if (ParamDA)
        {
            ParamDA->ContributeToParameterMap(FlattenedParams);
        }
    }

    // Convert from TMap to TArray<FCustomParam> for network compatibility
    for (const auto& ParamPair : FlattenedParams)
    {
        NewSpec.SetCustomParam(ParamPair.Key, ParamPair.Value);
    }

    return NewSpec;
}