#include "Effects/Exec_Damage.h"
#include "Attributes/AttributeSet_Core.h"
#include "AbilitySystemComponent.h"
#include "Core/PoE2Log.h"
#include "Core/PoE2Tags.h"

UExec_Damage::UExec_Damage()
{
    // Capture Health attribute from target
    RelevantAttributesToCapture.Add(FGameplayEffectAttributeCaptureDefinition(
        UAttributeSet_Core::GetHealthAttribute(),
        EGameplayEffectAttributeCaptureSource::Target,
        false
    ));
}

void UExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // Get the target's ASC
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
    if (!TargetASC)
    {
        UE_LOG(LogPoE2Framework, Warning, TEXT("Exec_Damage: No target ASC"));
        return;
    }

    // Read damage from SetByCaller using the Data.Damage tag
    float DamageValue = Spec.GetSetByCallerMagnitude(FPoE2Tags::Get().Data_Damage, false, 0.0f);

    if (DamageValue <= 0.0f)
    {
        UE_LOG(LogPoE2Framework, Log, TEXT("Exec_Damage: No damage to apply (%.2f)"), DamageValue);
        return;
    }

    UE_LOG(LogPoE2Framework, Log, TEXT("Exec_Damage: Applying %.2f damage"), DamageValue);

    // Apply damage to Health attribute (negative modifier to reduce health)
    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UAttributeSet_Core::GetHealthAttribute(), EGameplayModOp::Additive, -DamageValue));
}