#include "AscendAttributeSetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAttributeSetBase)

void UAscendAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttributes(Attribute, NewValue);
}

void UAscendAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UAscendAttributeSetBase::ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const
{
	const float MinValue = const_cast<UAscendAttributeSetBase*>(this)->GetClampMinimumValueFor(Attribute);
	const float MaxValue = GetClampMaximumValueFor(Attribute);
	NewValue = FMath::Clamp(NewValue, MinValue, MaxValue);
}

float UAscendAttributeSetBase::GetClampMinimumValueFor(const FGameplayAttribute& Attribute)
{
	return 0.0f;
}

float UAscendAttributeSetBase::GetClampMaximumValueFor(const FGameplayAttribute& Attribute) const
{
	return TNumericLimits<float>::Max();
}

const FGameplayTagContainer& UAscendAttributeSetBase::GetSourceTagsFromContext(const FGameplayEffectModCallbackData& Data)
{
	static const FGameplayTagContainer EmptyTags;

	const FGameplayTagContainer* SourceTags = Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	return SourceTags ? *SourceTags : EmptyTags;
}

void UAscendAttributeSetBase::AdjustAttributeForMaxChange(
	FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute,
	float NewMaxValue,
	const FGameplayAttribute& AffectedAttributeProperty) const
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!ASC || FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue))
	{
		return;
	}

	const float CurrentValue = AffectedAttribute.GetCurrentValue();
	const float NewDelta = CurrentMaxValue > 0.0f
		? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue
		: NewMaxValue;

	ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
}

void UAscendAttributeSetBase::GetExecutionDataFromMod(
	const FGameplayEffectModCallbackData& Data,
	FAscendAttributeSetExecutionData& OutExecutionData)
{
	OutExecutionData = FAscendAttributeSetExecutionData();

	OutExecutionData.Context = Data.EffectSpec.GetContext();
	OutExecutionData.DeltaValue = Data.EvaluatedData.Magnitude;
	OutExecutionData.SourceASC = OutExecutionData.Context.GetOriginalInstigatorAbilitySystemComponent();
	OutExecutionData.TargetASC = &Data.Target;
	OutExecutionData.EffectCauser = OutExecutionData.Context.GetEffectCauser();
	OutExecutionData.SourceObject = OutExecutionData.Context.GetSourceObject();

	if (const FGameplayTagContainer* SourceTags = Data.EffectSpec.CapturedSourceTags.GetAggregatedTags())
	{
		OutExecutionData.SourceTags.AppendTags(*SourceTags);
	}

	if (const FGameplayTagContainer* TargetTags = Data.EffectSpec.CapturedTargetTags.GetAggregatedTags())
	{
		OutExecutionData.TargetTags.AppendTags(*TargetTags);
	}

	Data.EffectSpec.GetAllAssetTags(OutExecutionData.SpecAssetTags);

	if (OutExecutionData.TargetASC && OutExecutionData.TargetASC->AbilityActorInfo.IsValid())
	{
		const FGameplayAbilityActorInfo* TargetInfo = OutExecutionData.TargetASC->AbilityActorInfo.Get();
		OutExecutionData.TargetActor = TargetInfo && TargetInfo->AvatarActor.IsValid()
			? TargetInfo->AvatarActor.Get()
			: nullptr;
		OutExecutionData.TargetController = TargetInfo ? TargetInfo->PlayerController.Get() : nullptr;
		OutExecutionData.TargetPawn = TargetInfo && TargetInfo->AvatarActor.IsValid()
			? Cast<APawn>(TargetInfo->AvatarActor.Get())
			: nullptr;
	}

	if (OutExecutionData.SourceASC && OutExecutionData.SourceASC->AbilityActorInfo.IsValid())
	{
		const FGameplayAbilityActorInfo* SourceInfo = OutExecutionData.SourceASC->AbilityActorInfo.Get();
		OutExecutionData.SourceActor = OutExecutionData.EffectCauser;
		if (!OutExecutionData.SourceActor && SourceInfo && SourceInfo->AvatarActor.IsValid())
		{
			OutExecutionData.SourceActor = SourceInfo->AvatarActor.Get();
		}
		OutExecutionData.SourceController = SourceInfo ? SourceInfo->PlayerController.Get() : nullptr;
		OutExecutionData.SourcePawn = SourceInfo && SourceInfo->AvatarActor.IsValid()
			? Cast<APawn>(SourceInfo->AvatarActor.Get())
			: nullptr;
	}
}
