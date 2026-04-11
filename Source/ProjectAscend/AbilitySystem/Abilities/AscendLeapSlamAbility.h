#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendAnimationAbility.h"
#include "GameFramework/RootMotionSource.h"
#include "AscendLeapSlamAbility.generated.h"

class UAbilityTask_ApplyRootMotionJumpForce;
class UCurveFloat;
class UCurveVector;

/**
 * PoE-style leap slam ability with arc movement and AoE landing damage.
 * Uses RootMotionJumpForce for parabolic traversal with AttackSpeed-scaled travel velocity.
 * Montage flow: Wind-up -> [JumpTriggerEvent] -> Air section -> Landing section -> End.
 */
UCLASS()
class PROJECTASCEND_API UAscendLeapSlamAbility : public UAscendAnimationAbility
{
	GENERATED_BODY()

public:
	UAscendLeapSlamAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

protected:
	virtual bool ResolveAnimationPlaybackData_Implementation(FAscendAbilityAnimationPlaybackData& OutPlaybackData) const override;
	virtual FGameplayTagContainer GetAnimationEventTags_Implementation() const override;
	virtual bool ShouldEndAbilityOnCompleted_Implementation() const override;
	virtual bool ShouldEndAbilityOnBlendOut_Implementation() const override;
	virtual bool ShouldEndAbilityOnInterrupted_Implementation() const override;
	virtual void OnAnimationEventReceivedNative(FGameplayTag EventTag, const FGameplayEventData& EventData) override;
	virtual void OnAnimationCancelledNative(FGameplayTag EventTag, const FGameplayEventData& EventData) override;
	virtual void OnAnimationInterruptedNative(FGameplayTag EventTag, const FGameplayEventData& EventData) override;

	UFUNCTION()
	void HandleJumpFinished();

	UFUNCTION()
	void HandleJumpLanded();

	/**
	 * Determines the leap destination from cursor/mouse position.
	 * @param OutTargetLocation Receives the clamped target location.
	 * @return True if a valid target location was resolved.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Leap Slam")
	bool ResolveLeapTargetLocation(FVector& OutTargetLocation) const;

	/** Returns the AttackSpeed attribute value as a scalar multiplier for travel speed. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Leap Slam")
	float GetLeapAttackSpeedScalar() const;

	/** Applies the landing effect container and gameplay cue at the avatar's current location. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Ability|Leap Slam")
	void ApplyLandingEffects();

private:
	bool StartLeapMovement();
	void CleanupJumpTask();
	void BuildLandingTargetData(const FVector& LandingLocation, FGameplayAbilityTargetDataHandle& OutTargetData) const;
	float CalculateLandingDamageMultiplier() const;

private:
	/** Montage event tag that triggers the leap arc (e.g. at the peak of the wind-up swing). */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Event")
	FGameplayTag JumpTriggerEventTag;

	/** Montage section to jump to when the arc begins. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Montage")
	FName StartMontageSection = NAME_None;
	
	/** Montage section to jump to when the arc begins. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Montage")
	FName AirMontageSection = NAME_None;

	/** Montage section to jump to upon landing. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Montage")
	FName LandingMontageSection = NAME_None;

	/** Tag used to look up the effect container applied on landing. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Event")
	FGameplayTag LandingEffectContainerTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Gameplay Cue", meta=(Categories="GameplayCue"))
	FGameplayTag LeapStartGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Gameplay Cue", meta=(Categories="GameplayCue"))
	FGameplayTag LeapLandingGameplayCueTag;

	/** When true, BuildLandingTargetData performs a sphere overlap instead of relying on TargetType. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	bool bBuildLandingTargetDataDirectly = false;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float MaxLeapDistance = 900.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float MinLeapDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float BaseJumpHeight = 275.0f;

	/** Base horizontal travel speed in UE units/sec, scaled by AttackSpeed at runtime. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float BaseTravelSpeed = 900.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float MinTravelDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float MaxTravelDuration = 1.25f;

	/** Minimum time before the "landed" callback can fire (prevents instant triggers on short hops). */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float MinimumLandedTriggerTime = 0.15f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float LandingDamageRadius = 250.0f;

	/** Minimum outgoing damage multiplier applied by the landing container. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Damage")
	float MinDistanceDamageMultiplier = 1.0f;

	/** Maximum outgoing damage multiplier reached at MaxLeapDistance. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Damage")
	float MaxDistanceDamageMultiplier = 1.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	TEnumAsByte<ECollisionChannel> LandingQueryChannel = ECC_Pawn;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	TObjectPtr<UCurveVector> JumpPathOffsetCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	TObjectPtr<UCurveFloat> JumpTimeMappingCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	ERootMotionFinishVelocityMode FinishVelocityMode = ERootMotionFinishVelocityMode::SetVelocity;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	FVector FinishSetVelocity = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Ability|Leap Slam|Config")
	float FinishClampVelocity = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_ApplyRootMotionJumpForce> ActiveJumpTask = nullptr;

	UPROPERTY(Transient)
	FVector CachedTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	float CachedTravelDistance = 0.0f;

	UPROPERTY(Transient)
	float CachedTravelDuration = 0.0f;

	UPROPERTY(Transient)
	FVector CachedActivationTargetLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bLeapStarted = false;

	UPROPERTY(Transient)
	bool bLandingEffectsApplied = false;

	UPROPERTY(Transient)
	bool bLandingSectionTriggered = false;

	UPROPERTY(Transient)
	bool bHasCachedActivationTarget = false;
};
