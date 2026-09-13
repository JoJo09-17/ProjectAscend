#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AscendPlayerController.generated.h"

class UInputAction;
class UAscendAbilitySlotComponent;
class UAscendAbilitySystemComponent;
class AAscendRangedProjectile;
class AAscendEnemyCharacter;
struct FInputActionValue;

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
	
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
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

	/** Fires the quick, single-shot ranged attack bound to the left mouse button. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Combat")
	void FireLightAttack();

	/**
	 * Fires the right-click heavy attack. ChargeAlpha is intentionally exposed for
	 * a future press-and-release charge flow; the current input uses 0.0 (uncharged).
	 */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	void FireHeavyAttack(float ChargeAlpha = 0.0f);

private:
	friend class FAscendMeleeSweepTest;
	void PrepareAttackFacing(bool bContinueCombo = false);
	void HandleRangedAttackFinished(bool bInterrupted);
	void ExecuteQueuedRangedAttack();
	bool bQueuedRangedLight = false;
	double QueuedRangedInputTime = 0.0;
	FTimerHandle QueuedRangedAttackTimer;
	FVector ResolveDodgeDirection() const;
	FVector LastMoveDirection = FVector::ZeroVector;
	void BindCombatActions();
	void HandleMappedButton(const FInputActionValue& Value, int32 Command, bool bGamepad, bool bPressed);
	void HandleGamepadStick(const FInputActionValue& Value, bool bMove);
	void HandleKeyboardActivity(const FInputActionValue& Value);
	FVector2D GamepadMoveValue = FVector2D::ZeroVector;
	FVector2D GamepadSwitchValue = FVector2D::ZeroVector;
	void ProcessGamepadInput(float DeltaTime);
	void RefreshLockTarget(const FVector2D& SwitchDirection = FVector2D::ZeroVector);
	void PerformGamepadDodge();
	FVector GamepadDirectionToWorld(const FVector2D& Stick) const;
	bool bUsingGamepad = false;
	bool bTargetSwitchLatched = false;
	double NextDodgeTime = 0.0;
	TWeakObjectPtr<AAscendEnemyCharacter> LockedTarget;
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Gamepad", meta = (ClampMin = "100.0"))
	float LockRange = 2000.0f;
	/** A nearer target must beat the current one by this many units to avoid target flicker. */
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Gamepad", meta = (ClampMin = "0.0"))
	float AutoSwitchTargetHysteresis = 75.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Gamepad")
	float DodgeSpeed = 1600.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Gamepad")
	float DodgeDuration = 0.18f;
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Gamepad")
	float DodgeCooldown = 0.65f;
	void UpdateMouseFacing(bool bContinueCombo = false);
	void FireRangedProjectile(float InDamage, float InSpeed, float InRadius);
	UAscendAbilitySystemComponent* GetASC() const;

	UPROPERTY(VisibleAnywhere, Category = "Ascend|Abilities")
	TObjectPtr<UAscendAbilitySlotComponent> AbilitySlotComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "0.0"))
	float LightAttackDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "0.0"))
	float HeavyAttackDamage = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "0.0"))
	float LightProjectileSpeed = 2600.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "0.0"))
	float HeavyProjectileSpeed = 1700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "1.0"))
	float LightProjectileRadius = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Combat", meta = (ClampMin = "1.0"))
	float HeavyProjectileRadius = 28.0f;
};
