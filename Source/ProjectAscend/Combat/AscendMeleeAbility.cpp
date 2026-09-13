#include "Combat/AscendMeleeAbility.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Character/Base/AscendCharacterBase.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UAscendMeleeAbility::UAscendMeleeAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	ActivationGroup = EAscendAbilityActivationGroup::ExclusiveBlocking;
}

bool UAscendMeleeAbility::CanActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* RelevantTags) const
{
	const AAscendCharacterBase* Avatar = Info ? Cast<AAscendCharacterBase>(Info->AvatarActor.Get()) : nullptr;
	return Avatar && !Avatar->IsDead() && Avatar->GetCharacterMovement()->IsMovingOnGround()
		&& Super::CanActivateAbility(Handle, Info, SourceTags, TargetTags, RelevantTags);
}

void UAscendMeleeAbility::ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* Event)
{
	Combat = Info->AvatarActor->FindComponentByClass<UAscendMeleeCombatComponent>();
	if (!Combat.IsValid() || !Combat->StartAttack(this, Attack))
	{
		EndAbility(Handle, Info, ActivationInfo, true, true);
		return;
	}
	Super::ActivateAbility(Handle, Info, ActivationInfo, Event);
}

bool UAscendMeleeAbility::ResolveAnimationPlaybackData_Implementation(FAscendAbilityAnimationPlaybackData& OutData) const
{
	OutData.Montage = Attack.Montage;
	const AAscendCharacterBase* Avatar = Cast<AAscendCharacterBase>(GetAvatarActorFromActorInfo());
	OutData.PlayRate = Attack.PlayRate * (Avatar ? Avatar->GetCombatAttackSpeed() : 1.f);
	return OutData.IsValid();
}

void UAscendMeleeAbility::EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
	FGameplayAbilityActivationInfo ActivationInfo, bool bReplicate, bool bCancelled)
{
	if (Combat.IsValid()) { Combat->FinishAttack(this, bCancelled); }
	Super::EndAbility(Handle, Info, ActivationInfo, bReplicate, bCancelled);
}
