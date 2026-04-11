#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "AscendCooldownMMC.generated.h"

/**
 * Calculates cooldown duration from a configurable base value and the source AttackSpeed attribute.
 * Higher AttackSpeed shortens cooldowns while remaining clamped to a stable range.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTASCEND_API UAscendCooldownMMC : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UAscendCooldownMMC();

protected:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

protected:
	/** Authored base cooldown duration before AttackSpeed scaling. Override via Blueprint child per effect. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float BaseCooldownDuration = 6.0f;

	/** Lower clamp for the AttackSpeed scalar used by the calculation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float MinimumAttackSpeedScalar = 0.5f;

	/** Upper clamp for the AttackSpeed scalar used by the calculation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|MMC")
	float MaximumAttackSpeedScalar = 3.0f;
};
