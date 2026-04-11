#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AscendPlayerController.generated.h"

class UInputAction;
class UAscendAbilitySlotComponent;
class UAscendAbilitySystemComponent;

/**
 * Player controller for the ARPG top-down template.
 * Routes Enhanced Input through a tag-based ability activation pipeline
 * by forwarding press/release events to the ASC and calling
 * ProcessAbilityInput during PostProcessInput.
 */
UCLASS()
class PROJECTASCEND_API AAscendPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAscendPlayerController();
	
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;
	
	/** Returns the world-space location under the mouse cursor via visibility trace. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Input")
	FVector GetMouseHitLocation();

	/**
	 * Binds ability input actions to press/release delegates on the possessed pawn's
	 * AscendInputComponent. Call after possessing a pawn with a valid ASC.
	 *
	 * @param InputConfig   Map of GameplayTag→InputAction pairs from the input config asset.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Input")
	void BindingInput(const TMap<FGameplayTag, UInputAction*> InInputConfig);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ascend|Input", meta=(Categories="InputTag"))
	TMap<FGameplayTag, UInputAction*> InputConfig;
	
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);

	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	UAscendAbilitySlotComponent* GetAbilitySlotComponent() const { return AbilitySlotComponent; }

private:
	UAscendAbilitySystemComponent* GetASC() const;

	UPROPERTY(VisibleAnywhere, Category = "Ascend|Abilities")
	TObjectPtr<UAscendAbilitySlotComponent> AbilitySlotComponent;
};
