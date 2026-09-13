#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AscendRangedProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class UAscendProjectileProfile;
class UNiagaraComponent;

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
	void InitializeProjectile(const FVector& Direction, float InDamage, float InSpeed, float InRadius, bool bInPierceEnemies = false);
	void SetProjectileProfile(UAscendProjectileProfile* InProfile);

virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

private:
	friend class FAscendMeleeSweepTest;
	UFUNCTION() void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnRepPierceEnemies();
	UFUNCTION() void OnRepProjectileProfile();
	UPROPERTY(ReplicatedUsing=OnRepProjectileProfile) TObjectPtr<UAscendProjectileProfile> ProjectileProfile;
	UPROPERTY() TObjectPtr<UNiagaraComponent> FlightFX;
	UPROPERTY(ReplicatedUsing=OnRepPierceEnemies) bool bPierceEnemies = false;
	TSet<TWeakObjectPtr<AActor>> PiercedActors;
	UPROPERTY() TObjectPtr<class UStaticMesh> ArrowMesh;

	void ApplyDamageTo(AActor* Target,const FHitResult* Impact = nullptr);

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
