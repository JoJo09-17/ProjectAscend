#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "AscendInputComponent.generated.h"

/**
 * Enhanced Input component that provides tag-based ability binding.
 * Maps InputAction assets to GameplayTags and routes press/release events
 * through the GAS ability activation pipeline.
 */
UCLASS(Config = Input)
class PROJECTASCEND_API UAscendInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UAscendInputComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Binds a map of GameplayTag→InputAction pairs to ability press/release delegates.
	 * Pressed fires on ETriggerEvent::Started; Released fires on Completed and Canceled
	 * so that ability input is properly released even if the action was interrupted.
	 *
	 * @param InputConfigs   Tag-to-action map (typically from an input config asset).
	 * @param Object         Object to bind the delegates to.
	 * @param PressedFunc    Callback invoked with the associated GameplayTag on press.
	 * @param ReleasedFunc   Callback invoked with the associated GameplayTag on release.
	 * @param BindHandles    Output array of binding handles for later removal.
	 */
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(
		const TMap<FGameplayTag, UInputAction*>& InputConfigs,
		UserClass* Object,
		PressedFuncType PressedFunc,
		ReleasedFuncType ReleasedFunc,
		TArray<uint32>& BindHandles);

	/**
	 * Binds a single native (non-ability) input action to a delegate.
	 *
	 * @param InputAction    The InputAction asset to bind.
	 * @param TriggerEvent   When the delegate should fire.
	 * @param Object         Object to bind the delegate to.
	 * @param Func           Callback function.
	 * @param BindHandles    Output array of binding handles for later removal.
	 */
	template<class UserClass, typename FuncType>
	void BindNativeAction(
		const UInputAction* InputAction,
		ETriggerEvent TriggerEvent,
		UserClass* Object,
		FuncType Func,
		TArray<uint32>& BindHandles);
};

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UAscendInputComponent::BindAbilityActions(
	const TMap<FGameplayTag, UInputAction*>& InputConfigs,
	UserClass* Object,
	PressedFuncType PressedFunc,
	ReleasedFuncType ReleasedFunc,
	TArray<uint32>& BindHandles)
{
	for (const TPair<FGameplayTag, UInputAction*>& InputConfig : InputConfigs)
	{
		if (!InputConfig.Key.IsValid() || !InputConfig.Value)
		{
			continue;
		}

		if (PressedFunc)
		{
			BindHandles.Add(
				BindAction(InputConfig.Value, ETriggerEvent::Started, Object, PressedFunc, InputConfig.Key).GetHandle()
			);
		}

		if (ReleasedFunc)
		{
			BindHandles.Add(
				BindAction(InputConfig.Value, ETriggerEvent::Completed, Object, ReleasedFunc, InputConfig.Key).GetHandle()
			);

			BindHandles.Add(
				BindAction(InputConfig.Value, ETriggerEvent::Canceled, Object, ReleasedFunc, InputConfig.Key).GetHandle()
			);
		}
	}
}

template<class UserClass, typename FuncType>
void UAscendInputComponent::BindNativeAction(
	const UInputAction* InputAction,
	ETriggerEvent TriggerEvent,
	UserClass* Object,
	FuncType Func,
	TArray<uint32>& BindHandles)
{
	if (!InputAction)
	{
		return;
	}

	BindHandles.Add(BindAction(InputAction, TriggerEvent, Object, Func).GetHandle());
}
