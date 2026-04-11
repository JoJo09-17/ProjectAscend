#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "AscendAbilitySystemWidget.generated.h"

class UAscendAbilitySystemComponent;

/**
 * User widget that listens directly to an ASC and forwards attribute or slot updates to Blueprint.
 * Use this as the runtime bridge for health/mana bars without introducing MVVM view models.
 */
UCLASS()
class PROJECTASCEND_API UAscendAbilitySystemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Ascend|UI")
	bool InitializeAbilitySystemWidget(UAbilitySystemComponent* InAbilitySystemComponent);

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	UAscendAbilitySystemComponent* GetOwnerAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	float GetCurrentHealth() const;

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	float GetCurrentMana() const;

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	float GetMaxMana() const;

	UFUNCTION(BlueprintPure, Category = "Ascend|UI")
	float GetAttackSpeed() const;

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void K2_InitializeAbilitySystemWidget(bool bBindingDone);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnHealthChanged(float NewValue, float OldValue, float NewPercentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnMaxHealthChanged(float NewValue, float OldValue, float NewPercentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnManaChanged(float NewValue, float OldValue, float NewPercentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnMaxManaChanged(float NewValue, float OldValue, float NewPercentage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnAttackSpeedChanged(float NewValue, float OldValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ascend|UI")
	void OnAbilitySlotChanged(const FGameplayTag& SlotTag, FGameplayAbilitySpecHandle AbilityHandle, bool bOccupied);

private:
	void UnbindFromCurrentASC();
	void BroadcastCurrentAttributeValues() const;
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleManaChanged(const FOnAttributeChangeData& Data);
	void HandleMaxManaChanged(const FOnAttributeChangeData& Data);
	void HandleAttackSpeedChanged(const FOnAttributeChangeData& Data);

	UFUNCTION()
	void HandleAbilitySlotChangedEvent(const FGameplayTag& SlotTag, FGameplayAbilitySpecHandle AbilityHandle, bool bOccupied);

	float GetSafePercentage(float CurrentValue, float MaxValue) const;

	TWeakObjectPtr<UAscendAbilitySystemComponent> AbilitySystemComponent;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle ManaChangedHandle;
	FDelegateHandle MaxManaChangedHandle;
	FDelegateHandle AttackSpeedChangedHandle;
};
