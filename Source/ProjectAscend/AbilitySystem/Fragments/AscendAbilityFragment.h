#pragma once

#include "CoreMinimal.h"
#include "AscendGameplayTags.h"
#include "AscendAbilityFragment.generated.h"

/**
 * Base struct for ability definition fragments. Provides tag-based identification
 * and dynamic tag matching for fragment queries.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendAbilityFragment
{
	GENERATED_BODY()

public:
	virtual ~FAscendAbilityFragment() = default;

	const FGameplayTag& GetFragmentTag() const
	{
		return FragmentTag;
	}

	const FGameplayTagContainer& GetFragmentDynamicTags() const
	{
		return FragmentDynamicTags;
	}

	bool HasFragmentTagExact(const FGameplayTag& InTag) const
	{
		return InTag.IsValid() && FragmentTag.IsValid() && FragmentTag.MatchesTagExact(InTag);
	}

	bool HasFragmentTag(const FGameplayTag& InTag) const
	{
		return InTag.IsValid() && FragmentTag.IsValid() && FragmentTag.MatchesTag(InTag);
	}

	bool HasDynamicTagExact(const FGameplayTag& InTag) const
	{
		return InTag.IsValid() && FragmentDynamicTags.HasTagExact(InTag);
	}

	bool HasDynamicTag(const FGameplayTag& InTag) const
	{
		return InTag.IsValid() && FragmentDynamicTags.HasTag(InTag);
	}

	bool HasAnyDynamicTags(const FGameplayTagContainer& InTags) const
	{
		return !InTags.IsEmpty() && FragmentDynamicTags.HasAny(InTags);
	}

	bool HasAllDynamicTags(const FGameplayTagContainer& InTags) const
	{
		return !InTags.IsEmpty() && FragmentDynamicTags.HasAll(InTags);
	}

protected:
	/** Stable identifier for this fragment type (e.g. Fragment.InputBinding). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Fragment|Identify", meta=(Categories="Fragment"))
	FGameplayTag FragmentTag;

	/** Tags used for dynamic fragment filtering and queries. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Fragment|Identify")
	FGameplayTagContainer FragmentDynamicTags;
};
