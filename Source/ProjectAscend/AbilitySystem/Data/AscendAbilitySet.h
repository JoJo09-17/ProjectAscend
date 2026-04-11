#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Logging/LogMacros.h"
#include "GameplayAbilitySpecHandle.h"
#include "Engine/DataAsset.h"
#include "AscendAbilitySet.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class UAscendAbilityDefinition;
class UAscendAbilitySystemComponent;

/** Describes a gameplay ability to grant from an ability set. */
USTRUCT(BlueprintType)
struct FAscendAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	int32 AbilityLevel = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

/** Describes a gameplay effect to apply from an ability set. */
USTRUCT(BlueprintType)
struct FAscendAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	float EffectLevel = 1.0f;
};

/** Describes an attribute set to add from an ability set. */
USTRUCT(BlueprintType)
struct FAscendAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	TSubclassOf<UAttributeSet> AttributeSet;
};

/** Describes an attribute value to initialize from an ability set. */
USTRUCT(BlueprintType)
struct FAscendAbilitySet_AttributeInitializer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (Categories = "Attribute"))
	FGameplayTag AttributeTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	float BaseValue = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet")
	bool bOverrideCurrentValue = false;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (EditCondition = "bOverrideCurrentValue"))
	float CurrentValue = 0.0f;
};

/**
 * Tracks handles for abilities, effects, and attributes granted by an ability set.
 * Supports revocation via TakeFromAbilitySystem().
 */
USTRUCT(BlueprintType)
struct FAscendAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	bool HasAny() const;
	bool HasAbilityHandle(const FGameplayAbilitySpecHandle& Handle) const;
	int32 GetAbilityHandleCount() const;
	bool GetFirstAbilityHandle(FGameplayAbilitySpecHandle& OutHandle) const;
	bool GetLastAbilityHandle(FGameplayAbilitySpecHandle& OutHandle) const;
	const TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() const { return AbilitySpecHandles; }

	/** Removes all granted abilities, effects, and attribute sets from the specified ASC. */
	void TakeFromAbilitySystem(UAscendAbilitySystemComponent* AscendASC);

protected:
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/**
 * Data asset that bundles abilities, effects, and attributes into a single grantable package.
 * Used to configure starting loadouts, equipment grants, and progression unlocks.
 */
UCLASS()
class PROJECTASCEND_API UAscendAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static FPrimaryAssetType AssetType;

	UAscendAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Grants all abilities, effects, and attributes to the specified ASC.
	 * @param ASC Target ability system component.
	 * @param OutGrantedHandles Receives handles for later revocation (may be null).
	 * @param SourceObject Optional source object to associate with granted specs.
	 * @param bAutoAssignRuntimeSlots If true, slot-bindable abilities are assigned to the first available supported runtime slot.
	 */
	void GiveToAbilitySystem(
		UAscendAbilitySystemComponent* ASC,
		FAscendAbilitySet_GrantedHandles* OutGrantedHandles,
		UObject* SourceObject = nullptr,
		bool bAutoAssignRuntimeSlots = false) const;

	UFUNCTION(BlueprintCallable, Category = "Ascend|AbilitySet", meta = (DisplayName = "GiveToAbilitySystem"))
	void BP_GiveToAbilitySystem(
		UAscendAbilitySystemComponent* ASC,
		struct FAscendAbilitySet_GrantedHandles& OutGrantedHandles,
		UObject* SourceObject = nullptr,
		bool bAutoAssignRuntimeSlots = false);

	UFUNCTION(BlueprintCallable, Category = "Ascend|AbilitySet", meta = (DisplayName = "TakeFromAbilitySystem"))
	static void BP_TakeFromAbilitySystem(UAscendAbilitySystemComponent* ASC, FAscendAbilitySet_GrantedHandles GrantedHandles);

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (TitleProperty = Ability))
	TArray<TObjectPtr<UAscendAbilityDefinition>> GrantedGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (TitleProperty = Ability))
	TArray<FAscendAbilitySet_GameplayAbility> GrantedDefaultGameplayAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (TitleProperty = GameplayEffect))
	TArray<FAscendAbilitySet_GameplayEffect> GrantedGameplayEffects;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (TitleProperty = AttributeSet))
	TArray<FAscendAbilitySet_AttributeSet> GrantedAttributes;

	/** Returns true when this ability set provides direct attribute initialization data. */
	bool HasAttributeInitializers() const;

	UPROPERTY(EditDefaultsOnly, Category = "Ascend|AbilitySet", meta = (TitleProperty = AttributeTag))
	TArray<FAscendAbilitySet_AttributeInitializer> GrantedAttributeInitializers;
};
