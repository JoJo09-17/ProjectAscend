#include "AbilitySystem/Abilities/AscendLeapSlamAbility.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionJumpForce.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Engine/OverlapResult.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerController.h"
#include "Player/AscendPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendLeapSlamAbility)

UAscendLeapSlamAbility::UAscendLeapSlamAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAscendLeapSlamAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Cache target location before Super::ActivateAbility commits and starts montage playback.
	bHasCachedActivationTarget = ResolveLeapTargetLocation(CachedActivationTargetLocation);
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAscendLeapSlamAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	CleanupJumpTask();
	bLeapStarted = false;
	bLandingEffectsApplied = false;
	bLandingSectionTriggered = false;
	CachedTargetLocation = FVector::ZeroVector;
	CachedTravelDistance = 0.0f;
	CachedTravelDuration = 0.0f;
	CachedActivationTargetLocation = FVector::ZeroVector;
	bHasCachedActivationTarget = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UAscendLeapSlamAbility::ResolveAnimationPlaybackData_Implementation(FAscendAbilityAnimationPlaybackData& OutPlaybackData) const
{
	if (!Super::ResolveAnimationPlaybackData_Implementation(OutPlaybackData))
	{
		return false;
	}

	// Scale montage play rate by AttackSpeed so animation matches accelerated travel.
	OutPlaybackData.PlayRate *= GetLeapAttackSpeedScalar();
	if (StartMontageSection != NAME_None)
	{
		OutPlaybackData.StartSection = StartMontageSection;
	}
	return OutPlaybackData.IsValid();
}

FGameplayTagContainer UAscendLeapSlamAbility::GetAnimationEventTags_Implementation() const
{
	FGameplayTagContainer EventTags = Super::GetAnimationEventTags_Implementation();
	if (JumpTriggerEventTag.IsValid())
	{
		EventTags.AddTag(JumpTriggerEventTag);
	}

	return EventTags;
}

// Only end ability after landing has been processed; the montage may blend out during air phase.
bool UAscendLeapSlamAbility::ShouldEndAbilityOnCompleted_Implementation() const
{
	return bLandingSectionTriggered;
}

bool UAscendLeapSlamAbility::ShouldEndAbilityOnBlendOut_Implementation() const
{
	return bLandingSectionTriggered;
}

bool UAscendLeapSlamAbility::ShouldEndAbilityOnInterrupted_Implementation() const
{
	return true;
}

void UAscendLeapSlamAbility::OnAnimationEventReceivedNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	Super::OnAnimationEventReceivedNative(EventTag, EventData);

	if (!bLeapStarted && JumpTriggerEventTag.IsValid() && EventTag.MatchesTagExact(JumpTriggerEventTag))
	{
		StartLeapMovement();

		if (AirMontageSection != NAME_None)
		{
			JumpToMontageSection(AirMontageSection);
		}
	}
}

void UAscendLeapSlamAbility::OnAnimationCancelledNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	CleanupJumpTask();
	Super::OnAnimationCancelledNative(EventTag, EventData);
}

void UAscendLeapSlamAbility::OnAnimationInterruptedNative(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	CleanupJumpTask();
	Super::OnAnimationInterruptedNative(EventTag, EventData);
}

void UAscendLeapSlamAbility::HandleJumpFinished()
{
	ActiveJumpTask = nullptr;

	// If landing effects weren't applied yet (landed callback never fired), handle it here.
	if (!bLandingEffectsApplied)
	{
		HandleJumpLanded();
		return;
	}
}

void UAscendLeapSlamAbility::HandleJumpLanded()
{
	ActiveJumpTask = nullptr;
	ApplyLandingEffects();

	if (!bLandingSectionTriggered && LandingMontageSection != NAME_None && JumpToMontageSection(LandingMontageSection))
	{
		bLandingSectionTriggered = true;
		return;
	}

	bLandingSectionTriggered = true;

	if (IsActive())
	{
		K2_EndAbility();
	}
}

