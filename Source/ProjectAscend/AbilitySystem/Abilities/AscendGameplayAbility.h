#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendAbilityActivationGroup.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Effects/AscendGameplayEffectTypes.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_InputBinding.h"
#include "AscendGameplayAbility.generated.h"

class UAscendAbilityDefinition;
class UAscendAbilitySystemComponent;
class UAscendAnimationProvider;
struct FAscendAbilityFragment_EffectContainers;
struct FAscendAbilityFragment_AnimationProvider;

/**
 * Base ability class for the Ascend GAS framework.
 * Bridges ability instances to their UAscendAbilityDefinition for data-driven configuration,
 * and provides utility methods for effect containers, gameplay cues, and animation providers.
 */
UCLASS(Abstract)
class PROJECTASCEND_API UAscendGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAscendGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Returns the definition asset that configured this ability instance. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability")
	const UAscendAbilityDefinition* GetAbilityDefinition() const;

	/** Returns the source object associated with this ability's spec handle (e.g. weapon, item). */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability")
	UObject* GetAbilitySourceObject() const;

	/** Returns the ability tags defined on the associated definition asset. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability")
	FGameplayTagContainer GetAbilityDefinitionTags() const;

	/** Returns the owned tags defined on the associated definition asset. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability")
	FGameplayTagContainer GetOwnedTagsFromDefinition() const;

	/**
	 * Builds gameplay cue parameters from the current ability context.
	 * @param Location Optional world location; defaults to avatar actor location if zero.
	 * @return Populated GameplayCueParameters ready for cue execution.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|GameplayCue")
	FGameplayCueParameters MakeGameplayCueParameters(FVector Location = FVector::ZeroVector) const;

	/** Fires a one-shot gameplay cue on the owner's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|GameplayCue")
	void ExecuteGameplayCueOnOwner(const FGameplayTag& GameplayCueTag, FGameplayCueParameters Parameters);

	/** Adds a persistent gameplay cue on the owner's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|GameplayCue")
	void AddGameplayCueOnOwner(const FGameplayTag& GameplayCueTag, FGameplayCueParameters Parameters);

	/** Removes a persistent gameplay cue from the owner's ASC. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|GameplayCue")
	void RemoveGameplayCueFromOwner(const FGameplayTag& GameplayCueTag);

	/** Returns the animation provider configured on this ability's definition, if any. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Animation")
	UAscendAnimationProvider* GetAnimationProvider() const;

	/** Looks up an effect container by tag from the definition's fragment data. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects")
	bool GetEffectContainer(FGameplayTag ContainerTag, FAscendGameplayEffectContainer& OutContainer) const;

	/**
	 * Creates effect specs from a raw container, resolving targets via the container's TargetType.
	 * @param Container The effect container to convert.
	 * @param EventData Event data used for target resolution.
	 * @param OverrideGameplayLevel Overrides the ability level for effect spec creation; uses current level if -1.
	 * @return A populated container spec ready for application.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects", meta = (AutoCreateRefTerm = "EventData"))
	FAscendGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(
		const FAscendGameplayEffectContainer& Container,
		const FGameplayEventData& EventData,
		int32 OverrideGameplayLevel = -1);

	/** Convenience wrapper: resolves a container by tag, then creates specs from it. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects", meta = (AutoCreateRefTerm = "EventData"))
	FAscendGameplayEffectContainerSpec MakeEffectContainerSpec(
		FGameplayTag ContainerTag,
		const FGameplayEventData& EventData,
		int32 OverrideGameplayLevel = -1);

	/** Applies all effect specs in the container to their resolved targets. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects")
	TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FAscendGameplayEffectContainerSpec& ContainerSpec);

	/** Writes a SetByCaller value to every outgoing effect spec contained in the container spec. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects")
	void SetSetByCallerMagnitudeOnContainerSpec(
		UPARAM(ref) FAscendGameplayEffectContainerSpec& ContainerSpec,
		FGameplayTag DataTag,
		float Magnitude) const;

	/** Convenience wrapper: resolves container by tag, creates specs, and applies them in one call. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Effects", meta = (AutoCreateRefTerm = "EventData"))
	TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(
		FGameplayTag ContainerTag,
		const FGameplayEventData& EventData,
		int32 OverrideGameplayLevel = -1);

	/** Returns the activation policy from the definition's input binding fragment. */
	EAscendAbilityActivationPolicy GetActivationPolicy() const;

	/** Returns how this ability participates in owner-side activation concurrency. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ascend|Ability")
	EAscendAbilityActivationGroup GetActivationGroup() const { return ActivationGroup; }

	/**
	 * Extends GAS activation checks with semantic ability-tag relationships defined outside the ability class.
	 * @return True if the ability passes both standard GAS checks and relationship-based requirements.
	 */
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void PreActivate(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Controls whether this ability can coexist with other action abilities on the same ASC. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Ability")
	EAscendAbilityActivationGroup ActivationGroup = EAscendAbilityActivationGroup::Independent;

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Ability", Transient)
	mutable TObjectPtr<const UAscendAbilityDefinition> AbilityDefinition;

	const FAscendAbilityFragment_EffectContainers* GetEffectContainersFragment() const;
	const FAscendAbilityFragment_AnimationProvider* GetAnimationProviderFragment() const;
	void SetSetByCallerMagnitudeOnEffectSpec(FGameplayEffectSpecHandle& EffectSpecHandle, FGameplayTag DataTag, float Magnitude) const;
};
