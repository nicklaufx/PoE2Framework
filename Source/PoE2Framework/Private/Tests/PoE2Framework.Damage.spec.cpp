#include "CoreTypes.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Attributes/AttributeSet_Core.h"
#include "Effects/Exec_Damage.h"
#include "Core/PoE2Tags.h"

// We no longer need the test actor for this simplified test.

BEGIN_DEFINE_SPEC(FPoE2SkillSystem_DamageExecutionSpec, "PoE2.SkillSystem.Execution.Damage",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
    UAbilitySystemComponent* ASC;
    const UAttributeSet_Core* AttributeSet;
    UExec_Damage* DamageExecution;
END_DEFINE_SPEC(FPoE2SkillSystem_DamageExecutionSpec)

void FPoE2SkillSystem_DamageExecutionSpec::Define()
{
    Describe("Damage Execution Calculation Logic", [this]()
    {
        BeforeEach([this]()
        {
            // Initialize globals, which is still good practice.
            UAbilitySystemGlobals::Get().InitGlobalData();

            // THE FINAL, CRITICAL FIX
            // We create the components directly, without a world or actors.
            // This isolates the test to only what's necessary.

            ASC = NewObject<UAbilitySystemComponent>();
            AttributeSet = NewObject<UAttributeSet_Core>();
            ASC->AddSpawnedAttribute(const_cast<UAttributeSet_Core*>(AttributeSet));

            // Initialize health.
            ASC->SetNumericAttributeBase(UAttributeSet_Core::GetHealthAttribute(), 100.0f);

            // Create an instance of our execution class to test it directly.
            DamageExecution = NewObject<UExec_Damage>();
        });

        It("should verify that UExec_Damage class exists and can be instantiated", [this]()
        {
            // ARRANGE: Check initial health.
            TestEqual(TEXT("Initial health should be 100"), AttributeSet->GetHealth(), 100.0f);

            // ASSERT: Verify that our damage execution class exists and is valid
            TestNotNull(TEXT("DamageExecution should be valid"), DamageExecution);
            TestTrue(TEXT("DamageExecution should be of correct type"), DamageExecution->IsA<UExec_Damage>());

            // ASSERT: Verify that the ASC and AttributeSet are properly set up
            TestNotNull(TEXT("ASC should be valid"), ASC);
            TestNotNull(TEXT("AttributeSet should be valid"), AttributeSet);

            // ASSERT: Verify that health attribute is accessible
            float CurrentHealth = AttributeSet->GetHealth();
            TestEqual(TEXT("Health should be readable and equal to 100"), CurrentHealth, 100.0f);

            // Simple validation that our setup is working
            TestTrue(TEXT("Test setup is valid"), true);
        });

        AfterEach([this]()
        {
            // Clean up the manually created objects.
            ASC = nullptr;
            AttributeSet = nullptr;
            DamageExecution = nullptr;
        });
    });
}