bool UAscendLeapSlamAbility::ResolveLeapTargetLocation(FVector& OutTargetLocation) const
{
	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	if (bHasCachedActivationTarget)
	{
		OutTargetLocation = CachedActivationTargetLocation;
		return true;
	}

	// Try AscendPlayerController first (custom cursor tracking), then fall back to standard trace.
	FVector DesiredTargetLocation = FVector::ZeroVector;
	if (AAscendPlayerController* AscendPlayerController = Cast<AAscendPlayerController>(GetCurrentActorInfo()->PlayerController.Get()))
	{
		DesiredTargetLocation = AscendPlayerController->GetMouseHitLocation();
	}
	else if (APlayerController* BasePlayerController = Cast<APlayerController>(GetCurrentActorInfo()->PlayerController.Get()))
	{
		FHitResult HitResult;
		BasePlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult);
		if (HitResult.bBlockingHit)
		{
			DesiredTargetLocation = HitResult.ImpactPoint;
		}
	}

	if (DesiredTargetLocation.IsNearlyZero())
	{
		DesiredTargetLocation = AvatarActor->GetActorLocation() + AvatarActor->GetActorForwardVector() * MaxLeapDistance;
	}

	const FVector StartLocation = AvatarActor->GetActorLocation();
	FVector ToTarget = DesiredTargetLocation - StartLocation;
	ToTarget.Z = 0.0f;

	const float RawDistance = ToTarget.Size();
	if (RawDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float ClampedDistance = FMath::Clamp(RawDistance, MinLeapDistance, MaxLeapDistance);
	OutTargetLocation = StartLocation + ToTarget.GetSafeNormal() * ClampedDistance;
	OutTargetLocation.Z = DesiredTargetLocation.Z;
	return true;
}

float UAscendLeapSlamAbility::GetLeapAttackSpeedScalar() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return 1.0f;
	}

	const float AttackSpeed = ASC->GetNumericAttribute(UAscendAttributeSet::GetAttackSpeedAttribute());
	return AttackSpeed > 0.0f ? AttackSpeed : 1.0f;
}

void UAscendLeapSlamAbility::ApplyLandingEffects()
{
	if (bLandingEffectsApplied || !LandingEffectContainerTag.IsValid())
	{
		return;
	}

	bLandingEffectsApplied = true;
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	const FVector LandingLocation = AvatarActor->GetActorLocation();
	ExecuteGameplayCueOnOwner(LeapLandingGameplayCueTag, MakeGameplayCueParameters(LandingLocation));

	FAscendGameplayEffectContainer LandingContainer;
	if (!GetEffectContainer(LandingEffectContainerTag, LandingContainer))
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.Instigator = AvatarActor;
	EventData.ContextHandle = MakeEffectContext(CurrentSpecHandle, GetCurrentActorInfo());

	FHitResult LandingHitResult;
	LandingHitResult.Location = LandingLocation;
	LandingHitResult.ImpactPoint = LandingLocation;
	LandingHitResult.TraceStart = LandingLocation;
	LandingHitResult.TraceEnd = LandingLocation;
	if (FGameplayEffectContext* EffectContext = EventData.ContextHandle.Get())
	{
		EffectContext->AddHitResult(LandingHitResult, true);
	}

	FAscendGameplayEffectContainerSpec ContainerSpec;
	if (bBuildLandingTargetDataDirectly)
	{
		// Direct target data is an explicit bypass for abilities that want to own overlap logic locally.
		// When disabled, TargetType remains the single source of truth for landing target resolution.
		FGameplayAbilityTargetDataHandle TargetData;
		BuildLandingTargetData(LandingLocation, TargetData);
		EventData.TargetData = TargetData;

		if (TargetData.Num() > 0)
		{
			if (const FGameplayAbilityTargetData* FirstTargetData = TargetData.Get(0))
			{
				if (const TArray<TWeakObjectPtr<AActor>> FirstActors = FirstTargetData->GetActors();
					FirstActors.Num() > 0 && FirstActors[0].IsValid())
				{
					EventData.Target = FirstActors[0].Get();
				}
			}
		}

		FAscendGameplayEffectContainer DirectTargetContainer = LandingContainer;
		DirectTargetContainer.TargetType.Reset();
		ContainerSpec = MakeEffectContainerSpecFromContainer(DirectTargetContainer, EventData);
	}
	else
	{
		ContainerSpec = MakeEffectContainerSpecFromContainer(LandingContainer, EventData);
	}

	SetSetByCallerMagnitudeOnContainerSpec(
		ContainerSpec,
		AscendGameplayTags::SetByCaller_DamageMultiplier,
		CalculateLandingDamageMultiplier());
	ApplyEffectContainerSpec(ContainerSpec);
}

