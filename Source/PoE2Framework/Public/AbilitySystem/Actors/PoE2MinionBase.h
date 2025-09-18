#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Spec/SkillSpec.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "PoE2MinionBase.generated.h"

class UAbilitySystemComponent;

UCLASS(BlueprintType)
class POE2FRAMEWORK_API APoE2MinionBase : public APawn
{
    GENERATED_BODY()

public:
    APoE2MinionBase();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Minion")
    virtual void InitFromSpec(const FSkillSpec& InSpec, UAbilitySystemComponent* InOwnerASC, const TArray<TScriptInterface<IMechanicHandler>>& HandlerPrototypes);

    UFUNCTION(BlueprintPure, Category = "Minion|Mechanics")
    int32 GetActiveHandlerCount() const;

protected:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Minion")
    FSkillSpec CurrentSpec;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Minion")
    TObjectPtr<UAbilitySystemComponent> OwnerASC;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Minion")
    TArray<TScriptInterface<IMechanicHandler>> ActiveHandlers;
};
