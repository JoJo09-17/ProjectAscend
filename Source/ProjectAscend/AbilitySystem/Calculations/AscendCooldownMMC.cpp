#include "AbilitySystem/Calculations/AscendCooldownMMC.h"

#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendCooldownMMC)

namespace
{
	struct FAscendCooldownMMCStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(AttackSpeed);

		FAscendCooldownMMCStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UAscendAttributeSet, AttackSpeed, Source, false);
		}
	};

	const FAscendCooldownMMCStatics& GetAscendCooldownMMCStatics()
	{
		static const FAscendCooldownMMCStatics Statics;
		return Statics;
	}
}

UAscendCooldownMMC::UAscendCooldownMMC()
{
	RelevantAttributesToCapture.Add(GetAscendCooldownMMCStatics().AttackSpeedDef);
}

float UAscendCooldownMMC::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float AttackSpeed = 1.0f;
	GetCapturedAttributeMagnitude(GetAscendCooldownMMCStatics().AttackSpeedDef, Spec, EvaluationParameters, AttackSpeed);

	const float AttackSpeedScalar = FMath::Clamp(AttackSpeed, MinimumAttackSpeedScalar, MaximumAttackSpeedScalar);
	return FMath::Max(0.0f, BaseCooldownDuration / AttackSpeedScalar);
}
