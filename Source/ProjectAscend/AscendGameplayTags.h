#pragma once

#include "NativeGameplayTags.h"

/**
 * Central namespace for all native GameplayTag declarations used by Project Ascend.
 * Tags are defined via UE_DECLARE_GAMEPLAY_TAG_EXTERN here and instantiated in the .cpp.
 *
 * Grouping:
 *   - Ability activation failure tags
 *   - Input tags (movement, look, ability slots)
 *   - SetByCaller tags (damage, healing, runtime scalars)
 *   - Input system fragments/states
 *   - Movement mode tags and lookup maps
 */
namespace AscendGameplayTags
{
	PROJECTASCEND_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// Ability activation failure feedback
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_IsDead);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cooldown);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Cost);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsBlocked);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_TagsMissing);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_Networking);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ActivateFail_ActivationGroup);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Type_Death);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);

	// Enhanced Input tags
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Slot1);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Slot2);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Slot3);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Ability_Jump);

	// Attribute tags used by AbilitySet-driven attribute initialization.
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Health);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MaxHealth);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Mana);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MaxMana);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_AttackSpeed);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Damage);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Healing);

	// SetByCaller tags
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Heal);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_DamageMultiplier);

	// Input system fragments and states
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fragment_InputBinding);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fragment_EffectContainer);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fragment_AnimationProvider);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Fragment_UIData);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_InputBlocked_Ability);

	// Movement mode lookup maps
	PROJECTASCEND_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	PROJECTASCEND_API extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	// Movement mode tags
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);
	PROJECTASCEND_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};
