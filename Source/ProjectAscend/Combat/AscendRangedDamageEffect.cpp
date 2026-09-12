#include "Combat/AscendRangedDamageEffect.h"

#include "AbilitySystem/Calculations/AscendDamageExecutionCalculation.h"

UAscendRangedDamageEffect::UAscendRangedDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition DamageExecution;
	DamageExecution.CalculationClass = UAscendDamageExecutionCalculation::StaticClass();
	Executions.Add(DamageExecution);
}
