#include "Effects/GE_Damage.h"
#include "Effects/Exec_Damage.h"

UGE_Damage::UGE_Damage()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // Use custom execution calculation for damage
    Executions.AddDefaulted();
    Executions[0].CalculationClass = UExec_Damage::StaticClass();
}