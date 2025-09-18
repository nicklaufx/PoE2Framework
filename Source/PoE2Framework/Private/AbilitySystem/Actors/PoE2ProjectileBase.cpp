#include "AbilitySystem/Actors/PoE2ProjectileBase.h"
#include "Spec/SkillSpec.h"
#include "Core/PoE2Log.h"
#include "CueSystem/PoE2CueManager.h"
#include "AbilitySystem/Handlers/MechanicHandler.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Core/PoE2Tags.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

APoE2ProjectileBase::APoE2ProjectileBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // Enable replication as required
    bReplicates = true;
    SetReplicatingMovement(true);

    // Create sphere collision component
    USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    RootComponent = SphereComponent;
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
    SphereComponent->OnComponentHit.AddDynamic(this, &APoE2ProjectileBase::OnHit);

    // Create projectile movement component
    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));
    MovementComponent->SetUpdatedComponent(SphereComponent);
    MovementComponent->InitialSpeed = 1000.0f;
    MovementComponent->MaxSpeed = 1000.0f;
    MovementComponent->bRotationFollowsVelocity = true;
    MovementComponent->bShouldBounce = false;
    MovementComponent->ProjectileGravityScale = 0.0f;
}

void APoE2ProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(APoE2ProjectileBase, CurrentSpec);
}

void APoE2ProjectileBase::InitFromSpec(const FSkillSpec& InSpec)
{
    CurrentSpec = InSpec;

    // Set projectile speed
    if (MovementComponent && CurrentSpec.ProjectileSpeed > 0.0f)
    {
        MovementComponent->InitialSpeed = CurrentSpec.ProjectileSpeed;
        MovementComponent->MaxSpeed = CurrentSpec.ProjectileSpeed;
    }

    // Set projectile lifetime
    if (CurrentSpec.Lifetime > 0.0f)
    {
        SetLifeSpan(CurrentSpec.Lifetime);
    }

    // Set initial direction (forward direction)
    if (MovementComponent)
    {
        MovementComponent->Velocity = GetActorForwardVector() * MovementComponent->InitialSpeed;
    }

    UE_LOG(LogPoE2Framework, Log, TEXT("Projectile initialized from spec: SkillId=%s, Speed=%.1f, Lifetime=%.1f"),
        *CurrentSpec.SkillId.ToString(), CurrentSpec.ProjectileSpeed, CurrentSpec.Lifetime);
}

void APoE2ProjectileBase::BeginPlay()
{
    Super::BeginPlay();
}

void APoE2ProjectileBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void APoE2ProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // Only process hits on the server
    if (!HasAuthority())
    {
        return;
    }

    // Don't hit ourselves or our owner
    if (OtherActor == this || OtherActor == GetOwner())
    {
        return;
    }

    UE_LOG(LogPoE2Framework, Log, TEXT("Projectile hit: %s at location %s"),
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"),
        *Hit.Location.ToString());

    // Apply damage effect if available
    if (CurrentSpec.DamageEffectClass && OwnerASC)
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            FGameplayEffectContextHandle ContextHandle = OwnerASC->MakeEffectContext();
            ContextHandle.AddSourceObject(this);

            FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(CurrentSpec.DamageEffectClass, 1.0f, ContextHandle);

            if (SpecHandle.IsValid())
            {
                // Use our static tag to pass the damage value to Exec_Damage
                SpecHandle.Data->SetSetByCallerMagnitude(FPoE2Tags::Get().Data_Damage, CurrentSpec.FinalDamage);
                OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
            }
        }
    }

    // Play impact cue
    FGameplayCueParameters CueParams;
    CueParams.Location = Hit.Location;
    CueParams.Normal = Hit.Normal;
    CueParams.PhysicalMaterial = Hit.PhysMaterial;

    // Use a generic impact cue tag
    FGameplayTag ImpactCueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Projectile.Impact"));
    UPoE2CueManager::PlayNetCue(this, ImpactCueTag, CueParams);

    // Handle projectile mechanics by iterating through handlers from the spec
    // All game mechanics (including pierce) are now handled by MechanicHandlers
    for (TSubclassOf<UObject> HandlerClass : CurrentSpec.MechanicHandlers)
    {
        if (HandlerClass)
        {
            if (IMechanicHandler* Handler = Cast<IMechanicHandler>(HandlerClass->GetDefaultObject()))
            {
                EHitHandlerResult Result = Handler->OnHit_Implementation(this, OtherActor, Hit, CurrentSpec);

                if (Result == EHitHandlerResult::Stop)
                {
                    // Handler decided to destroy the projectile
                    UE_LOG(LogPoE2Framework, Log, TEXT("Handler stopped projectile"));
                    Destroy();
                    return;
                }
                else if (Result == EHitHandlerResult::Pierce)
                {
                    // Handler decided to pierce, continue without destroying
                    UE_LOG(LogPoE2Framework, Log, TEXT("Handler allowed projectile to pierce through target"));
                    return;
                }
                // Continue means keep processing other handlers
            }
        }
    }

    // If no handler made a decision, default behavior is to destroy
    UE_LOG(LogPoE2Framework, Log, TEXT("No handler made a pierce decision, projectile destroyed"));
    Destroy();
}