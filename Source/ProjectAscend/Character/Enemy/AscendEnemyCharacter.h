#pragma once

#include "CoreMinimal.h"
#include "Character/Base/AscendCharacterBase.h"
#include "AscendEnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EAscendEnemyCombatStyle : uint8
{
	Ranged,
	Melee
};

/**
 * AI-controlled enemy character with a self-contained ASC.
 */
UCLASS()
class PROJECTASCEND_API AAscendEnemyCharacter : public AAscendCharacterBase
{
	GENERATED_BODY()

public:
	AAscendEnemyCharacter();

	virtual void BeginPlay() override;
	void HandleDamageReaction(AActor* DamageSource,const FHitResult* Hit = nullptr);
	void EmitHitEffects(UAnimSequenceBase* Animation,int32 InstanceID,class UAscendImpactFXProfile* Override,bool bVFX,bool bDecal);
	UFUNCTION(BlueprintPure, Category="Combat|HitReaction") bool IsHitReacting() const { return ActiveHitMontage.IsValid(); }
	UAnimMontage* SelectHitReaction(const FVector& SourceLocation) const;
	UAnimMontage* GetActiveHitMontage() const { return ActiveHitMontage.Get(); }


	EAscendEnemyCombatStyle GetCombatStyle() const;
	float GetAwarenessRange() const { return AwarenessRange; }
	float GetRetreatDistance() const { return RetreatDistance; }
	float GetPreferredAttackDistance() const { return PreferredAttackDistance; }
	float GetAttackRange() const { return AttackRange; }
	float GetAttackCooldown() const { return AttackCooldown; }
	float GetProjectileDamage() const { return ProjectileDamage; }
	float GetProjectileSpeed() const { return ProjectileSpeed; }
	float GetProjectileRadius() const { return ProjectileRadius; }
	float GetMeleeAttackRange() const { return MeleeAttackRange; }
	float GetMeleeAttackCooldown() const { return MeleeAttackCooldown; }

private:
	UFUNCTION(NetMulticast, Reliable) void MulticastHitReaction(UAnimMontage* Montage,FVector HitPoint,FVector HitNormal,FVector ProjectileDirection);
	friend class FAscendEnemyHitReactionTest;
	FVector HitEffectPoint = FVector::ZeroVector;
	FVector HitEffectNormal = FVector::ForwardVector;
	FVector HitProjectileDirection = FVector::ForwardVector;
	bool bHitVFXEmitted = false;
	bool bHitDecalEmitted = false;
	TWeakObjectPtr<UAnimMontage> ActiveHitMontage;
	int32 HitMontageInstanceID = INDEX_NONE;
	double NextHitReactionTime = -100.0;

protected:
	virtual void InitializeGAS() override;
	virtual void HandleDeathFinished() override;
	virtual void HandleDeathStarted() override;

	/** Selects the combat policy used by the AI controller. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Combat")
	EAscendEnemyCombatStyle CombatStyle = EAscendEnemyCombatStyle::Ranged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Melee", meta=(ClampMin="50"))
	float MeleeAttackRange = 145.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Melee", meta=(ClampMin="0.1"))
	float MeleeAttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "100.0"))
	float AwarenessRange = 2600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "0.0"))
	float RetreatDistance = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "0.0"))
	float PreferredAttackDistance = 1100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "0.0"))
	float AttackRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "0.05"))
	float AttackCooldown = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "0.0"))
	float ProjectileDamage = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "1.0"))
	float ProjectileSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Ranged", meta = (ClampMin = "1.0"))
	float ProjectileRadius = 14.0f;
};
