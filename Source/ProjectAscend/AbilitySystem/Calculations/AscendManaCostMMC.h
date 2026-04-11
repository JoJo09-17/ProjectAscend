#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AscendManaCostMMC.generated.h"

/**
 * Calculates an additive mana delta from a configurable base cost and the source AttackSpeed attribute.
 * Higher AttackSpeed increases the resource cost to offset faster ability throughput.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCEND_API UAscendManaCostMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UAscendManaCostMMC();

protected:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	/** Authored positive cost value before AttackSpeed scaling. Override via Blueprint child per effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float BaseManaCost = 20.0f;

	/** Lower clamp for the AttackSpeed scalar used by the calculation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float MinimumAttackSpeedScalar = 0.5f;

	/** Upper clamp for the AttackSpeed scalar used by the calculation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float MaximumAttackSpeedScalar = 3.0f;
};
