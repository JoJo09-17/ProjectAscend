#include "Player/AscendAbilitySlotComponent.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "AscendGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilitySlotComponent)

UAscendAbilitySlotComponent::UAscendAbilitySlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SupportedSlotInputTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot1);
	SupportedSlotInputTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot2);
	SupportedSlotInputTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot3);
}

void UAscendAbilitySlotComponent::GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const
{
	SupportedSlotInputTags.GetGameplayTagArray(OutSlotTags);
}

bool UAscendAbilitySlotComponent::IsSupportedSlotTag(const FGameplayTag& SlotTag) const
{
	return SlotTag.IsValid() && SupportedSlotInputTags.HasTagExact(SlotTag);
}

bool UAscendAbilitySlotComponent::AssignAbilityToSlot(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& SlotTag,
	bool bReplaceExisting)
{
	if (UAscendAbilitySystemComponent* ASC = GetAscendAbilitySystem())
	{
		return ASC->AssignAbilityToSlot(AbilityHandle, SlotTag, bReplaceExisting);
	}

	return false;
}

bool UAscendAbilitySlotComponent::ClearAbilitySlot(const FGameplayTag& SlotTag)
{
	if (UAscendAbilitySystemComponent* ASC = GetAscendAbilitySystem())
	{
		return ASC->ClearAbilitySlot(SlotTag);
	}

	return false;
}

bool UAscendAbilitySlotComponent::MoveAbilityToSlot(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& SlotTag,
	bool bSwapIfOccupied)
{
	if (UAscendAbilitySystemComponent* ASC = GetAscendAbilitySystem())
	{
		return ASC->MoveAbilityToSlot(AbilityHandle, SlotTag, bSwapIfOccupied);
	}

	return false;
}

bool UAscendAbilitySlotComponent::SwapAbilitySlots(const FGameplayTag& FirstSlotTag, const FGameplayTag& SecondSlotTag)
{
	if (UAscendAbilitySystemComponent* ASC = GetAscendAbilitySystem())
	{
		return ASC->SwapAbilitySlots(FirstSlotTag, SecondSlotTag);
	}

	return false;
}

int32 UAscendAbilitySlotComponent::AutoAssignGrantedAbilities(bool bReplaceExisting)
{
	if (UAscendAbilitySystemComponent* ASC = GetAscendAbilitySystem())
	{
		return ASC->AutoAssignDefaultSlotsForGrantedAbilities(bReplaceExisting);
	}

	return 0;
}

UAscendAbilitySystemComponent* UAscendAbilitySlotComponent::GetAscendAbilitySystem() const
{
	const AActor* OwnerActor = GetOwner();
	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!AbilitySystemInterface)
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(OwnerActor))
		{
			AbilitySystemInterface = Cast<IAbilitySystemInterface>(PlayerController->GetPawn());
		}
	}

	if (!AbilitySystemInterface)
	{
		return nullptr;
	}

	return Cast<UAscendAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
}
