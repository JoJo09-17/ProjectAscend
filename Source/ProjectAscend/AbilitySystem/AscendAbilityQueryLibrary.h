#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystem/Data/AscendAbilitySet.h"
#include "AscendAbilityQueryLibrary.generated.h"

class UAscendAbilityDefinition;
class UAscendAbilitySystemComponent;

/**
 * Blueprint-accessible query helpers for ability handles, definitions, and slot state.
 */
UCLASS()
class PROJECTASCEND_API UAscendAbilityQueryLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** @return True if the handle is non-invalid. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool IsAbilityHandleValid(FGameplayAbilitySpecHandle AbilityHandle);

	/** @return True if the granted handles set contains at least one ability. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GrantedHandlesHasAny(const FAscendAbilitySet_GrantedHandles& GrantedHandles);

	/** @return Number of ability handles in the granted handles set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static int32 GetGrantedAbilityHandleCount(const FAscendAbilitySet_GrantedHandles& GrantedHandles);

	/** Checks whether a specific ability handle exists in the granted handles set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GrantedHandlesContainsAbilityHandle(
		const FAscendAbilitySet_GrantedHandles& GrantedHandles,
		FGameplayAbilitySpecHandle AbilityHandle);

	/** Retrieves the first ability handle from the granted handles set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GetFirstGrantedAbilityHandle(
		const FAscendAbilitySet_GrantedHandles& GrantedHandles,
		FGameplayAbilitySpecHandle& OutHandle);

	/** Retrieves the last ability handle from the granted handles set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GetLastGrantedAbilityHandle(
		const FAscendAbilitySet_GrantedHandles& GrantedHandles,
		FGameplayAbilitySpecHandle& OutHandle);

	/** Retrieves the ability definition for a given handle. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static UAscendAbilityDefinition* GetAbilityDefinition(
		const UAscendAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle AbilityHandle);

	/** Retrieves the source object associated with a granted ability. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static UObject* GetAbilitySourceObject(
		const UAscendAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle AbilityHandle);

	/** Collects all dynamic input tags for an ability (including slot tag). */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GetAbilityInputTags(
		const UAscendAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle AbilityHandle,
		FGameplayTagContainer& OutInputTags);

	/** Resolves the slot input tag an ability is currently bound to. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool GetAbilitySlot(
		const UAscendAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle AbilityHandle,
		FGameplayTag& OutSlotInputTag);

	/** Finds the first granted ability handle with the requested semantic ability tag. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool FindAbilityHandleByTag(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTag& AbilityTag,
		FGameplayAbilitySpecHandle& OutAbilityHandle);

	/** Finds the first granted ability handle matching the requested tag set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static bool FindAbilityHandleByTags(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		FGameplayAbilitySpecHandle& OutAbilityHandle);

	/** Collects all granted ability handles matching the requested tag set. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Abilities")
	static int32 FindAbilityHandlesByTags(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles);

	/** Native helper returning a copy of the first granted spec with the requested semantic tag. */
	static bool FindAbilitySpecByTag(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTag& AbilityTag,
		FGameplayAbilitySpec& OutAbilitySpec);

	/** Native helper returning a copy of the first granted spec matching the requested tag set. */
	static bool FindAbilitySpecByTags(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		FGameplayAbilitySpec& OutAbilitySpec);

	/** Native helper returning copies of all granted specs matching the requested tag set. */
	static int32 FindAbilitySpecsByTags(
		const UAscendAbilitySystemComponent* ASC,
		const FGameplayTagContainer& AbilityTags,
		bool bRequireAllTags,
		TArray<FGameplayAbilitySpec>& OutAbilitySpecs);
};
