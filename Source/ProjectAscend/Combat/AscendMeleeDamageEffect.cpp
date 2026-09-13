#include "Combat/AscendMeleeDamageEffect.h"
#include "AbilitySystem/Calculations/AscendDamageExecutionCalculation.h"

UAscendMeleeDamageEffect::UAscendMeleeDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayEffectExecutionDefinition Execution;
	Execution.CalculationClass = UAscendDamageExecutionCalculation::StaticClass();
	Executions.Add(Execution);
}
