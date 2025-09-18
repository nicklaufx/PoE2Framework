#include "AbilitySystem/Handlers/MechanicHandlerBase.h"
#include "AbilitySystemComponent.h"
#include "Spec/SkillSpec.h"
#include "Core/PoE2Log.h"
#include "Engine/Engine.h"

UMechanicHandlerBase::UMechanicHandlerBase()
{
    HandlerName = TEXT("BaseHandler");
    bDebugLogging = false;
}

void UMechanicHandlerBase::OnCast_Implementation(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec)
{
    if (bDebugLogging)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("%s::OnCast - Skill: %s"), *HandlerName, *SkillSpec.SkillId.ToString());
    }
    
    // Base implementation does nothing - override in derived classes
}

void UMechanicHandlerBase::OnSpawn_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec)
{
    if (bDebugLogging)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("%s::OnSpawn - Owner: %s, Skill: %s"), 
            *HandlerName, 
            OwnerActor ? *OwnerActor->GetName() : TEXT("None"), 
            *SkillSpec.SkillId.ToString());
    }
    
    // Base implementation does nothing - override in derived classes
}

EHitHandlerResult UMechanicHandlerBase::OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec)
{
    if (bDebugLogging)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("%s::OnHit - Owner: %s, Target: %s, Skill: %s"), 
            *HandlerName,
            OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
            Target ? *Target->GetName() : TEXT("None"),
            *SkillSpec.SkillId.ToString());
    }
    
    // Base implementation returns Stop (default projectile behavior)
    return EHitHandlerResult::Stop;
}

void UMechanicHandlerBase::OnTick_Implementation(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec)
{
    // Base implementation does nothing - override in derived classes
    // Note: No debug logging here due to performance concerns (called every frame)
}

void UMechanicHandlerBase::OnEnd_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec)
{
    if (bDebugLogging)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("%s::OnEnd - Owner: %s, Skill: %s"), 
            *HandlerName,
            OwnerActor ? *OwnerActor->GetName() : TEXT("None"),
            *SkillSpec.SkillId.ToString());
    }
    
    // Base implementation does nothing - override in derived classes
}