#include "AscendGameplayTags.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "AscendLogChannels.h"

namespace AscendGameplayTags
{
	// Ability activation failure feedback
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Type_Death, "Ability.Type.Death", "Semantic tag used to identify death abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Death, "Event.Death", "Gameplay event sent when an actor runs out of health.");

	// Enhanced Input tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_Slot1, "InputTag.Ability.Slot1", "1. Slot Ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_Slot2, "InputTag.Ability.Slot2", "2. Slot Ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_Slot3, "InputTag.Ability.Slot3", "3. Slot Ability");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Ability_Jump, "InputTag.Ability.Jump", "Jump Ability");

	// Attribute initialization tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Health, "Attribute.Health", "Current health attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MaxHealth, "Attribute.MaxHealth", "Maximum health attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Mana, "Attribute.Mana", "Current mana attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MaxMana, "Attribute.MaxMana", "Maximum mana attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_AttackSpeed, "Attribute.AttackSpeed", "Attack speed attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Damage, "Attribute.Damage", "Outgoing damage attribute.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Healing, "Attribute.Healing", "Outgoing healing attribute.");

	// Input system fragments and states
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_InputBinding, "Fragment.InputBinding", "Input Binding Fragment Identify");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_EffectContainer, "Fragment.EffectContainer", "EffectContainer Fragment Identify");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_AnimationProvider, "Fragment.AnimationProvider", "AnimationProvider Fragment Identify");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Fragment_UIData, "Fragment.UIData", "UIData Fragment Identify");
	
	
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_InputBlocked_Ability, "State.InputBlocked.Ability", "Blocks ability input processing while present on the owner.");

	// SetByCaller tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_DamageMultiplier, "SetByCaller.DamageMultiplier", "SetByCaller tag used by damage gameplay effects to scale source damage.");

	// Movement mode tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "Custom movement mode tag.");

	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogAscend, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}
