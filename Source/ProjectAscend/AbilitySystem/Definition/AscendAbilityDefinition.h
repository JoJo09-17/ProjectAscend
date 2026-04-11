#pragma once

#include "CoreMinimal.h"
#include "AscendDefinition.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_InputBinding.h"
#include "AscendAbilityDefinition.generated.h"

class UAscendGameplayAbility;
struct FAscendAbilityFragment_InputBinding;
struct FAscendAbilityFragment_EffectContainers;
struct FAscendAbilityFragment_AnimationProvider;

/**
 * Data asset defining a single ability: its class, level, tags, cooldown, and fragment-based
 * configuration (input binding, effects, animation).
 */
UCLASS()
class PROJECTASCEND_API UAscendAbilityDefinition : public UAscendDefinition
{
	GENERATED_BODY()

public:
	/** Gameplay ability class to grant when this definition is applied. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Definition")
	TSubclassOf<UAscendGameplayAbility> Ability = nullptr;

	/** Starting level for the granted ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Definition")
	int32 DefaultLevel = 1;

	/** Semantic tags describing this ability (used for queries, cancellation, UI filtering). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Definition", Meta = (Categories = "Ability"))
	FGameplayTagContainer AbilityTags;

	/** Tags granted to the owner while this ability is active. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Definition")
	FGameplayTagContainer OwnedTags;

	/** Base cooldown duration (can scale by level via curve table). */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ascend|Definition")
	FScalableFloat CooldownDuration;

	/** Tags identifying the cooldown slot. Multiple tags allow independent cooldowns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Definition", Meta = (Categories = "Gameplay.Cooldown"))
	FGameplayTagContainer CooldownTags;

	const FAscendAbilityFragment_InputBinding* GetInputBindingFragment() const;
	FAscendAbilityFragment_InputBinding* GetInputBindingFragment();

	const FAscendAbilityFragment_EffectContainers* GetEffectContainersFragment() const;
	FAscendAbilityFragment_EffectContainers* GetEffectContainersFragment();

	const FAscendAbilityFragment_AnimationProvider* GetAnimationProviderFragment() const;
	FAscendAbilityFragment_AnimationProvider* GetAnimationProviderFragment();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
};
