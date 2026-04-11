#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment.h"
#include "AscendAbilityFragment_InputBinding.generated.h"

/** Determines when an ability activates relative to its input event. */
UENUM(BlueprintType)
enum class EAscendAbilityActivationPolicy : uint8
{
	/** Activate once on the frame the input is pressed. */
	OnInputTriggered,
	/** Activate every frame while the input is held. */
	WhileInputActive,
	/** Activate immediately when granted (no input required). */
	OnSpawn
};

/**
 * Configures how an ability binds to player input: which input tags trigger it,
 * whether it supports runtime slot assignment, and how it activates.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Ability Fragment - Input Binding"))
struct PROJECTASCEND_API FAscendAbilityFragment_InputBinding : public FAscendAbilityFragment
{
	GENERATED_BODY()

public:
	FAscendAbilityFragment_InputBinding()
	{
		FragmentTag = AscendGameplayTags::Fragment_InputBinding;
	}

	/** Static input tags that always trigger this ability, independent of the runtime slot system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input", meta = (Categories = "Input", DisplayName = "Static Input Tags"))
	FGameplayTagContainer AbilityInputTags;

	/** Fixed input tag used when runtime slot binding is disabled. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input", meta = (Categories = "Input", EditCondition = "!bSupportsSlotBinding", EditConditionHides, DisplayName = "Fixed Input Tag"))
	FGameplayTag DefaultSlotInputTag;

	/** If true, the ability bar owns runtime routing and this ability must be assigned to a slot before it can be triggered. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input")
	bool bSupportsSlotBinding = true;

	/** Controls when the ability activates in response to input. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input")
	EAscendAbilityActivationPolicy ActivationPolicy = EAscendAbilityActivationPolicy::OnInputTriggered;

	/** If true, pressing this ability's input acts as a confirm for pending target data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input")
	bool bActAsConfirm = false;

	/** If true, pressing this ability's input acts as a cancel for pending target data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Fragment|Input")
	bool bActAsCancel = false;

	/** @return True if this fragment uses a fixed (non-runtime) slot binding. */
	bool HasFixedInputBinding() const
	{
		return !bSupportsSlotBinding && DefaultSlotInputTag.IsValid();
	}

	/** @return True if the ability bar can reassign this ability at runtime. */
	bool AllowsRuntimeSlotBinding() const
	{
		return bSupportsSlotBinding;
	}

	/** Clears DefaultSlotInputTag when slot binding is enabled (keeps data consistent). */
	void Normalize()
	{
		if (bSupportsSlotBinding)
		{
			DefaultSlotInputTag = FGameplayTag();
		}
	}
};
