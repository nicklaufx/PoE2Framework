#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spec/SkillSpec.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "PoE2AreaEffectBase.generated.h"

class UAbilitySystemComponent;
class USphereComponent;

UCLASS(BlueprintType)
class POE2FRAMEWORK_API APoE2AreaEffectBase : public AActor
{
    GENERATED_BODY()

public:
    APoE2AreaEffectBase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "AreaEffect")
    virtual void InitFromSpec(const FSkillSpec& InSpec, UAbilitySystemComponent* InOwnerASC, const TArray<TScriptInterface<IMechanicHandler>>& HandlerPrototypes);

    UFUNCTION(BlueprintPure, Category = "AreaEffect|Mechanics")
    int32 GetActiveHandlerCount() const;

protected:
    UFUNCTION(BlueprintCallable, Category = "AreaEffect", meta=(BlueprintProtected="true"))
    void HandleAreaPulse();

    void ApplyEffectToActor(AActor* TargetActor);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AreaEffect")
    TObjectPtr<USphereComponent> AreaComponent;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "AreaEffect")
    FSkillSpec CurrentSpec;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AreaEffect")
    TObjectPtr<UAbilitySystemComponent> OwnerASC;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AreaEffect")
    TArray<TScriptInterface<IMechanicHandler>> ActiveHandlers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AreaEffect")
    float DamageTickInterval;

    float TimeSinceLastPulse;

    static const FName AreaTickIntervalKey;
};
