#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AscendMeleeProfile.generated.h"

class UAnimMontage;
class UAscendImpactFXProfile;
class UAscendProjectileProfile;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EAscendCombatProfileStyle : uint8
{
	Saber,
	Archer
};

USTRUCT(BlueprintType)
struct FAscendRangedAttack
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UAscendProjectileProfile> ProjectileProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float Damage = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1")) float ProjectileSpeed = 2600.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1")) float ProjectileRadius = 12.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bPierceEnemies = false;
};

USTRUCT(BlueprintType)
struct FAscendMeleeAttack
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UAnimMontage> Montage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float Damage = 18.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) float DamageMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.1")) float PlayRate = 1.f;
};

/** Authored per skeleton. Hit timing lives on montage notify states. */
UCLASS(BlueprintType)
class PROJECTASCEND_API UAscendMeleeProfile : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="HitReaction") TObjectPtr<UAscendImpactFXProfile> HitFXProfile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction") TObjectPtr<UAnimMontage> HitFrontMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction") TObjectPtr<UAnimMontage> HitBackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction") TObjectPtr<UAnimMontage> HitLeftMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction") TObjectPtr<UAnimMontage> HitRightMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction", meta=(ClampMin="0")) float HitReactionMinInterval = 0.15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction", meta=(ClampMin="0.1")) float HitReactionPlayRate = 1.f;
	/** Multiplies the character's GAS AttackSpeed. 1 = original speed, 2 = twice as fast. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.1", UIMin="0.1", UIMax="5.0")) float AttackSpeedMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer") TArray<TObjectPtr<UAnimMontage>> RangedLightMontages;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer") TObjectPtr<UAnimMontage> RangedHeavyMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer", meta=(ClampMin="0.05")) float RangedInputBufferDuration = 0.45f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer", meta=(ClampMin="0")) float RangedComboResetDelay = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat") EAscendCombatProfileStyle CombatStyle = EAscendCombatProfileStyle::Saber;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer", meta=(EditCondition="CombatStyle == EAscendCombatProfileStyle::Archer", EditConditionHides)) FAscendRangedAttack RangedLightAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer", meta=(EditCondition="CombatStyle == EAscendCombatProfileStyle::Archer", EditConditionHides)) FAscendRangedAttack RangedHeavyAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Archer", meta=(EditCondition="CombatStyle == EAscendCombatProfileStyle::Archer", EditConditionHides)) FAscendRangedAttack EnemyRangedAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FAscendMeleeAttack> LightAttacks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FAscendMeleeAttack HeavyAttack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName WeaponBone = TEXT("hand_r");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FTransform WeaponOffset = FTransform(FRotator(0, 0, 90), FVector(0, 0, 0));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="10")) float BladeLength = 95.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1")) float HitHalfWidth = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UMaterialInterface> SwordMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UMaterialInterface> TrailMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor TrailColor = FLinearColor(0.15f, 0.65f, 1.f, 1.f);
};