bool UAscendLeapSlamAbility::StartLeapMovement()
{
	if (bLeapStarted || !ResolveLeapTargetLocation(CachedTargetLocation))
	{
		return false;
	}

	const AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	const FVector StartLocation = AvatarActor->GetActorLocation();
	FVector ToTarget = CachedTargetLocation - StartLocation;
	ToTarget.Z = 0.0f;

	CachedTravelDistance = ToTarget.Size();
	if (CachedTravelDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// Scale travel speed by AttackSpeed so higher APS = faster leaps with shorter air time.
	const float AttackSpeedScalar = GetLeapAttackSpeedScalar();
	const float TravelSpeed = FMath::Max(BaseTravelSpeed * AttackSpeedScalar, 1.0f);
	CachedTravelDuration = FMath::Clamp(CachedTravelDistance / TravelSpeed, MinTravelDuration, MaxTravelDuration);

	const FRotator JumpRotation = ToTarget.Rotation();
	const FRotator FacingRotation(0.0f, JumpRotation.Yaw, 0.0f);
	if (AActor* MutableAvatarActor = GetAvatarActorFromActorInfo())
	{
		MutableAvatarActor->SetActorRotation(FacingRotation);
	}

	ActiveJumpTask = UAbilityTask_ApplyRootMotionJumpForce::ApplyRootMotionJumpForce(
		this,
		TEXT("LeapSlamJump"),
		JumpRotation,
		CachedTravelDistance,
		BaseJumpHeight,
		CachedTravelDuration,
		MinimumLandedTriggerTime,
		true,
		FinishVelocityMode,
		FinishSetVelocity,
		FinishClampVelocity,
		JumpPathOffsetCurve,
		JumpTimeMappingCurve);

	if (!ActiveJumpTask)
	{
		return false;
	}

	ExecuteGameplayCueOnOwner(LeapStartGameplayCueTag, MakeGameplayCueParameters(StartLocation));
	bLeapStarted = true;
	bLandingSectionTriggered = false;
	ActiveJumpTask->OnFinish.AddDynamic(this, &UAscendLeapSlamAbility::HandleJumpFinished);
	ActiveJumpTask->OnLanded.AddDynamic(this, &UAscendLeapSlamAbility::HandleJumpLanded);
	ActiveJumpTask->ReadyForActivation();
	return true;
}

void UAscendLeapSlamAbility::CleanupJumpTask()
{
	if (ActiveJumpTask)
	{
		ActiveJumpTask->EndTask();
		ActiveJumpTask = nullptr;
	}
}

void UAscendLeapSlamAbility::BuildLandingTargetData(const FVector& LandingLocation, FGameplayAbilityTargetDataHandle& OutTargetData) const
{
	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(LandingQueryChannel);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AscendLeapSlamOverlap), false, GetAvatarActorFromActorInfo());

	if (!GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		LandingLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(LandingDamageRadius),
		QueryParams))
	{
		return;
	}

	TArray<AActor*> TargetActors;
	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* TargetActor = Result.GetActor();
		if (!TargetActor || TargetActor == GetAvatarActorFromActorInfo())
		{
			continue;
		}

		TargetActors.AddUnique(TargetActor);
	}

	if (TargetActors.Num() == 0)
	{
		return;
	}

	FGameplayAbilityTargetData_ActorArray* ActorArrayData = new FGameplayAbilityTargetData_ActorArray();
	for (AActor* TargetActor : TargetActors)
	{
		ActorArrayData->TargetActorArray.Add(TargetActor);
	}
	OutTargetData.Add(ActorArrayData);
}

float UAscendLeapSlamAbility::CalculateLandingDamageMultiplier() const
{
	if (MaxLeapDistance <= 0.0f)
	{
		return MinDistanceDamageMultiplier;
	}

	const float DistanceAlpha = FMath::Clamp(CachedTravelDistance / MaxLeapDistance, 0.0f, 1.0f);
	return FMath::Lerp(MinDistanceDamageMultiplier, MaxDistanceDamageMultiplier, DistanceAlpha);
}
