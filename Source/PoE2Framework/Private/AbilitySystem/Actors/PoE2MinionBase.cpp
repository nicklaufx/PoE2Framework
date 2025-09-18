#include "AbilitySystem/Actors/PoE2MinionBase.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UObjectGlobals.h"

APoE2MinionBase::APoE2MinionBase()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(false);
    bReplicates = true;
}

void APoE2MinionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoE2MinionBase, CurrentSpec);
}

void APoE2MinionBase::BeginPlay()
{
    Super::BeginPlay();
}

void APoE2MinionBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    for (const TScriptInterface<IMechanicHandler>& Handler : ActiveHandlers)
    {
        if (Handler)
        {
            IMechanicHandler::Execute_OnTick(Handler.GetObject(), this, DeltaSeconds, CurrentSpec);
        }
    }
}

void APoE2MinionBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (const TScriptInterface<IMechanicHandler>& Handler : ActiveHandlers)
    {
        if (Handler)
        {
            IMechanicHandler::Execute_OnEnd(Handler.GetObject(), this, CurrentSpec);
        }
    }
    ActiveHandlers.Reset();

    Super::EndPlay(EndPlayReason);
}

void APoE2MinionBase::InitFromSpec(const FSkillSpec& InSpec, UAbilitySystemComponent* InOwnerASC, const TArray<TScriptInterface<IMechanicHandler>>& HandlerPrototypes)
{
    CurrentSpec = InSpec;
    OwnerASC = InOwnerASC;

    ActiveHandlers.Reset();
    for (const TScriptInterface<IMechanicHandler>& HandlerPrototype : HandlerPrototypes)
    {
        UObject* PrototypeObject = HandlerPrototype.GetObject();
        if (!PrototypeObject)
        {
            continue;
        }

        UObject* DuplicatedObject = DuplicateObject(PrototypeObject, this);
        if (!DuplicatedObject)
        {
            continue;
        }

        TScriptInterface<IMechanicHandler> HandlerInstance;
        HandlerInstance.SetObject(DuplicatedObject);
        HandlerInstance.SetInterface(Cast<IMechanicHandler>(DuplicatedObject));

        if (!HandlerInstance.GetInterface())
        {
            continue;
        }

        ActiveHandlers.Add(HandlerInstance);
        IMechanicHandler::Execute_OnSpawn(DuplicatedObject, this, CurrentSpec);
    }

    if (CurrentSpec.Lifetime > 0.0f)
    {
        SetLifeSpan(CurrentSpec.Lifetime);
    }

    SetActorTickEnabled(ActiveHandlers.Num() > 0);
}

int32 APoE2MinionBase::GetActiveHandlerCount() const
{
    return ActiveHandlers.Num();
}
