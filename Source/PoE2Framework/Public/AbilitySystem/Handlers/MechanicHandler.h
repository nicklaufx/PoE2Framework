#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Engine/HitResult.h"
#include "MechanicHandler.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class AActor;
struct FSkillSpec;

// Hit handler result enum
UENUM(BlueprintType)
enum class EHitHandlerResult : uint8
{
    Continue,    // Continue with normal hit processing
    Stop,        // Stop processing this hit
    Pierce,      // Pierce through the target and continue
    Chain        // Chain to another target
};

UINTERFACE(MinimalAPI, BlueprintType, Blueprintable)
class UMechanicHandler : public UInterface
{
    GENERATED_BODY()
};

class POE2FRAMEWORK_API IMechanicHandler
{
    GENERATED_BODY()

public:
    /** Called from GA_SkillBase right after the ability is committed. */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mechanic Handler")
    void OnCast(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec);

    /** Called from the skill's carrier actor during its initialization. */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mechanic Handler")
    void OnSpawn(AActor* OwnerActor, const FSkillSpec& SkillSpec);

    /** Called from the skill's carrier actor when it hits a target. */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mechanic Handler")
    EHitHandlerResult OnHit(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec);

    /** Called every frame on the skill's carrier actor. */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mechanic Handler")
    void OnTick(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec);

    /** Called from the skill's carrier actor right before it is destroyed. */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Mechanic Handler")
    void OnEnd(AActor* OwnerActor, const FSkillSpec& SkillSpec);

    // C++ virtual functions for implementation
    virtual void OnCast_Implementation(UAbilitySystemComponent* CasterASC, const FSkillSpec& SkillSpec) {}
    virtual void OnSpawn_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) {}
    virtual EHitHandlerResult OnHit_Implementation(AActor* OwnerActor, AActor* Target, const FHitResult& HitResult, const FSkillSpec& SkillSpec) { return EHitHandlerResult::Continue; }
    virtual void OnTick_Implementation(AActor* OwnerActor, float DeltaTime, const FSkillSpec& SkillSpec) {}
    virtual void OnEnd_Implementation(AActor* OwnerActor, const FSkillSpec& SkillSpec) {}
};