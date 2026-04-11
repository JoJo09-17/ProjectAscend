#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "AscendCharacterBase.generated.h"

class UAscendAbilitySet;

/** Broadcast when a character dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDied, AAscendCharacterBase*, Victim);

/**
 * Base character for all actors that use the Gameplay Ability System.
 * Owns an ASC, grants startup content from an AbilitySet, and optionally applies a legacy default-attributes effect.
 */
UCLASS()
class PROJECTASCEND_API AAscendCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAscendCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Resolves the ASC from PlayerState if available, otherwise falls back to the local component. */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	/** Broadcasts when this character dies. Use for quest systems, AI awareness, and UI updates. */
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCharacterDied OnCharacterDied;

	/** Kill this character. Triggers death handling and broadcasts OnCharacterDied. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void Die();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const { return bIsDead; }

	/** Called by the attribute set when the character's health reaches zero. */
	virtual void HandleOutOfHealth(const struct FAscendAttributeSetExecutionData& ExecutionData);

	/** Completes death when no dedicated death ability is configured or once it finishes. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void FinishDeath();

protected:
	/** Initializes the ASC actor info and grants startup content. */
	virtual void InitializeGAS();
	virtual void HandleDeathStarted();
	virtual void HandleDeathFinished();

	/** Set of abilities granted to this character on spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability System|AbilitySet")
	TObjectPtr<UAscendAbilitySet> AbilitySet;

	/** Legacy fallback Gameplay Effect applied when the ability set does not provide direct attribute initialization. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<class UAscendAbilitySystemComponent> AbilitySystemComponent;

	/** Optional semantic tag used to locate a granted death ability before falling back to FinishDeath. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FGameplayTag DeathAbilityTag;

private:
	bool bStartupAbilitiesGranted = false;
	bool bDefaultAttributesApplied = false;
	bool bIsDead = false;
	bool bDeathFinished = false;
};
