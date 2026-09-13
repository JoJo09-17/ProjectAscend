#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Combat/AscendMeleeProfile.h"
#include "AscendCharacterBase.generated.h"

class UAscendAbilitySet;
class UAscendMeleeCombatComponent;
class UAscendMaterialControllerComponent;

/** Broadcast when a character dies. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDied, AAscendCharacterBase*, Victim);
DECLARE_MULTICAST_DELEGATE_OneParam(FAscendRangedAttackFinished, bool);
DECLARE_MULTICAST_DELEGATE(FAscendRangedComboReady);

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
	/** Plays the configured ranged attack; returns false while a previous shot is animating. */
	bool PlayRangedAttackAnimation(bool bHeavy = false, float ChargeAlpha = 0.f, bool bContinueCombo = false);
	bool CanAdvanceRangedCombo() const { return IsRangedAttacking() && !bRangedHeavyAttack && bRangedShotReleased && bRangedComboWindowOpen; }
	void ReleaseRangedShot(class UAnimSequenceBase* Animation);
	void SetRangedComboWindow(class UAnimSequenceBase* Animation, bool bOpen);
	bool IsActiveRangedMontageInstance(int32 InstanceID) const { return IsRangedAttacking() && InstanceID == ActiveRangedMontageInstanceID; }
	UFUNCTION(BlueprintPure, Category="Combat") float GetCombatAttackSpeed() const;
	/** Cancels a normal attack for dodge, but refuses to cancel heavy attacks. */
	bool TryInterruptNormalAttack();
	UFUNCTION(BlueprintPure, Category="Combat")
	bool IsRangedAttacking() const { return ActiveRangedMontage.IsValid(); }
	bool IsRangedHeavyAttacking() const { return IsRangedAttacking() && bRangedHeavyAttack; }
	FAscendRangedAttackFinished OnRangedAttackFinished;
	FAscendRangedComboReady OnRangedComboReady;
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
	TObjectPtr<UAscendMeleeCombatComponent> MeleeCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
	TObjectPtr<UAscendMaterialControllerComponent> MaterialController;

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
	friend class FAscendMeleeSweepTest;
    friend class AAscendEnemyCharacter;
	void OnRangedAttackEnded(class UAnimMontage* Montage, bool bInterrupted);
	int32 RangedComboIndex = 0;
	bool bRangedHeavyAttack = false;
	bool bRangedShotReleased = false;
	bool bRangedComboWindowOpen = false;
	FAscendRangedAttack PendingRangedAttack;
	FVector RangedShotDirection = FVector::ForwardVector;
	void SpawnRangedProjectile(const FAscendRangedAttack& Attack, const FVector& Direction);
	double LastRangedAttackTime = -100.0;
	TWeakObjectPtr<class UAnimMontage> ActiveRangedMontage;
	int32 ActiveRangedMontageInstanceID = INDEX_NONE;
	bool bStartupAbilitiesGranted = false;
	bool bDefaultAttributesApplied = false;
	bool bIsDead = false;
	bool bDeathFinished = false;
};
