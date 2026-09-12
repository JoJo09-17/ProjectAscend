#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AscendRangedProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * Lightweight, code-only projectile used by the starter ranged attacks.
 * It deliberately has no gameplay-effect asset dependency, so combat can be
 * tested before the final GAS ability definitions and VFX are authored.
 */
UCLASS()
class PROJECTASCEND_API AAscendRangedProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAscendRangedProjectile();

	/** Sets the projectile values before its first collision. */
	void InitializeProjectile(const FVector& Direction, float InDamage, float InSpeed, float InRadius);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	void ApplyDamageTo(AActor* Target);

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleInstanceOnly, Category = "Projectile")
	float Damage = 10.0f;

	bool bHasImpacted = false;
};
