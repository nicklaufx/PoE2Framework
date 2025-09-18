#include "AbilitySystem/Actors/PoE2AreaEffectBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Core/PoE2Tags.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UObjectGlobals.h"

APoE2AreaEffectBase::APoE2AreaEffectBase()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(false);
    bReplicates = true;

    AreaComponent = CreateDefaultSubobject<USphereComponent>(TEXT("AreaComponent"));
    SetRootComponent(AreaComponent);
    AreaComponent->InitSphereRadius(100.0f);
    AreaComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AreaComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    AreaComponent->SetGenerateOverlapEvents(true);

    DamageTickInterval = 1.0f;
    TimeSinceLastPulse = 0.0f;
}

const FName APoE2AreaEffectBase::AreaTickIntervalKey(TEXT("Area.TickInterval"));

void APoE2AreaEffectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoE2AreaEffectBase, CurrentSpec);
}

void APoE2AreaEffectBase::BeginPlay()
{
    Super::BeginPlay();
}

void APoE2AreaEffectBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (HasAuthority())
    {
        TimeSinceLastPulse += DeltaSeconds;
        if (TimeSinceLastPulse >= DamageTickInterval && DamageTickInterval > KINDA_SMALL_NUMBER)
        {
            TimeSinceLastPulse -= DamageTickInterval;
            HandleAreaPulse();
        }
    }

    for (const TScriptInterface<IMechanicHandler>& Handler : ActiveHandlers)
    {
        if (Handler)
        {
            IMechanicHandler::Execute_OnTick(Handler.GetObject(), this, DeltaSeconds, CurrentSpec);
        }
    }
}

void APoE2AreaEffectBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void APoE2AreaEffectBase::InitFromSpec(const FSkillSpec& InSpec, UAbilitySystemComponent* InOwnerASC, const TArray<TScriptInterface<IMechanicHandler>>& HandlerPrototypes)
{
    CurrentSpec = InSpec;
    OwnerASC = InOwnerASC;

    DamageTickInterval = FMath::Max(0.05f, CurrentSpec.GetCustomParam(AreaTickIntervalKey, 1.0f));
    TimeSinceLastPulse = DamageTickInterval;

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

    if (AreaComponent)
    {
        const float Radius = (CurrentSpec.AreaRadius > 0.0f) ? CurrentSpec.AreaRadius : AreaComponent->GetUnscaledSphereRadius();
        AreaComponent->SetSphereRadius(Radius, true);
        AreaComponent->UpdateOverlaps();
    }

    const bool bShouldTick = (ActiveHandlers.Num() > 0) || (CurrentSpec.DamageEffectClass != nullptr);
    SetActorTickEnabled(bShouldTick);

    if (HasAuthority() && CurrentSpec.DamageEffectClass)
    {
        HandleAreaPulse();
        TimeSinceLastPulse = 0.0f;
    }
}

int32 APoE2AreaEffectBase::GetActiveHandlerCount() const
{
    return ActiveHandlers.Num();
}

void APoE2AreaEffectBase::HandleAreaPulse()
{
    if (!AreaComponent)
    {
        return;
    }

    TArray<AActor*> OverlappingActors;
    AreaComponent->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor || Actor == this || Actor == GetOwner())
        {
            continue;
        }

        ApplyEffectToActor(Actor);
    }
}

void APoE2AreaEffectBase::ApplyEffectToActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    if (OwnerASC && CurrentSpec.DamageEffectClass)
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
        {
            FGameplayEffectContextHandle ContextHandle = OwnerASC->MakeEffectContext();
            ContextHandle.AddSourceObject(this);

            FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(CurrentSpec.DamageEffectClass, 1.0f, ContextHandle);

            if (SpecHandle.IsValid())
            {
                SpecHandle.Data->SetSetByCallerMagnitude(FPoE2Tags::Get().Data_Damage, CurrentSpec.FinalDamage);
                OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
            }
        }
    }

    FHitResult DummyHit;
    DummyHit.Location = TargetActor->GetActorLocation();
    DummyHit.ImpactPoint = DummyHit.Location;

    for (const TScriptInterface<IMechanicHandler>& Handler : ActiveHandlers)
    {
        if (Handler)
        {
            IMechanicHandler::Execute_OnHit(Handler.GetObject(), this, TargetActor, DummyHit, CurrentSpec);
        }
    }
}
