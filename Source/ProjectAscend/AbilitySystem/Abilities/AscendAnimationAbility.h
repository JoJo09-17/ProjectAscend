#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AscendAnimationAbility.generated.h"

class UAbilityTask_PlayMontageWaitEvent;
class UAnimMontage;

/** Playback configuration for a montage-based ability. Resolved at activation time. */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendAbilityAnimationPlaybackData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	FName StartSection = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	float RootMotionScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	float StartTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	bool bStopWhenAbilityEnds = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	bool bAllowInterruptAfterBlendOut = false;

	/** When true, event tags must match exactly rather than partially. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Animation")
	bool bOnlyMatchExactEventTags = false;

	bool IsValid() const
	{
		return Montage != nullptr && PlayRate > 0.0f;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAscendAnimationAbilityEventSignature, FGameplayTag, EventTag, const FGameplayEventData&, EventData);

/**
 * Montage-driven ability base class using the provider pattern for animation resolution.
 * Manages the full montage lifecycle (play, blend-out, interrupt, cancel) with
 * three-tier dispatch: native virtuals -> Blueprint delegates -> Blueprint implementable events.
 */
UCLASS(Abstract)
class PROJECTASCEND_API UAscendAnimationAbility : public UAscendGameplayAbility
{
	GENERATED_BODY()

public:
	UAscendAnimationAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	/**
	 * Resolves playback data and starts the montage task.
	 * @param TriggerEventData Event data from the ability trigger.
	 * @return True if the montage task was successfully created and activated.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Animation")
	bool StartAnimationAbilityTask(const FGameplayEventData& TriggerEventData);

	/** Ends the ability (and consequently the montage) if currently active. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Animation")
	void StopAnimationAbility(bool bWasCancelled);

	/** Jumps to a named section within the currently playing montage. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Animation")
	bool JumpToMontageSection(FName SectionName);

protected:
	// Native callbacks for C++ subclass logic.
	virtual void OnAnimationEventReceivedNative(FGameplayTag EventTag, const FGameplayEventData& EventData);
	virtual void OnAnimationCompletedNative(FGameplayTag EventTag, const FGameplayEventData& EventData);
	virtual void OnAnimationBlendOutNative(FGameplayTag EventTag, const FGameplayEventData& EventData);
	virtual void OnAnimationInterruptedNative(FGameplayTag EventTag, const FGameplayEventData& EventData);
	virtual void OnAnimationCancelledNative(FGameplayTag EventTag, const FGameplayEventData& EventData);

	/**
	 * Resolves the montage playback data for this activation.
	 * Default implementation delegates to the animation provider on the definition.
	 * @param OutPlaybackData Receives the resolved playback configuration.
	 * @return True if valid playback data was resolved.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ResolveAnimationPlaybackData(FAscendAbilityAnimationPlaybackData& OutPlaybackData) const;

	/** Returns the gameplay event tags the montage task should listen for. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	FGameplayTagContainer GetAnimationEventTags() const;

	/** Whether the ability should commit cost/cooldown before starting the montage. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ShouldCommitAnimationAbility() const;

	/** Whether the ability should end when the montage completes naturally. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ShouldEndAbilityOnCompleted() const;

	/** Whether the ability should end when the montage blends out. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ShouldEndAbilityOnBlendOut() const;

	/** Whether the ability should end when the montage is cancelled. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ShouldEndAbilityOnCancelled() const;

	/** Whether the ability should end when the montage is interrupted. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Ability|Animation")
	bool ShouldEndAbilityOnInterrupted() const;

	// Blueprint implementable events for designer-side montage callbacks.
	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|Ability|Animation")
	void K2_OnAnimationEventReceived(FGameplayTag EventTag, const FGameplayEventData& EventData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|Ability|Animation")
	void K2_OnAnimationCompleted(FGameplayTag EventTag, const FGameplayEventData& EventData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|Ability|Animation")
	void K2_OnAnimationBlendOut(FGameplayTag EventTag, const FGameplayEventData& EventData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|Ability|Animation")
	void K2_OnAnimationInterrupted(FGameplayTag EventTag, const FGameplayEventData& EventData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|Ability|Animation")
	void K2_OnAnimationCancelled(FGameplayTag EventTag, const FGameplayEventData& EventData);

	UFUNCTION()
	void HandleMontageEvent(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void HandleMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void HandleMontageBlendOut(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void HandleMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void HandleMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Ability|Animation")
	FName MontageTaskInstanceName = TEXT("AnimationAbilityTask");

	/** Fallback event tags if no override is provided via GetAnimationEventTags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Ability|Animation")
	FGameplayTagContainer DefaultEventTags;

	/** Broadcast when a gameplay event is received during montage playback. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|Ability|Animation")
	FAscendAnimationAbilityEventSignature OnAnimationEventReceived;

	/** Broadcast when the montage completes naturally. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|Ability|Animation")
	FAscendAnimationAbilityEventSignature OnAnimationCompleted;

	/** Broadcast when the montage begins blending out. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|Ability|Animation")
	FAscendAnimationAbilityEventSignature OnAnimationBlendOut;

	/** Broadcast when the montage is interrupted. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|Ability|Animation")
	FAscendAnimationAbilityEventSignature OnAnimationInterrupted;

	/** Broadcast when the montage is cancelled. */
	UPROPERTY(BlueprintAssignable, Category = "Ascend|Ability|Animation")
	FAscendAnimationAbilityEventSignature OnAnimationCancelled;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageWaitEvent> ActiveMontageTask = nullptr;
};
