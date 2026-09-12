#include "Combat/AscendRangedProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AscendGameplayTags.h"
#include "Combat/AscendRangedDamageEffect.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AAscendRangedProjectile::AAscendRangedProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 3.0f;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(12.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
	RootComponent = CollisionComponent;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(CollisionComponent);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCanEverAffectNavigation(false);
	VisualMesh->SetRelativeScale3D(FVector(0.15f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		VisualMesh->SetStaticMesh(SphereMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2600.0f;
	ProjectileMovement->MaxSpeed = 2600.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void AAscendRangedProjectile::InitializeProjectile(const FVector& Direction, float InDamage, float InSpeed, float InRadius)
{
	Damage = InDamage;
	// Ignoring the owner in the hit callback alone does not prevent movement from stopping.
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	if (GetOwner())
	{
		if (UPrimitiveComponent* OwnerCollision = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()))
		{
			OwnerCollision->IgnoreActorWhenMoving(this, true);
		}
	}
	CollisionComponent->SetSphereRadius(InRadius);
	VisualMesh->SetRelativeScale3D(FVector(InRadius / 80.0f));
	ProjectileMovement->InitialSpeed = InSpeed;
	ProjectileMovement->MaxSpeed = InSpeed;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * InSpeed;
}

void AAscendRangedProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner())
	{
		if (UPrimitiveComponent* OwnerCollision = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()))
		{
			OwnerCollision->IgnoreActorWhenMoving(this, false);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void AAscendRangedProjectile::OnProjectileHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bHasImpacted || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	bHasImpacted = true;
	UE_LOG(LogTemp, Display, TEXT("[AscendCombat] Projectile hit %s."), *OtherActor->GetName());
	ApplyDamageTo(OtherActor);
	Destroy();
}

void AAscendRangedProjectile::ApplyDamageTo(AActor* Target)
{
	if (IAbilitySystemInterface* TargetAbilitySystemOwner = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* TargetASC = TargetAbilitySystemOwner->GetAbilitySystemComponent())
		{
			UAbilitySystemComponent* SourceASC = nullptr;
			if (const IAbilitySystemInterface* SourceAbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner()))
			{
				SourceASC = SourceAbilitySystemOwner->GetAbilitySystemComponent();
			}
			UAbilitySystemComponent* SpecASC = SourceASC ? SourceASC : TargetASC;
			FGameplayEffectContextHandle Context = SpecASC->MakeEffectContext();
			Context.AddInstigator(GetOwner(), this);
			Context.AddSourceObject(this);

			FGameplayEffectSpecHandle DamageSpec = SpecASC->MakeOutgoingSpec(
				UAscendRangedDamageEffect::StaticClass(), 1.0f, Context);
			if (DamageSpec.IsValid())
			{
				DamageSpec.Data->SetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_Damage, Damage);
				const float HealthBefore = TargetASC->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute());
				SpecASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
				const float HealthAfter = TargetASC->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute());
				UE_LOG(LogTemp, Display, TEXT("[AscendCombat] Applied %.1f damage to %s: health %.1f -> %.1f"),
					Damage, *Target->GetName(), HealthBefore, HealthAfter);
				return;
			}

			UE_LOG(LogTemp, Warning, TEXT("[AscendCombat] Failed to create damage effect spec for %s."), *Target->GetName());
		}
	}

	// Keeps the projectile useful against non-GAS test actors as well.
	Target->TakeDamage(Damage, FDamageEvent(), GetInstigatorController(), this);
}
