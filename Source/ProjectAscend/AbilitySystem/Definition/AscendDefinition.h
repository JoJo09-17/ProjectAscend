#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "AscendDefinition.generated.h"

/**
 * Base data asset using a fragment pattern (TInstancedStruct) to compose ability data.
 * Fragments are inspected by type or tag to retrieve specific configuration.
 */
UCLASS()
class PROJECTASCEND_API UAscendDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/** Ordered list of fragments that compose this definition's data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragments", NoClear, 
	meta = (PinHiddenByDefault, ShowTreeView, ExcludeBaseStruct))
	TArray<TInstancedStruct<FAscendAbilityFragment>> Fragments;

	/** Finds a fragment by its UScriptStruct type (exact or derived). */
	const FAscendAbilityFragment* FindFragmentByStruct(const UScriptStruct* FragmentStruct) const;
	FAscendAbilityFragment* FindFragmentByStruct(const UScriptStruct* FragmentStruct);

	/** Finds a fragment whose FragmentTag matches exactly. */
	const FAscendAbilityFragment* FindFragmentByFragmentTag(FGameplayTag InFragmentTag) const;

	/** Returns all fragments that contain the given dynamic tag. */
	TArray<const FAscendAbilityFragment*> FindFragmentsByDynamicTag(FGameplayTag InDynamicTag) const;

	/** Returns all fragments matching the dynamic tag filter (any or all, controlled by bRequireAllTags). */
	TArray<const FAscendAbilityFragment*> FindFragmentsByDynamicTags(FGameplayTagContainer InDynamicTags, bool bRequireAllTags = false) const;

	/** Typed fragment lookup by struct type. */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		const FAscendAbilityFragment* Fragment = FindFragmentByStruct(ResultClass::StaticStruct());
		return Fragment ? static_cast<const ResultClass*>(Fragment) : nullptr;
	}

	template <typename ResultClass>
	ResultClass* FindFragmentByClass()
	{
		FAscendAbilityFragment* Fragment = FindFragmentByStruct(ResultClass::StaticStruct());
		return Fragment ? static_cast<ResultClass*>(Fragment) : nullptr;
	}
};
