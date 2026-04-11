#include "AbilitySystem/Calculations/AscendManaCostMMC.h"

#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendManaCostMMC)

namespace
{
	struct FAscendManaCostMMCStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(AttackSpeed);

		FAscendManaCostMMCStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendAttributeSet, AttackSpeed, Source, false);
		}
	};

	const FAscendManaCostMMCStatics& GetAscendManaCostMMCStatics()
	{
		static const FAscendManaCostMMCStatics Statics;
		return Statics;
	}
}

UAscendManaCostMMC::UAscendManaCostMMC()
{
	RelevantAttributesToCapture.Add(GetAscendManaCostMMCStatics().AttackSpeedDef);
}

float UAscendManaCostMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackSpeed = 1.0f;
	GetCapturedAttributeMagnitude(GetAscendManaCostMMCStatics().AttackSpeedDef, Spec, EvaluationParameters, AttackSpeed);

	const float AttackSpeedScalar = FMath::Clamp(AttackSpeed, MinimumAttackSpeedScalar, MaximumAttackSpeedScalar);
	const float ManaCost = FMath::Max(0.0f, BaseManaCost * AttackSpeedScalar);

	// Cost gameplay effects typically use an additive modifier on Mana, so the runtime delta must be negative.
	return -ManaCost;
}
