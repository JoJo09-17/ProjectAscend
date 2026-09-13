#include "Character/Enemy/AscendEnemyCharacter.h"
#include "AI/AscendEnemyAIController.h"
#include "Character/Player/AscendPlayerState.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Combat/AscendRangedProjectile.h"
#include "Combat/AscendImpactFXProfile.h"
#include "GameFramework/ProjectileMovementComponent.h"

EAscendEnemyCombatStyle AAscendEnemyCharacter::GetCombatStyle() const
{
	if (MeleeCombat && MeleeCombat->Profile)
	{
		return MeleeCombat->Profile->CombatStyle == EAscendCombatProfileStyle::Saber
			? EAscendEnemyCombatStyle::Melee : EAscendEnemyCombatStyle::Ranged;
	}
	return CombatStyle;
}

AAscendEnemyCharacter::AAscendEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAscendAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AIControllerClass = AAscendEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AAscendEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Combat AI owns facing; controller yaw must not overwrite attack direction.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Existing Blueprint children may have serialized the old empty controller
	// class. Repair that legacy default so placed demo enemies become active.
	if (HasAuthority() && !GetController())
	{
		AIControllerClass = AAscendEnemyAIController::StaticClass();
		SpawnDefaultController();
	}
}

void AAscendEnemyCharacter::HandleDeathFinished()
{
	Super::HandleDeathFinished();
	// Ragdoll the mesh and disable navigation collision
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	SetLifeSpan(3.0f);
}

void AAscendEnemyCharacter::InitializeGAS()
{
	Super::InitializeGAS();
}

UAnimMontage* AAscendEnemyCharacter::SelectHitReaction(const FVector& SourceLocation) const
{
 const auto* Profile = MeleeCombat ? MeleeCombat->Profile.Get() : nullptr;
 if (!Profile) { return nullptr; }
 const FVector Local = GetActorRotation().UnrotateVector((SourceLocation - GetActorLocation()).GetSafeNormal2D());
 UAnimMontage* Montage = FMath::Abs(Local.X) >= FMath::Abs(Local.Y)
  ? (Local.X >= 0 ? Profile->HitFrontMontage.Get() : Profile->HitBackMontage.Get())
  : (Local.Y >= 0 ? Profile->HitRightMontage.Get() : Profile->HitLeftMontage.Get());
 return Montage ? Montage : Profile->HitFrontMontage.Get();
}

void AAscendEnemyCharacter::HandleDamageReaction(AActor* DamageSource,const FHitResult* Hit)
{
 if (!HasAuthority() || IsDead() || !MeleeCombat || !MeleeCombat->Profile || !GetWorld() || GetWorld()->GetTimeSeconds() < NextHitReactionTime) { return; }
 FVector SourceLocation = DamageSource ? DamageSource->GetActorLocation() : GetActorLocation() + GetActorForwardVector();
 if (const auto* Projectile = Cast<AAscendRangedProjectile>(DamageSource))
 {
  if (const auto* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>(); Movement && !Movement->Velocity.IsNearlyZero())
  {
   // Swept overlaps may dispatch after the arrow has passed through the target.
   SourceLocation = GetActorLocation() - Movement->Velocity.GetSafeNormal2D() * 100.f;
  }
 }
 UAnimMontage* Montage = SelectHitReaction(SourceLocation);
 if (!Montage || !GetMesh()->GetAnimInstance() || !GetMesh()->GetSkeletalMeshAsset() || Montage->GetSkeleton() != GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()) { return; }
 NextHitReactionTime = GetWorld()->GetTimeSeconds() + MeleeCombat->Profile->HitReactionMinInterval;
 FVector Point = GetActorLocation()+FVector(0,0,45);
 FVector Normal = (SourceLocation - GetActorLocation()).GetSafeNormal(SMALL_NUMBER,GetActorForwardVector());
 if (Hit && Hit->Component.IsValid())
 {
  Point = Hit->ImpactPoint;
  if (!Hit->ImpactNormal.IsNearlyZero()) { Normal = Hit->ImpactNormal; }
 }
 FVector Direction = -Normal;
 if (const auto* Projectile = Cast<AAscendRangedProjectile>(DamageSource))
 {
  Direction = Projectile->GetActorForwardVector();
  if (const auto* Movement = Projectile->FindComponentByClass<UProjectileMovementComponent>(); Movement && !Movement->Velocity.IsNearlyZero()) { Direction = Movement->Velocity.GetSafeNormal(); }
 }
 MulticastHitReaction(Montage,Point,Normal,Direction);
}

