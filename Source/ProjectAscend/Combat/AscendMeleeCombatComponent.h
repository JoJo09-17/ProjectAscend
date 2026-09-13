#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "AscendMeleeProfile.h"
#include "AscendMeleeCombatComponent.generated.h"

class AAscendCharacterBase;
class UAscendMeleeAbility;
class UBoxComponent;
class UProceduralMeshComponent;
class UAnimNotifyState;

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class PROJECTASCEND_API UAscendMeleeCombatComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAscendMeleeCombatComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee") TObjectPtr<UAscendMeleeProfile> Profile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee") bool bDrawHitBoxes = false;
	UFUNCTION(BlueprintCallable, Category="Melee") bool RequestAttack(bool bHeavy = false);
	UFUNCTION(BlueprintPure, Category="Melee") bool IsAttacking() const { return ActiveAbility.IsValid(); }
	bool IsAttackMovementLocked() const { return bAttackMovementLocked; }
	bool IsHeavyAttacking() const { return IsAttacking() && bHeavyRequested; }
	UFUNCTION(BlueprintPure, Category="Melee") bool IsDamageWindowOpen() const { return WindowToken.IsValid(); }
	UFUNCTION(BlueprintPure, Category="Melee") int32 GetHitCount() const { return HitActors.Num(); }
	UFUNCTION(BlueprintCallable, Category="Melee") void CancelAttack();
	bool StartAttack(UAscendMeleeAbility* Ability, FAscendMeleeAttack& OutAttack);
	void FinishAttack(UAscendMeleeAbility* Ability, bool bCancelled);
	void BeginDamageWindow(const UAnimNotifyState* Token);
	void EndDamageWindow(const UAnimNotifyState* Token);
	static bool AreHostile(const AActor* Source, const AActor* Target);
	// Same geometry routine is used by runtime and collision regression tests.
	static void SweepBlade(UWorld* World, const FTransform& From, const FTransform& To,
		const FVector& HalfExtent, AActor* Source, TArray<FHitResult>& OutHits);
private:
	friend class AAscendCharacterBase;
	friend class AAscendEnemyCharacter;
	friend class FAscendMeleeSweepTest;
	void LockAttackMovement();
	void UnlockAttackMovement();
	bool bAttackMovementLocked = false;
	uint8 MovementModeBeforeAttack = 0;
	uint8 CustomMovementModeBeforeAttack = 0;
	UFUNCTION(Server, Reliable) void ServerRequestAttack(bool bHeavy);
	void CreateWeapon();
	FTransform GetBladeTransform() const;
	void TraceBlade(const FTransform& From, const FTransform& To);
	void UpdateTrail(float DeltaTime);
	TWeakObjectPtr<AAscendCharacterBase> Character;
	TWeakObjectPtr<UAscendMeleeAbility> ActiveAbility;
	TWeakObjectPtr<const UAnimNotifyState> WindowToken;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	FGameplayAbilitySpecHandle AbilityHandle;
	FAscendMeleeAttack CurrentAttack;
	FTransform PreviousBlade;
	bool bHeavyRequested = false;
	bool bQueuedLight = false;
	int32 ComboIndex = 0;
	double LastAttackEnd = -100.0;
	FTimerHandle QueuedAttackTimer;
	UPROPERTY(Transient) TObjectPtr<UProceduralMeshComponent> Sword;
	UPROPERTY(Transient) TObjectPtr<UProceduralMeshComponent> Trail;
	UPROPERTY(Transient) TObjectPtr<UBoxComponent> HitBox;
	struct FTrailSample { FVector Base, Tip; float Age; };
	TArray<FTrailSample> TrailSamples;
};
