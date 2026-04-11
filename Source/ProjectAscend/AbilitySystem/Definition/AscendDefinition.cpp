#include "AscendDefinition.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendDefinition)

const FAscendAbilityFragment* UAscendDefinition::FindFragmentByStruct(const UScriptStruct* FragmentStruct) const
{
	if (!FragmentStruct)
	{
		return nullptr;
	}

	for (const TInstancedStruct<FAscendAbilityFragment>& Fragment : Fragments)
	{
		const UScriptStruct* InstanceStruct = Fragment.GetScriptStruct();
		if (InstanceStruct && InstanceStruct->IsChildOf(FragmentStruct))
		{
			return reinterpret_cast<const FAscendAbilityFragment*>(Fragment.GetMemory());
		}
	}

	return nullptr;
}

FAscendAbilityFragment* UAscendDefinition::FindFragmentByStruct(const UScriptStruct* FragmentStruct)
{
	if (!FragmentStruct)
	{
		return nullptr;
	}

	for (TInstancedStruct<FAscendAbilityFragment>& Fragment : Fragments)
	{
		const UScriptStruct* InstanceStruct = Fragment.GetScriptStruct();
		if (InstanceStruct && InstanceStruct->IsChildOf(FragmentStruct))
		{
			return reinterpret_cast<FAscendAbilityFragment*>(Fragment.GetMutableMemory());
		}
	}

	return nullptr;
}

const FAscendAbilityFragment* UAscendDefinition::FindFragmentByFragmentTag(FGameplayTag InFragmentTag) const
{
	if (!InFragmentTag.IsValid())
	{
		return nullptr;
	}

	for (const TInstancedStruct<FAscendAbilityFragment>& FragmentData : Fragments)
	{
		const FAscendAbilityFragment* Fragment = reinterpret_cast<const FAscendAbilityFragment*>(FragmentData.GetMemory());
		if (Fragment && Fragment->HasFragmentTagExact(InFragmentTag))
		{
			return Fragment;
		}
	}

	return nullptr;
}

TArray<const FAscendAbilityFragment*> UAscendDefinition::FindFragmentsByDynamicTag(FGameplayTag InDynamicTag) const
{
	TArray<const FAscendAbilityFragment*> Result;

	if (!InDynamicTag.IsValid())
	{
		return Result;
	}

	Result.Reserve(Fragments.Num());

	for (const TInstancedStruct<FAscendAbilityFragment>& FragmentData : Fragments)
	{
		const FAscendAbilityFragment* Fragment = reinterpret_cast<const FAscendAbilityFragment*>(FragmentData.GetMemory());
		if (Fragment && Fragment->HasDynamicTag(InDynamicTag))
		{
			Result.Add(Fragment);
		}
	}

	return Result;
}

TArray<const FAscendAbilityFragment*> UAscendDefinition::FindFragmentsByDynamicTags(FGameplayTagContainer InDynamicTags,
	bool bRequireAllTags) const
{
	TArray<const FAscendAbilityFragment*> Result;

	if (InDynamicTags.IsEmpty())
	{
		return Result;
	}

	Result.Reserve(Fragments.Num());

	for (const TInstancedStruct<FAscendAbilityFragment>& FragmentData : Fragments)
	{
		const FAscendAbilityFragment* Fragment = reinterpret_cast<const FAscendAbilityFragment*>(FragmentData.GetMemory());
		if (!Fragment)
		{
			continue;
		}

		const bool bMatches = bRequireAllTags
			? Fragment->HasAllDynamicTags(InDynamicTags)
			: Fragment->HasAnyDynamicTags(InDynamicTags);

		if (bMatches)
		{
			Result.Add(Fragment);
		}
	}

	return Result;
}
