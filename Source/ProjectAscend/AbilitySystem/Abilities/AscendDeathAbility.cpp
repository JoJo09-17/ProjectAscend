#include "AbilitySystem/Abilities/AscendDeathAbility.h"

#include "Character/Base/AscendCharacterBase.h"
#include "GameplayAbilitySpec.h"
#include "AscendGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendDeathAbility)

UAscendDeathAbility::UAscendDeathAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationGroup = EAscendAbilityActivationGroup::ExclusiveBlocking;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(AscendGameplayTags::Ability_Type_Death);
	SetAssetTags(AssetTags);

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = AscendGameplayTags::Event_Death;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UAscendDeathAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (AAscendCharacterBase* Character = Cast<AAscendCharacterBase>(GetAvatarActorFromActorInfo()))
	{
		Character->FinishDeath();
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
