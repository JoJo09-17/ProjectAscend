#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AscendAbilityTagRelationshipMapping.generated.h"

/**
 * Describes how a semantic ability tag affects other abilities and activation requirements.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendAbilityTagRelationshipItem
{
	GENERATED_BODY()

	/** Ability tag this relationship entry applies to. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	/** Abilities matching these tags will be blocked while this ability is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTagContainer AbilityTagsToBlock;

	/** Abilities matching these tags will be cancelled when this ability is applied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTagContainer AbilityTagsToCancel;

	/** Owner must have these tags before the ability can activate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTagContainer ActivationRequiredTags;

	/** Owner cannot have these tags if the ability wants to activate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTagContainer ActivationBlockedTags;
};

/**
 * Adds a conditional layer of relationship rules that only applies when the owner matches the query.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendAbilityTagRelationshipLayer
{
	GENERATED_BODY()

	/** Owner tags must satisfy this query before any layered rules are evaluated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	FGameplayTagQuery OwnerTagQuery;

	/** Relationship rules that override or augment the base mapping for matching owners. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TArray<FAscendAbilityTagRelationshipItem> Relationships;
};

/**
 * Central mapping of semantic ability tags to block, cancel, and activation requirement rules.
 */
UCLASS(BlueprintType)
class PROJECTASCEND_API UAscendAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Collects additional block and cancel tags for the supplied semantic ability tags.
	 *
	 * @param AbilityTags Ability tags being evaluated.
	 * @param OutTagsToBlock Receives additional tags to block.
	 * @param OutTagsToCancel Receives additional tags to cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void GetAbilityTagsToBlockAndCancel(
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutTagsToBlock,
		FGameplayTagContainer& OutTagsToCancel) const;

	/**
	 * Collects additional block and cancel tags, optionally evaluating layered rules against owner tags.
	 * @param OwnerTags Gameplay tags currently owned by the ability owner.
	 * @param AbilityTags Semantic ability tags being evaluated.
	 * @param OutTagsToBlock Receives additional tags to block.
	 * @param OutTagsToCancel Receives additional tags to cancel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void GetAbilityTagsToBlockAndCancelForOwner(
		const FGameplayTagContainer& OwnerTags,
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutTagsToBlock,
		FGameplayTagContainer& OutTagsToCancel) const;

	/**
	 * Collects additional required and blocked owner tags for the supplied semantic ability tags.
	 *
	 * @param AbilityTags Ability tags being evaluated.
	 * @param OutActivationRequired Receives extra required owner tags.
	 * @param OutActivationBlocked Receives extra blocked owner tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void GetActivationRequiredAndBlockedTags(
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutActivationRequired,
		FGameplayTagContainer& OutActivationBlocked) const;

	/**
	 * Collects additional activation requirements, optionally evaluating layered rules against owner tags.
	 * @param OwnerTags Gameplay tags currently owned by the ability owner.
	 * @param AbilityTags Semantic ability tags being evaluated.
	 * @param OutActivationRequired Receives extra required owner tags.
	 * @param OutActivationBlocked Receives extra blocked owner tags.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void GetActivationRequiredAndBlockedTagsForOwner(
		const FGameplayTagContainer& OwnerTags,
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutActivationRequired,
		FGameplayTagContainer& OutActivationBlocked) const;

private:
	void GetAbilityTagsToBlockAndCancelFromItems(
		const TArray<FAscendAbilityTagRelationshipItem>& Items,
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutTagsToBlock,
		FGameplayTagContainer& OutTagsToCancel,
		TArray<FGameplayTag>* OutHandledAbilityTags = nullptr) const;

	void GetActivationRequiredAndBlockedTagsFromItems(
		const TArray<FAscendAbilityTagRelationshipItem>& Items,
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutActivationRequired,
		FGameplayTagContainer& OutActivationBlocked,
		TArray<FGameplayTag>* OutHandledAbilityTags = nullptr) const;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<FAscendAbilityTagRelationshipItem> Relationships;

	UPROPERTY(EditAnywhere, Category = "Abilities")
	TArray<FAscendAbilityTagRelationshipLayer> ConditionalLayers;
};
