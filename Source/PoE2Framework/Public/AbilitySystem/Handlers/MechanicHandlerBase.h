// Copyright Your Company, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Engine/HitResult.h"
#include "MechanicHandler.h"
#include "MechanicHandlerBase.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class AActor;
struct FSkillSpec;

/**
 * Base UObject implementation of IMechanicHandler.
 * Concrete mechanic handlers should inherit from this class and override the relevant _Implementation functions.
 * This provides a solid foundation for both C++ and Blueprint-based handlers.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class POE2FRAMEWORK_API UMechanicHandlerBase : public UObject, public IMechanicHandler
{
    GENERATED_BODY()

public:
    UMechanicHandlerBase();

    //================================================================================
    // IMechanicHandler Interface Implementation
    //================================================================================

    /** Called from GA_SkillBase right after the ability is committed. */
    virtual void OnCast_Implementation(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec) override;

    /** Called from the skill's carrier actor during its initialization. */
    virtual void OnSpawn_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) override;

    /** Called from the skill's carrier actor when it hits a target. */
    virtual EHitHandlerResult OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec) override;

    /** Called every frame on the skill's carrier actor. */
    virtual void OnTick_Implementation(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec) override;

    /** Called from the skill's carrier actor right before it is destroyed. */
    virtual void OnEnd_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) override;

protected:
    //================================================================================
    // Helper Properties
    //================================================================================

    /** Debug name for this handler, useful for logging and debugging. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    FString HandlerName;

    /** Whether this handler should log its lifecycle events for debugging. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
    bool bDebugLogging = false;
};