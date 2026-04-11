#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendAbilityActivationGroup.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AscendAbilitySystemComponent.generated.h"

class UAscendAbilityDefinition;
class UAscendAbilityTagRelationshipMapping;
struct FAscendAbilityFragment_InputBinding;
enum class EAscendAbilityActivationPolicy : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTagChangedSignature, const FGameplayTag&, Tag, bool, bTagAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAbilitySlotChangedSignature, const FGameplayTag&, SlotTag, FGameplayAbilitySpecHandle, AbilityHandle, bool, bOccupied);

/**
 * Extended AbilitySystemComponent providing definition-based ability management,
 * a runtime slot system (ability bar), and a frame-buffered input processing pipeline.
 */
UCLASS()
class PROJECTASCEND_API UAscendAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UAscendAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Flushes buffered input events and activates abilities according to their activation policy. */
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	/** Resets all buffered input state and releases pressed specs. */
	void ClearAbilityInput();

	/** Buffers a pressed event for all abilities matching the given input tag. */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	/** Buffers a released event for all abilities matching the given input tag. */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	/** Routes a confirm input to any ability waiting for target data confirmation. */
	void HandleConfirmInput();
	/** Routes a cancel input to any ability waiting for target data confirmation. */
	void HandleCancelInput();

	/** Increments the pending-confirm counter (called by abilities entering a targeting state). */
	void NotifyWaitingForConfirm();
	/** Decrements the pending-confirm counter (called when a confirm/cancel resolves). */
	void NotifyConfirmComplete();
	/** @return True if at least one ability is waiting for target confirmation. */
	bool HasPendingConfirm() const;
	/** @return True if the owner has a gameplay tag that blocks ability input. */
	bool IsAbilityInputBlocked() const;
	/** @return True if an ability in the requested activation group cannot start right now. */
	bool IsActivationGroupBlocked(EAscendAbilityActivationGroup Group) const;

	/** Returns all slot input tags configured on this ASC. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const;

	/** Grants an ability from a definition to this ASC. Blueprint-friendly entry point. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities", DisplayName = "Give Ascend Ability")
	FGameplayAbilitySpecHandle K2_GiveAscendAbility(const UAscendAbilityDefinition* Definition, UObject* SourceObject = nullptr);

	/** Grants an ability spec enriched by the given definition and registers all associated metadata. */
	FGameplayAbilitySpecHandle GiveAscendAbility(FGameplayAbilitySpec& AbilitySpec, const UAscendAbilityDefinition* AbilityDefinition, UObject* SourceObject = nullptr);

	/** Retrieves the ability definition associated with a spec handle. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	const UAscendAbilityDefinition* GetDefinitionForHandle(FGameplayAbilitySpecHandle Handle) const;

	/** Retrieves the source object stored when the ability was granted. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	UObject* GetSourceObjectForHandle(FGameplayAbilitySpecHandle Handle) const;

	/** Checks whether the given spec handle maps to a granted ability. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool HasAbilityHandle(FGameplayAbilitySpecHandle AbilityHandle) const;

	/** Collects all dynamic input tags (including the assigned slot tag) for an ability. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetDynamicInputTagsForAbility(FGameplayAbilitySpecHandle AbilityHandle, FGameplayTagContainer& OutInputTags) const;

	/** Tests whether a specific input tag is present on an ability. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool HasInputTagForAbility(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& InputTag) const;

	/** Collects all ability tags (asset + semantic) for an ability. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetAbilityTagsForAbility(FGameplayAbilitySpecHandle AbilityHandle, FGameplayTagContainer& OutAbilityTags) const;

	/** Retrieves a copy of the granted spec for native callers that need more than the handle. */
	bool GetAbilitySpecCopy(FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilitySpec& OutAbilitySpec) const;

	/** Finds the first granted ability with the requested semantic tag. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool FindFirstAbilityHandleByTag(const FGameplayTag& AbilityTag, FGameplayAbilitySpecHandle& OutAbilityHandle) const;

	/** Finds the first granted ability matching the requested tag set. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool FindFirstAbilityHandleByTags(
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		FGameplayAbilitySpecHandle& OutAbilityHandle) const;

	/** Collects all granted abilities matching the requested tag set. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	int32 FindAbilityHandlesByTags(
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles) const;

	/** Tests whether a specific ability tag is present on an ability. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool HasAbilityTag(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& AbilityTag) const;

	/** Collects all owned tags from the ability definition. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetOwnedTagsForAbility(FGameplayAbilitySpecHandle AbilityHandle, FGameplayTagContainer& OutOwnedTags) const;

	/**
	 * Collects additional activation requirements from the configured relationship mapping.
	 * @param AbilityTags Semantic ability tags being evaluated.
	 * @param OutRequiredTags Receives owner tags that must be present before activation.
	 * @param OutBlockedTags Receives owner tags that prevent activation when present.
	 * @return True if a relationship mapping was available.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetRelationshipActivationTagRequirements(
		const FGameplayTagContainer& AbilityTags,
		FGameplayTagContainer& OutRequiredTags,
		FGameplayTagContainer& OutBlockedTags) const;

	/** Cancels all active abilities matching the tag filter. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	void CancelAbilitiesByAbilityTags(const FGameplayTagContainer& WithTags, const FGameplayTagContainer& WithoutTags, UGameplayAbility* Ignore = nullptr);

	/** Binds an ability to an ability bar slot by input tag. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool AssignAbilityToSlot(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& SlotInputTag, bool bReplaceExisting = true);

	/** Unbinds whatever ability occupies the given slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool ClearAbilitySlot(const FGameplayTag& SlotInputTag);

	/** Appends a dynamic input tag to an ability's spec source tags. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool AddDynamicInputTagToAbility(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& InputTag);

	/** Removes a dynamic input tag from an ability's spec source tags and resets input state. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool RemoveDynamicInputTagFromAbility(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& InputTag);

	/** @return True if the given tag is in the SupportedSlotInputTags config. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool IsSupportedSlotInputTag(const FGameplayTag& SlotInputTag) const;

	/** Resolves the ability handle currently bound to a slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetAbilityHandleForSlot(const FGameplayTag& SlotInputTag, FGameplayAbilitySpecHandle& OutHandle) const;

	/** Resolves the slot input tag an ability is currently bound to. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool GetSlotForAbility(FGameplayAbilitySpecHandle AbilityHandle, FGameplayTag& OutSlotInputTag) const;

	/** Tests whether an ability is bound to a specific slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool IsAbilityAssignedToSlot(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& SlotInputTag) const;

	/** Convenience: resolves the definition for the ability in a given slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	const UAscendAbilityDefinition* GetDefinitionForSlot(const FGameplayTag& SlotInputTag) const;

	/** Convenience: resolves the source object for the ability in a given slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	UObject* GetSourceObjectForSlot(const FGameplayTag& SlotInputTag) const;

	/** @return True if an ability is currently bound to the given slot. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool IsSlotOccupied(const FGameplayTag& SlotInputTag) const;

	/** Moves an ability to a new slot, optionally swapping with the occupant. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool MoveAbilityToSlot(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& NewSlotInputTag, bool bSwapIfOccupied = true);

	/** Swaps the abilities bound to two slots. Falls back to a move if only one is occupied. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	bool SwapAbilitySlots(const FGameplayTag& FirstSlotInputTag, const FGameplayTag& SecondSlotInputTag);

	/** Assigns default slots from each ability's InputBinding fragment. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	int32 AutoAssignDefaultSlotsForGrantedAbilities(bool bReplaceExisting = false);

	/** Attempts to find an UAscendAbilitySystemComponent on the given object via IAbilitySystemInterface or component search. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability System", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	static bool TryGetAscendAbilitySystem(const UObject* Object, UAscendAbilitySystemComponent*& AbilitySystem);

	/** Returns the spawned attribute set of the requested class if it already exists on this ASC. */
	UAttributeSet* FindSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass) const;

	/** Returns an existing attribute set of the requested class or creates one if missing. */
	UAttributeSet* GetOrCreateSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass);

	/** Broadcast when a gameplay tag is added or removed from this ASC. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|AbilitySystem")
	FOnTagChangedSignature OnTagChanged;

	/** Broadcast when an ability is assigned to or removed from a slot. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|AbilitySystem")
	FOnAbilitySlotChangedSignature OnAbilitySlotChanged;

protected:
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	virtual void ApplyAbilityBlockAndCancelTags(
		const FGameplayTagContainer& AbilityTags,
		UGameplayAbility* RequestingAbility,
		bool bEnableBlockTags,
		const FGameplayTagContainer& BlockTags,
		bool bExecuteCancelTags,
		const FGameplayTagContainer& CancelTags) override;

	/** Gameplay tags that, when present on the owner, suppress all ability input. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Input")
	FGameplayTagContainer InputBlockedTags;

	/** Optional override for the project-wide ability tag relationship mapping. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability Tags")
	TObjectPtr<UAscendAbilityTagRelationshipMapping> AbilityTagRelationshipMappingOverride;

private:
	const FGameplayAbilitySpec* GetAbilitySpecByHandle(FGameplayAbilitySpecHandle AbilityHandle) const;
	FGameplayAbilitySpec* GetAbilitySpecByHandle(FGameplayAbilitySpecHandle AbilityHandle);
	EAscendAbilityActivationPolicy GetActivationPolicyForSpec(const FGameplayAbilitySpec& Spec) const;
	const UAscendAbilityTagRelationshipMapping* GetAbilityTagRelationshipMapping() const;
	EAscendAbilityActivationGroup GetActivationGroupForAbility(const UGameplayAbility* Ability) const;
	void AddAbilityToActivationGroup(EAscendAbilityActivationGroup Group, UGameplayAbility* Ability);
	void RemoveAbilityFromActivationGroup(EAscendAbilityActivationGroup Group, UGameplayAbility* Ability);
	void CancelActivationGroupAbilities(EAscendAbilityActivationGroup Group, UGameplayAbility* IgnoreAbility, bool bReplicateCancelAbility);

	void ApplyAbilityDefinitionToSpec(FGameplayAbilitySpec& AbilitySpec, const UAscendAbilityDefinition* AbilityDefinition) const;
	void BuildGrantedStaticInputTags(
		const UAscendAbilityDefinition* AbilityDefinition,
		const FAscendAbilityFragment_InputBinding& InputFragment,
		FGameplayTagContainer& OutStaticInputTags) const;
	void GetEffectiveSupportedSlotTags(FGameplayTagContainer& OutSlotTags) const;
	bool AbilitySpecHasInputTag(const FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag) const;
	void GetAbilityTagsForSpec(const FGameplayAbilitySpec& Spec, FGameplayTagContainer& OutAbilityTags) const;
	const FAscendAbilityFragment_InputBinding* GetSlotBindingFragmentForAbility(FGameplayAbilitySpecHandle AbilityHandle) const;
	void RemoveAllSlotTagsFromSpec(FGameplayAbilitySpec& Spec);
	void ResetInputStateForAbility(FGameplayAbilitySpecHandle AbilityHandle);
	FGameplayTag GetSlotInputTagFromSpec(const FGameplayAbilitySpec& Spec) const;
	void BroadcastSlotChanged(const FGameplayTag& SlotInputTag);

	TMap<FGameplayAbilitySpecHandle, TObjectPtr<UAscendAbilityDefinition>> AbilityDefinitionMap;
	TMap<FGameplayAbilitySpecHandle, TObjectPtr<UObject>> AbilitySourceObjectMap;
	TMap<FGameplayAbilitySpecHandle, FGameplayTag> AbilitySlotMap;
	/** Runtime semantic tags derived from definitions and used by queries, block/cancel, and relationships. */
	TMap<FGameplayAbilitySpecHandle, FGameplayTagContainer> AbilitySemanticTagMap;
	int32 ActivationGroupCounts[(uint8)EAscendAbilityActivationGroup::MAX] = {};
	int32 PendingConfirmCount = 0;

	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;
};
