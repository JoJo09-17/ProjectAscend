#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_PlayMontageWaitEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * Plays a montage and listens for gameplay events matching a tag container.
 * Broadcasts delegates on completion, blend-out, interruption, cancellation, and matched events.
 * Supports root motion scaling, exact/partial tag matching, and blend-out interrupt tolerance.
 */
UCLASS(meta = (DisplayName = "Ability Task: Play Montage Wait Event"))
class PROJECTASCEND_API UAbilityTask_PlayMontageWaitEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	/** Creates the task with a tag container for event matching. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "PlayMontageAndWaitForEvents", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageWaitEvent* CreatePlayMontageAndWaitProxyTags(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay, FGameplayTagContainer EventTags, float Rate = 1.0f, FName StartSection = NAME_None, bool bStopWhenAbilityEnds = true, float AnimRootMotionTranslationScale = 1.0f, float StartTimeSeconds = 0.0f, bool bAllowInterruptAfterBlendOut = false, bool bOnlyExact = false);

	/** Creates the task with a single tag for event matching. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "PlayMontageAndWaitForEvent", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_PlayMontageWaitEvent* CreatePlayMontageAndWaitProxy(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* MontageToPlay, FGameplayTag EventTag, float Rate = 1.0f, FName StartSection = NAME_None, bool bStopWhenAbilityEnds = true, float AnimRootMotionTranslationScale = 1.0f, float StartTimeSeconds = 0.0f, bool bAllowInterruptAfterBlendOut = false, bool bOnlyExact = false);

	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;

	void GameplayEventContainerCallback(FGameplayTag GameplayTag, const FGameplayEventData* GameplayEventData);

	/** Broadcast when the montage completes naturally (reaches the end). */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCompleted;

	/** Broadcast when the montage begins blending out. */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnBlendOut;

	/** Broadcast when the montage is interrupted. */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnInterrupted;

	/** Broadcast when the task is cancelled externally. */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnCancelled;

	/** Broadcast when a gameplay event matching the tag filter is received. */
	UPROPERTY(BlueprintAssignable)
	FPlayMontageAndWaitForEventDelegate OnEvent;

protected:
	virtual void OnDestroy(bool AbilityEnded) override;

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnGameplayAbilityCancelled();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	bool StopPlayingMontage();

	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;

	UPROPERTY()
	FGameplayTagContainer EventTags;

	UPROPERTY()
	float Rate;

	UPROPERTY()
	FName StartSection;

	UPROPERTY()
	float StartTimeSeconds;

	UPROPERTY()
	float AnimRootMotionTranslationScale;

	UPROPERTY()
	bool bStopWhenAbilityEnds;

	UPROPERTY()
	bool bAllowInterruptAfterBlendOut;

	/** When true, gameplay events must match tags exactly rather than partially. */
	UPROPERTY()
	bool bOnlyExact;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle InterruptedHandle;
	FDelegateHandle SingleHandle;
};
