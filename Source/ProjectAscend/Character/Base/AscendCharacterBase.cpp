#include "AscendCharacterBase.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "AbilitySystem/Attributes/AscendAttributeTypes.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AbilitySystem/Data/AscendAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/AscendPlayerState.h"
#include "GameplayEffect.h"
#include "AscendGameplayTags.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Materials/AscendMaterialControllerComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/AscendRangedProjectile.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Character/Player/AscendCharacterMovementComponent.h"

float AAscendCharacterBase::GetCombatAttackSpeed() const
{
	float Speed = 1.f;
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent(); ASC && ASC->HasAttributeSetForAttribute(UAscendAttributeSet::GetAttackSpeedAttribute()))
	{
		Speed = FMath::Clamp(ASC->GetNumericAttribute(UAscendAttributeSet::GetAttackSpeedAttribute()), 0.1f, 3.f);
	}
	if (MeleeCombat && MeleeCombat->Profile) { Speed *= FMath::Max(MeleeCombat->Profile->AttackSpeedMultiplier, 0.1f); }
	return Speed;
}

bool AAscendCharacterBase::TryInterruptNormalAttack()
{
	if (IsDead() || (IsRangedAttacking() && bRangedHeavyAttack) || (MeleeCombat && MeleeCombat->IsHeavyAttacking())) { return false; }
	if (IsRangedAttacking())
	{
		UAnimMontage* Montage = ActiveRangedMontage.Get();
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance()) { Anim->Montage_Stop(0.f, Montage); }
		// Release immediately even if the animation instance was replaced.
		OnRangedAttackEnded(Montage, true);
		RangedComboIndex = 0;
	}
	if (MeleeCombat) { MeleeCombat->CancelAttack(); }
	return true;
}

bool AAscendCharacterBase::PlayRangedAttackAnimation(bool bHeavy, float ChargeAlpha, bool bContinueCombo)
{
	if (IsDead()) { return false; }
	if (const auto* Enemy = Cast<AAscendEnemyCharacter>(this); Enemy && Enemy->IsHitReacting()) { return false; }
	if (!MeleeCombat || !MeleeCombat->Profile) { return true; }
	const UAscendMeleeProfile* Profile = MeleeCombat->Profile;
	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	if (IsRangedAttacking() && (!bContinueCombo || bHeavy || !CanAdvanceRangedCombo())) { return false; }
	if (!IsRangedAttacking() && GetWorld()->GetTimeSeconds() - LastRangedAttackTime > Profile->RangedComboResetDelay) { RangedComboIndex = 0; }
	UAnimMontage* Montage = bHeavy ? Profile->RangedHeavyMontage.Get() :
		(Profile->RangedLightMontages.IsEmpty() ? nullptr : Profile->RangedLightMontages[RangedComboIndex % Profile->RangedLightMontages.Num()].Get());
	FAscendRangedAttack Shot = bHeavy ? Profile->RangedHeavyAttack : (IsPlayerControlled() ? Profile->RangedLightAttack : Profile->EnemyRangedAttack);
	if (bHeavy)
	{
		const float Charge = FMath::Clamp(ChargeAlpha, 0.f, 1.f);
		Shot.Damage *= FMath::Lerp(1.f, 2.f, Charge);
		Shot.ProjectileRadius *= FMath::Lerp(1.f, 1.5f, Charge);
	}
	const FVector Direction = GetActorForwardVector().GetSafeNormal2D();
	if (!Montage) { SpawnRangedProjectile(Shot, Direction); return true; }
	const float AttackSpeed = GetCombatAttackSpeed();
	if (!Anim || Montage->GetSkeleton() != GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()) { return false; }
	UAnimMontage* Previous = ActiveRangedMontage.Get();
	// Ignore the old instance's end callback while blending directly into the next.
	ActiveRangedMontage.Reset();
	if (Anim->Montage_Play(Montage, AttackSpeed) <= 0.f) { ActiveRangedMontage = Previous; return false; }
	ActiveRangedMontage = Montage;
	const FAnimMontageInstance* Instance = Anim->GetActiveInstanceForMontage(Montage);
	ActiveRangedMontageInstanceID = Instance ? Instance->GetInstanceID() : INDEX_NONE;
	bRangedHeavyAttack = bHeavy;
	bRangedShotReleased = false;
	bRangedComboWindowOpen = false;
	PendingRangedAttack = Shot;
	RangedShotDirection = Direction;
	MeleeCombat->LockAttackMovement();
	FOnMontageEnded EndDelegate;
	const int32 InstanceID = ActiveRangedMontageInstanceID;
	EndDelegate.BindWeakLambda(this, [this, InstanceID](UAnimMontage* EndingMontage, bool bInterrupted)
	{
		if (ActiveRangedMontageInstanceID == InstanceID) { OnRangedAttackEnded(EndingMontage, bInterrupted); }
	});
	Anim->Montage_SetEndDelegate(EndDelegate, Montage);
	LastRangedAttackTime = GetWorld()->GetTimeSeconds() + Montage->GetPlayLength() / AttackSpeed;
	RangedComboIndex = bHeavy ? 0 : RangedComboIndex + 1;
	UE_LOG(LogTemp, Display, TEXT("[AscendRanged] %s played %s."), *GetName(), *Montage->GetName());
	return true;
}

