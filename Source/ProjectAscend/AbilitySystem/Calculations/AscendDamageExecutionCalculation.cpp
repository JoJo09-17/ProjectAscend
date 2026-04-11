#include "AbilitySystem/Calculations/AscendDamageExecutionCalculation.h"

#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AscendGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendDamageExecutionCalculation)

namespace
{
	struct FAscendDamageExecutionStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

		FAscendDamageExecutionStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendAttributeSet, Damage, Source, false);
		}
	};

	const FAscendDamageExecutionStatics& GetDamageExecutionStatics()
	{
		static const FAscendDamageExecutionStatics Statics;
		return Statics;
	}
}

UAscendDamageExecutionCalculation::UAscendDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(GetDamageExecutionStatics().DamageDef);
}

void UAscendDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float SourceDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetDamageExecutionStatics().DamageDef,
		EvaluationParameters,
		SourceDamage);
	SourceDamage = FMath::Max(SourceDamage, 0.0f);

	const float BonusDamage = FMath::Max(
		Spec.GetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_Damage, false, 0.0f),
		0.0f);
	const float DamageMultiplier = FMath::Max(
		Spec.GetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_DamageMultiplier, false, 1.0f),
		0.0f);
	const float FinalDamage = (SourceDamage * DamageMultiplier) + BonusDamage;
	if (FinalDamage <= 0.0f)
	{
		return;
	}

	// Damage remains an authored combat stat on the source. The execution converts it into
	// incoming target damage so health reduction stays centralized in the AttributeSet.
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UAscendAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		FinalDamage));
}
