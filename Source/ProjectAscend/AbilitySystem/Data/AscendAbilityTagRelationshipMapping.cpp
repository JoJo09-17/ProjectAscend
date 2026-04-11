#include "AbilitySystem/Data/AscendAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilityTagRelationshipMapping)

void UAscendAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutTagsToBlock,
	FGameplayTagContainer& OutTagsToCancel) const
{
	const FGameplayTagContainer EmptyOwnerTags;
	GetAbilityTagsToBlockAndCancelForOwner(EmptyOwnerTags, AbilityTags, OutTagsToBlock, OutTagsToCancel);
}

void UAscendAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancelForOwner(
	const FGameplayTagContainer& OwnerTags,
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutTagsToBlock,
	FGameplayTagContainer& OutTagsToCancel) const
{
	TArray<FGameplayTag> LayerHandledAbilityTags;

	// Conditional layers let a single semantic ability tag behave differently for specific owner states.
	for (const FAscendAbilityTagRelationshipLayer& Layer : ConditionalLayers)
	{
		if (!OwnerTags.IsEmpty() && !Layer.OwnerTagQuery.IsEmpty() && Layer.OwnerTagQuery.Matches(OwnerTags))
		{
			GetAbilityTagsToBlockAndCancelFromItems(
				Layer.Relationships,
				AbilityTags,
				OutTagsToBlock,
				OutTagsToCancel,
				&LayerHandledAbilityTags);
		}
	}

	GetAbilityTagsToBlockAndCancelFromItems(Relationships, AbilityTags, OutTagsToBlock, OutTagsToCancel, &LayerHandledAbilityTags);
}

void UAscendAbilityTagRelationshipMapping::GetActivationRequiredAndBlockedTags(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired,
	FGameplayTagContainer& OutActivationBlocked) const
{
	const FGameplayTagContainer EmptyOwnerTags;
	GetActivationRequiredAndBlockedTagsForOwner(EmptyOwnerTags, AbilityTags, OutActivationRequired, OutActivationBlocked);
}

void UAscendAbilityTagRelationshipMapping::GetActivationRequiredAndBlockedTagsForOwner(
	const FGameplayTagContainer& OwnerTags,
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired,
	FGameplayTagContainer& OutActivationBlocked) const
{
	TArray<FGameplayTag> LayerHandledAbilityTags;

	for (const FAscendAbilityTagRelationshipLayer& Layer : ConditionalLayers)
	{
		if (!OwnerTags.IsEmpty() && !Layer.OwnerTagQuery.IsEmpty() && Layer.OwnerTagQuery.Matches(OwnerTags))
		{
			GetActivationRequiredAndBlockedTagsFromItems(
				Layer.Relationships,
				AbilityTags,
				OutActivationRequired,
				OutActivationBlocked,
				&LayerHandledAbilityTags);
		}
	}

	GetActivationRequiredAndBlockedTagsFromItems(
		Relationships,
		AbilityTags,
		OutActivationRequired,
		OutActivationBlocked,
		&LayerHandledAbilityTags);
}

void UAscendAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancelFromItems(
	const TArray<FAscendAbilityTagRelationshipItem>& Items,
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutTagsToBlock,
	FGameplayTagContainer& OutTagsToCancel,
	TArray<FGameplayTag>* OutHandledAbilityTags) const
{
	for (const FAscendAbilityTagRelationshipItem& Relationship : Items)
	{
		if (!Relationship.AbilityTag.IsValid() || !AbilityTags.HasTag(Relationship.AbilityTag))
		{
			continue;
		}

		if (OutHandledAbilityTags && OutHandledAbilityTags->Contains(Relationship.AbilityTag))
		{
			continue;
		}

		OutTagsToBlock.AppendTags(Relationship.AbilityTagsToBlock);
		OutTagsToCancel.AppendTags(Relationship.AbilityTagsToCancel);

		if (OutHandledAbilityTags)
		{
			OutHandledAbilityTags->Add(Relationship.AbilityTag);
		}
	}
}

void UAscendAbilityTagRelationshipMapping::GetActivationRequiredAndBlockedTagsFromItems(
	const TArray<FAscendAbilityTagRelationshipItem>& Items,
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired,
	FGameplayTagContainer& OutActivationBlocked,
	TArray<FGameplayTag>* OutHandledAbilityTags) const
{
	for (const FAscendAbilityTagRelationshipItem& Relationship : Items)
	{
		if (!Relationship.AbilityTag.IsValid() || !AbilityTags.HasTag(Relationship.AbilityTag))
		{
			continue;
		}

		if (OutHandledAbilityTags && OutHandledAbilityTags->Contains(Relationship.AbilityTag))
		{
			continue;
		}

		OutActivationRequired.AppendTags(Relationship.ActivationRequiredTags);
		OutActivationBlocked.AppendTags(Relationship.ActivationBlockedTags);

		if (OutHandledAbilityTags)
		{
			OutHandledAbilityTags->Add(Relationship.AbilityTag);
		}
	}
}