void AAscendCharacterBase::SpawnRangedProjectile(const FAscendRangedAttack& Attack, const FVector& Direction)
{
	if (!HasAuthority() || IsDead() || !GetWorld()) { return; }
	const FVector Origin = GetActorLocation() + FVector(0,0,55);
	const FTransform SpawnTransform(Direction.Rotation(), Origin);
	if (auto* Projectile = GetWorld()->SpawnActorDeferred<AAscendRangedProjectile>(AAscendRangedProjectile::StaticClass(),SpawnTransform,this,this,ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		Projectile->SetProjectileProfile(Attack.ProjectileProfile);
		Projectile->InitializeProjectile(Direction, Attack.Damage, Attack.ProjectileSpeed, Attack.ProjectileRadius, Attack.bPierceEnemies);
		Projectile->FinishSpawning(SpawnTransform);
		UE_LOG(LogTemp, Display, TEXT("[AscendRanged] %s released shot; montage=%s damage=%.1f"), *GetName(), *GetNameSafe(ActiveRangedMontage.Get()), Attack.Damage);
	}
}

void AAscendCharacterBase::ReleaseRangedShot(UAnimSequenceBase* Animation)
{
	if (!IsRangedAttacking() || Animation != ActiveRangedMontage.Get() || bRangedShotReleased || IsDead()) { return; }
	bRangedShotReleased = true;
	SpawnRangedProjectile(PendingRangedAttack, RangedShotDirection);
}

void AAscendCharacterBase::SetRangedComboWindow(UAnimSequenceBase* Animation, bool bOpen)
{
	if (!IsRangedAttacking() || Animation != ActiveRangedMontage.Get() || bRangedHeavyAttack || IsDead()) { return; }
	bRangedComboWindowOpen = bOpen;
	if (CanAdvanceRangedCombo()) { OnRangedComboReady.Broadcast(); }
}

void AAscendCharacterBase::OnRangedAttackEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (ActiveRangedMontage.Get() != Montage) { return; }
	ActiveRangedMontage.Reset();
	ActiveRangedMontageInstanceID = INDEX_NONE;
	bRangedHeavyAttack = false;
	bRangedShotReleased = false;
	bRangedComboWindowOpen = false;
	LastRangedAttackTime = GetWorld()->GetTimeSeconds();
	if (bInterrupted) { RangedComboIndex = 0; }
	if (MeleeCombat) { MeleeCombat->UnlockAttackMovement(); }
	OnRangedAttackFinished.Broadcast(bInterrupted);
}

