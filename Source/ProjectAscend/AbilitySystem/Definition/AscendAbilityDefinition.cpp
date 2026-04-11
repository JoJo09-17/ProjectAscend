#include "AbilitySystem/Definition/AscendAbilityDefinition.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_AnimationProvider.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_EffectContainers.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_InputBinding.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilityDefinition)

const FAscendAbilityFragment_InputBinding* UAscendAbilityDefinition::GetInputBindingFragment() const
{
	return FindFragmentByClass<FAscendAbilityFragment_InputBinding>();
}

FAscendAbilityFragment_InputBinding* UAscendAbilityDefinition::GetInputBindingFragment()
{
	return FindFragmentByClass<FAscendAbilityFragment_InputBinding>();
}

const FAscendAbilityFragment_EffectContainers* UAscendAbilityDefinition::GetEffectContainersFragment() const
{
	return FindFragmentByClass<FAscendAbilityFragment_EffectContainers>();
}

FAscendAbilityFragment_EffectContainers* UAscendAbilityDefinition::GetEffectContainersFragment()
{
	return FindFragmentByClass<FAscendAbilityFragment_EffectContainers>();
}

const FAscendAbilityFragment_AnimationProvider* UAscendAbilityDefinition::GetAnimationProviderFragment() const
{
	return FindFragmentByClass<FAscendAbilityFragment_AnimationProvider>();
}

FAscendAbilityFragment_AnimationProvider* UAscendAbilityDefinition::GetAnimationProviderFragment()
{
	return FindFragmentByClass<FAscendAbilityFragment_AnimationProvider>();
}

#if WITH_EDITOR
void UAscendAbilityDefinition::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	// Re-normalize input binding when slot binding support changes (clears stale DefaultSlotInputTag).
	const FName ChangedPropertyName = PropertyChangedEvent.GetPropertyName();
	if (ChangedPropertyName != GET_MEMBER_NAME_CHECKED(FAscendAbilityFragment_InputBinding, bSupportsSlotBinding))
	{
		return;
	}

	if (FAscendAbilityFragment_InputBinding* InputBinding = GetInputBindingFragment())
	{
		InputBinding->Normalize();
	}
}
#endif
