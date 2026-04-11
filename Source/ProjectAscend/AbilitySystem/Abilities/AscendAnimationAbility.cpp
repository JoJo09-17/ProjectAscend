#include "AbilitySystem/Abilities/AscendAnimationAbility.h"
#include "AbilitySystem/AbilityTasks/AbilityTask_PlayMontageWaitEvent.h"
#include "AbilitySystem/Providers/AscendAnimationProvider.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAnimationAbility)

UAscendAnimationAbility::UAscendAnimationAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAscendAnimationAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ShouldCommitAnimationAbility() && !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FGameplayEventData EmptyEventData;
	if (!StartAnimationAbilityTask(TriggerEventData ? *TriggerEventData : EmptyEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UAscendAnimationAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ActiveMontageTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAscendAnimationAbility::StartAnimationAbilityTask(const FGameplayEventData& TriggerEventData)
{
	if (ActiveMontageTask)
	{
		return false;
	}

	FAscendAbilityAnimationPlaybackData PlaybackData;
	if (!ResolveAnimationPlaybackData(PlaybackData) || !PlaybackData.IsValid())
	{
		return false;
	}

	const FGameplayTagContainer EventTags = GetAnimationEventTags();
	ActiveMontageTask = UAbilityTask_PlayMontageWaitEvent::CreatePlayMontageAndWaitProxyTags(
		this,
		MontageTaskInstanceName,
		PlaybackData.Montage,
		EventTags,
		PlaybackData.PlayRate,
		PlaybackData.StartSection,
		PlaybackData.bStopWhenAbilityEnds,
		PlaybackData.RootMotionScale,
		PlaybackData.StartTimeSeconds,
		PlaybackData.bAllowInterruptAfterBlendOut,
		PlaybackData.bOnlyMatchExactEventTags);

	if (!ActiveMontageTask)
	{
		return false;
	}

	ActiveMontageTask->OnEvent.AddDynamic(this, &UAscendAnimationAbility::HandleMontageEvent);
	ActiveMontageTask->OnCompleted.AddDynamic(this, &UAscendAnimationAbility::HandleMontageCompleted);
	ActiveMontageTask->OnBlendOut.AddDynamic(this, &UAscendAnimationAbility::HandleMontageBlendOut);
	ActiveMontageTask->OnInterrupted.AddDynamic(this, &UAscendAnimationAbility::HandleMontageInterrupted);
	ActiveMontageTask->OnCancelled.AddDynamic(this, &UAscendAnimationAbility::HandleMontageCancelled);
	ActiveMontageTask->ReadyForActivation();

	return true;
}

void UAscendAnimationAbility::StopAnimationAbility(bool bWasCancelled)
{
	if (IsActive())
	{
		K2_EndAbility();
	}
}

bool UAscendAnimationAbility::JumpToMontageSection(FName SectionName)
{
	if (SectionName == NAME_None)
	{
		return false;
	}

	UAnimMontage* PlayingMontage = GetCurrentMontage();
	if (!PlayingMontage)
	{
		return false;
	}

	MontageJumpToSection(SectionName);
	return true;
}

void UAscendAnimationAbility::OnAnimationEventReceivedNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
}

void UAscendAnimationAbility::OnAnimationCompletedNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
}

void UAscendAnimationAbility::OnAnimationBlendOutNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
}

void UAscendAnimationAbility::OnAnimationInterruptedNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
}

void UAscendAnimationAbility::OnAnimationCancelledNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
}

bool UAscendAnimationAbility::ResolveAnimationPlaybackData_Implementation(FAscendAbilityAnimationPlaybackData& OutPlaybackData) const
{
	const UAscendAnimationProvider* AnimationProvider = GetAnimationProvider();
	if (!AnimationProvider)
	{
		return false;
	}

	OutPlaybackData.Montage = AnimationProvider->GetMontageToPlay(const_cast<UAscendAnimationAbility*>(this));
	OutPlaybackData.StartSection = AnimationProvider->GetSectionName(const_cast<UAscendAnimationAbility*>(this));
	OutPlaybackData.PlayRate = AnimationProvider->GetPlayRate(const_cast<UAscendAnimationAbility*>(this));
	OutPlaybackData.RootMotionScale = AnimationProvider->GetRootMotionScale(const_cast<UAscendAnimationAbility*>(this));

	return OutPlaybackData.IsValid();
}

FGameplayTagContainer UAscendAnimationAbility::GetAnimationEventTags_Implementation() const
{
	return DefaultEventTags;
}

bool UAscendAnimationAbility::ShouldCommitAnimationAbility_Implementation() const
{
	return true;
}

bool UAscendAnimationAbility::ShouldEndAbilityOnCompleted_Implementation() const
{
	return true;
}

bool UAscendAnimationAbility::ShouldEndAbilityOnBlendOut_Implementation() const
{
	return false;
}

bool UAscendAnimationAbility::ShouldEndAbilityOnCancelled_Implementation() const
{
	return true;
}

bool UAscendAnimationAbility::ShouldEndAbilityOnInterrupted_Implementation() const
{
	return true;
}

void UAscendAnimationAbility::HandleMontageEvent(FGameplayTag EventTag, FGameplayEventData EventData)
{
	OnAnimationEventReceivedNative(EventTag, EventData);
	OnAnimationEventReceived.Broadcast(EventTag, EventData);
	K2_OnAnimationEventReceived(EventTag, EventData);
}

void UAscendAnimationAbility::HandleMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	ActiveMontageTask = nullptr;
	OnAnimationCompletedNative(EventTag, EventData);
	OnAnimationCompleted.Broadcast(EventTag, EventData);
	K2_OnAnimationCompleted(EventTag, EventData);

	if (ShouldEndAbilityOnCompleted())
	{
		K2_EndAbility();
	}
}

void UAscendAnimationAbility::HandleMontageBlendOut(FGameplayTag EventTag, FGameplayEventData EventData)
{
	OnAnimationBlendOutNative(EventTag, EventData);
	OnAnimationBlendOut.Broadcast(EventTag, EventData);
	K2_OnAnimationBlendOut(EventTag, EventData);

	if (ShouldEndAbilityOnBlendOut())
	{
		K2_EndAbility();
	}
}

void UAscendAnimationAbility::HandleMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	ActiveMontageTask = nullptr;
	OnAnimationInterruptedNative(EventTag, EventData);
	OnAnimationInterrupted.Broadcast(EventTag, EventData);
	K2_OnAnimationInterrupted(EventTag, EventData);

	if (ShouldEndAbilityOnInterrupted())
	{
		K2_EndAbility();
	}
}

void UAscendAnimationAbility::HandleMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	ActiveMontageTask = nullptr;
	OnAnimationCancelledNative(EventTag, EventData);
	OnAnimationCancelled.Broadcast(EventTag, EventData);
	K2_OnAnimationCancelled(EventTag, EventData);

	if (ShouldEndAbilityOnCancelled())
	{
		K2_EndAbility();
	}
}