AAscendCharacterBase::AAscendCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UAscendCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	MeleeCombat = CreateDefaultSubobject<UAscendMeleeCombatComponent>(TEXT("MeleeCombat"));
	MaterialController = CreateDefaultSubobject<UAscendMaterialControllerComponent>(TEXT("MaterialController"));
}

UAbilitySystemComponent* AAscendCharacterBase::GetAbilitySystemComponent() const
{
	if (AAscendPlayerState* PS = GetPlayerState<AAscendPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}

	return AbilitySystemComponent;
}

void AAscendCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeGAS();
}

void AAscendCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeGAS();
}

void AAscendCharacterBase::InitializeGAS()
{
	UAscendAbilitySystemComponent* ResolvedASC = nullptr;

	if (AAscendPlayerState* PS = GetPlayerState<AAscendPlayerState>())
	{
		ResolvedASC = Cast<UAscendAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (ResolvedASC)
		{
			ResolvedASC->InitAbilityActorInfo(PS, this);
		}
	}
	else
	{
		ResolvedASC = AbilitySystemComponent;
		if (ResolvedASC)
		{
			ResolvedASC->InitAbilityActorInfo(this, this);
		}
	}

	AbilitySystemComponent = ResolvedASC;

	if (!AbilitySystemComponent)
	{
		return;
	}

	if (AbilitySet && !bStartupAbilitiesGranted)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr, this);
		bStartupAbilitiesGranted = true;
	}

	const bool bUseLegacyDefaultAttributes =
		DefaultAttributes &&
		!bDefaultAttributesApplied &&
		(!AbilitySet || !AbilitySet->HasAttributeInitializers());

	if (bUseLegacyDefaultAttributes)
	{
		const UGameplayEffect* DefaultAttributesGE = DefaultAttributes->GetDefaultObject<UGameplayEffect>();
		if (DefaultAttributesGE)
		{
			AbilitySystemComponent->ApplyGameplayEffectToSelf(
				DefaultAttributesGE,
				1.0f,
				AbilitySystemComponent->MakeEffectContext());
			bDefaultAttributesApplied = true;
		}
	}
}

void AAscendCharacterBase::Die()
{
	FAscendAttributeSetExecutionData ExecutionData;
	HandleOutOfHealth(ExecutionData);
}

void AAscendCharacterBase::HandleOutOfHealth(const FAscendAttributeSetExecutionData& ExecutionData)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	HandleDeathStarted();

	UAscendAbilitySystemComponent* AscendASC = Cast<UAscendAbilitySystemComponent>(GetAbilitySystemComponent());
	if (AscendASC && !bDeathFinished)
	{
		const FGameplayTag EffectiveDeathTag = DeathAbilityTag.IsValid()
			? DeathAbilityTag
			: AscendGameplayTags::Ability_Type_Death;

		FGameplayAbilitySpecHandle DeathAbilityHandle;
		if (EffectiveDeathTag.IsValid() &&
			AscendASC->FindFirstAbilityHandleByTag(EffectiveDeathTag, DeathAbilityHandle) &&
			DeathAbilityHandle.IsValid())
		{
			FGameplayEventData EventData;
			EventData.Instigator = ExecutionData.SourceActor.Get();
			EventData.Target = this;
			EventData.ContextHandle = ExecutionData.Context;
			EventData.EventMagnitude = FMath::Max(-ExecutionData.DeltaValue, 0.0f);
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AscendGameplayTags::Event_Death, EventData);
			return;
		}
	}

	FinishDeath();
}

void AAscendCharacterBase::FinishDeath()
{
	if (bDeathFinished)
	{
		return;
	}

	bDeathFinished = true;
	HandleDeathFinished();
	OnCharacterDied.Broadcast(this);
}

void AAscendCharacterBase::HandleDeathStarted()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance(); Anim && ActiveRangedMontage.IsValid())
	{
		Anim->Montage_Stop(0.f, ActiveRangedMontage.Get());
	}
	ActiveRangedMontage.Reset();
	if (MeleeCombat) { MeleeCombat->CancelAttack(); }
}

void AAscendCharacterBase::HandleDeathFinished()
{
}
