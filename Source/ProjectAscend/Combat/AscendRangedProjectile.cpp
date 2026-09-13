#include "Combat/AscendRangedProjectile.h"
#include "Combat/AscendProjectileProfile.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AscendGameplayTags.h"
#include "Combat/AscendRangedDamageEffect.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
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
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileOverlap);
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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ArrowAsset(TEXT("/Game/ArtAsset/Animations/Archer/Demo/Characters/Mannequins/Meshes/Arrow.Arrow"));
	ArrowMesh = ArrowAsset.Object;
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2600.0f;
	ProjectileMovement->MaxSpeed = 2600.0f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	// InitializeProjectile supplies a world vector before deferred spawning finishes.
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
}

void AAscendRangedProjectile::InitializeProjectile(const FVector& Direction, float InDamage, float InSpeed, float InRadius, bool bInPierceEnemies)
{
	bPierceEnemies = bInPierceEnemies;
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
	OnRepPierceEnemies();
	if (ProjectileProfile) { OnRepProjectileProfile(); }
	ProjectileMovement->InitialSpeed = InSpeed;
	ProjectileMovement->MaxSpeed = InSpeed;
	ProjectileMovement->Velocity = Direction.GetSafeNormal() * InSpeed;
}

void AAscendRangedProjectile::SetProjectileProfile(UAscendProjectileProfile* InProfile)
{
 ProjectileProfile = InProfile;
 OnRepProjectileProfile();
}

void AAscendRangedProjectile::BeginPlay()
{
 Super::BeginPlay();
 OnRepProjectileProfile();
}

void AAscendRangedProjectile::OnRepProjectileProfile()
{
 if (!ProjectileProfile) { return; }
 VisualMesh->SetStaticMesh(ProjectileProfile->Mesh);
 VisualMesh->SetRelativeTransform(ProjectileProfile->MeshTransform);
 VisualMesh->SetMaterial(0,ProjectileProfile->MaterialOverride);
 ProjectileMovement->ProjectileGravityScale = ProjectileProfile->GravityScale;
 if (HasAuthority()) { SetLifeSpan(FMath::Max(.1f,ProjectileProfile->LifeSpan)); }
 // Build cosmetics only after BeginPlay, so deferred spawning has established the flight rotation.
 if (!HasActorBegunPlay() || GetNetMode()==NM_DedicatedServer) { return; }
 if (FlightFX) { FlightFX->DestroyComponent(); FlightFX=nullptr; }
 if (ProjectileProfile->FlightNiagara)
 {
  FlightFX = UNiagaraFunctionLibrary::SpawnSystemAttached(ProjectileProfile->FlightNiagara,RootComponent,NAME_None,FVector::ZeroVector,FRotator::ZeroRotator,EAttachLocation::KeepRelativeOffset,true,false);
  if (FlightFX)
  {
   FlightFX->SetRelativeTransform(ProjectileProfile->FlightFXTransform);
   FVector Direction = GetActorForwardVector();
   if (ProjectileProfile->bDirectionInLocalSpace) { Direction = FlightFX->GetComponentTransform().InverseTransformVectorNoScale(Direction).GetSafeNormal(); }
   if (!ProjectileProfile->DirectionParameter.IsNone()) { FlightFX->SetVariableVec3(ProjectileProfile->DirectionParameter,Direction); }
   FlightFX->Activate(true);
  }
 }
}

void AAscendRangedProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (FlightFX) { FlightFX->DestroyComponent(); FlightFX=nullptr; }
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
	if (!HasAuthority() || bHasImpacted || !OtherActor || OtherActor == GetOwner())
	{
		return;
	}

	bHasImpacted = true;
	UE_LOG(LogTemp, Display, TEXT("[AscendCombat] Projectile hit %s."), *OtherActor->GetName());
	ApplyDamageTo(OtherActor,&Hit);
	Destroy();
}

void AAscendRangedProjectile::ApplyDamageTo(AActor* Target,const FHitResult* Impact)
{
	// Ranged enemies share a faction for now. Keeping this check in the shared
	// projectile makes later melee/ranged archetypes safe without duplicating it.
	if (Cast<AAscendEnemyCharacter>(GetOwner()) && Cast<AAscendEnemyCharacter>(Target))
	{
		return;
	}

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
			if (Impact) { Context.AddHitResult(*Impact,true); }

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

void AAscendRangedProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
 Super::GetLifetimeReplicatedProps(OutLifetimeProps);
 DOREPLIFETIME(AAscendRangedProjectile, bPierceEnemies);
 DOREPLIFETIME(AAscendRangedProjectile, ProjectileProfile);
}

void AAscendRangedProjectile::OnRepPierceEnemies()
{
 CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, bPierceEnemies ? ECR_Overlap : ECR_Block);
 CollisionComponent->SetGenerateOverlapEvents(bPierceEnemies);
 if (!ProjectileProfile && bPierceEnemies && ArrowMesh)
 {
  VisualMesh->SetStaticMesh(ArrowMesh);
  VisualMesh->SetRelativeRotation(FRotator(0,-90,0));
  VisualMesh->SetRelativeScale3D(FVector(0.9f));
  VisualMesh->SetRelativeLocation(FVector(-40.f,0,0));
 }
}

void AAscendRangedProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 if (!HasAuthority() || !bPierceEnemies || bHasImpacted || !OtherActor || OtherActor == GetOwner() || PiercedActors.Contains(OtherActor)) { return; }
 const auto* Character = Cast<AAscendCharacterBase>(OtherActor);
 if (!Character || Character->IsDead() || !UAscendMeleeCombatComponent::AreHostile(GetOwner(), OtherActor)) { return; }
 PiercedActors.Add(OtherActor);
 ApplyDamageTo(OtherActor,&SweepResult);
 UE_LOG(LogTemp, Display, TEXT("[AscendCombat] Arrow pierced %s."), *OtherActor->GetName());
}
