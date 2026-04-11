#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AscendDamageExecutionCalculation.generated.h"

/**
 * Resolves the source Damage attribute and optional SetByCaller bonus damage, then writes the
 * result into the target IncomingDamage meta attribute for centralized health processing.
 */
UCLASS()
class PROJECTASCEND_API UAscendDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UAscendDamageExecutionCalculation();

protected:
	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