void AAscendEnemyCharacter::MulticastHitReaction_Implementation(UAnimMontage* Montage,FVector HitPoint,FVector HitNormal,FVector ProjectileDirection)
{
 if (IsDead() || !Montage || !MeleeCombat || !MeleeCombat->Profile) { return; }
 UAnimInstance* Anim = GetMesh()->GetAnimInstance();
 if (!Anim) { return; }
 UAnimMontage* PreviousHit = ActiveHitMontage.Get();
 ActiveHitMontage.Reset();
 HitMontageInstanceID = INDEX_NONE;
 if (PreviousHit) { Anim->Montage_Stop(0.08f, PreviousHit); }
 if (UAnimMontage* Attack = ActiveRangedMontage.Get())
 {
  Anim->Montage_Stop(0.f, Attack);
  OnRangedAttackEnded(Attack, true);
 }
 MeleeCombat->CancelAttack();
 if (Anim->Montage_Play(Montage, FMath::Max(MeleeCombat->Profile->HitReactionPlayRate, .1f)) <= 0.f)
 {
  MeleeCombat->UnlockAttackMovement();
  return;
 }
 ActiveHitMontage = Montage;
 HitEffectPoint = HitPoint;
 HitEffectNormal = HitNormal;
 HitProjectileDirection = ProjectileDirection;
 bHitVFXEmitted = false;
 bHitDecalEmitted = false;
 const FAnimMontageInstance* Instance = Anim->GetActiveInstanceForMontage(Montage);
 HitMontageInstanceID = Instance ? Instance->GetInstanceID() : INDEX_NONE;
 MeleeCombat->LockAttackMovement();
 FOnMontageEnded EndDelegate;
 const int32 InstanceID = HitMontageInstanceID;
 EndDelegate.BindWeakLambda(this, [this, InstanceID](UAnimMontage* EndedMontage, bool)
 {
  if (HitMontageInstanceID != InstanceID || ActiveHitMontage.Get() != EndedMontage) { return; }
  ActiveHitMontage.Reset();
  HitMontageInstanceID = INDEX_NONE;
  MeleeCombat->UnlockAttackMovement();
 });
 Anim->Montage_SetEndDelegate(EndDelegate, Montage);
 UE_LOG(LogTemp, Display, TEXT("[AscendHitReaction] %s played %s."), *GetName(), *Montage->GetName());
}

void AAscendEnemyCharacter::HandleDeathStarted()
{
 UAnimMontage* Hit = ActiveHitMontage.Get();
 ActiveHitMontage.Reset();
 HitMontageInstanceID = INDEX_NONE;
 if (UAnimInstance* Anim = GetMesh()->GetAnimInstance(); Anim && Hit) { Anim->Montage_Stop(0.f, Hit); }
 if (MeleeCombat) { MeleeCombat->UnlockAttackMovement(); }
 Super::HandleDeathStarted();
}

void AAscendEnemyCharacter::EmitHitEffects(UAnimSequenceBase* Animation,int32 InstanceID,UAscendImpactFXProfile* Override,bool bVFX,bool bDecal)
{
 if (IsDead() || !IsHitReacting() || Animation != ActiveHitMontage.Get() || (InstanceID != INDEX_NONE && InstanceID != HitMontageInstanceID)) { return; }
 const UAscendImpactFXProfile* FX = Override ? Override : (MeleeCombat && MeleeCombat->Profile ? MeleeCombat->Profile->HitFXProfile.Get() : nullptr);
 if (!FX) { return; }
 const bool SpawnVFX = bVFX && !bHitVFXEmitted;
 const bool SpawnDecal = bDecal && !bHitDecalEmitted;
 bHitVFXEmitted |= SpawnVFX;
 bHitDecalEmitted |= SpawnDecal;
 if (SpawnVFX || SpawnDecal) { FX->SpawnEffects(this,HitEffectPoint,HitEffectNormal,SpawnVFX,SpawnDecal,HitProjectileDirection); }
}
