#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "AscendAbilitySlotComponent.generated.h"

class UAscendAbilitySystemComponent;

/**
 * Player-facing slot configuration and facade for runtime ability slot operations.
 * The ASC remains the runtime source of truth, while this component owns which slot tags
 * a local player actually exposes in its UI and input layer.
 */
UCLASS(ClassGroup = (Ascend), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class PROJECTASCEND_API UAscendAbilitySlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAscendAbilitySlotComponent();

	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	void GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const;

	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	bool IsSupportedSlotTag(const FGameplayTag& SlotTag) const;

	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	bool AssignAbilityToSlot(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& SlotTag, bool bReplaceExisting = true);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	bool ClearAbilitySlot(const FGameplayTag& SlotTag);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	bool MoveAbilityToSlot(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& SlotTag, bool bSwapIfOccupied = true);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities", meta=(Categories="InputTag"))
	bool SwapAbilitySlots(const FGameplayTag& FirstSlotTag, const FGameplayTag& SecondSlotTag);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Abilities")
	int32 AutoAssignGrantedAbilities(bool bReplaceExisting = false);

protected:
	UAscendAbilitySystemComponent* GetAscendAbilitySystem() const;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|Input|Slot", meta = (Categories = "InputTag"))
	FGameplayTagContainer SupportedSlotInputTags;
};
