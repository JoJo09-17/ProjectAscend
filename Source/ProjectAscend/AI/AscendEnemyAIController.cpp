#include "AI/AscendEnemyAIController.h"

#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Combat/AscendRangedProjectile.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AAscendEnemyAIController::AAscendEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	bSetControlRotationFromPawnOrientation = false;

	SightPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("SightPerception"));
	SetPerceptionComponent(*SightPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2600.0f;
	SightConfig->LoseSightRadius = 3000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 180.0f;
	SightConfig->SetMaxAge(1.5f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightPerception->ConfigureSense(*SightConfig);
	SightPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	SightPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::HandleTargetPerceptionUpdated);
}

void AAscendEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CombatTarget.Reset();
	NextAttackTime = 0.0;

	if (const AAscendEnemyCharacter* Enemy = Cast<AAscendEnemyCharacter>(InPawn))
	{
		SightConfig->SightRadius = Enemy->GetAwarenessRange();
		SightConfig->LoseSightRadius = Enemy->GetAwarenessRange() * 1.15f;
		SightPerception->ConfigureSense(*SightConfig);
	}
}

void AAscendEnemyAIController::OnUnPossess()
{
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	CombatTarget.Reset();
	Super::OnUnPossess();
}

void AAscendEnemyAIController::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	APawn* SensedPawn = Cast<APawn>(Actor);
	if (!SensedPawn || !SensedPawn->IsPlayerControlled())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed() && IsValidCombatTarget(SensedPawn))
	{
		CombatTarget = SensedPawn;
		SetFocus(SensedPawn, EAIFocusPriority::Gameplay);
	}
	else if (CombatTarget.Get() == SensedPawn)
	{
		CombatTarget.Reset();
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

bool AAscendEnemyAIController::IsValidCombatTarget(const AActor* Actor) const
{
	if (!IsValid(Actor) || Actor->IsActorBeingDestroyed())
	{
		return false;
	}
	if (const AAscendCharacterBase* TargetCharacter = Cast<AAscendCharacterBase>(Actor))
	{
		return !TargetCharacter->IsDead();
	}
	return true;
}

void AAscendEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AAscendEnemyCharacter* Enemy = Cast<AAscendEnemyCharacter>(GetPawn());
	APawn* Target = CombatTarget.Get();
	if (!Enemy || Enemy->IsDead() || !IsValidCombatTarget(Target))
	{
		StopMovement();
		CombatTarget.Reset();
		return;
	}

	if (Enemy->IsHitReacting()) { StopMovement(); return; }
	const float DistanceSquared = FVector::DistSquared2D(Enemy->GetActorLocation(), Target->GetActorLocation());
	if (DistanceSquared > FMath::Square(Enemy->GetAwarenessRange() * 1.15f))
	{
		CombatTarget.Reset();
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	if (Enemy->GetCombatStyle() == EAscendEnemyCombatStyle::Ranged)
	{
		UpdateRangedCombat(*Enemy, *Target, DeltaSeconds);
	}
	else { UpdateMeleeCombat(*Enemy, *Target, DeltaSeconds); }
}

void AAscendEnemyAIController::UpdateMeleeCombat(AAscendEnemyCharacter& Enemy, APawn& Target, float DeltaSeconds)
{
	if (!Enemy.MeleeCombat || !Enemy.MeleeCombat->Profile) { StopMovement(); return; }
	if (Enemy.MeleeCombat->IsAttacking())
	{
		StopMovement();
		Enemy.GetCharacterMovement()->StopMovementImmediately();
		return;
	}
	const FVector Offset = Target.GetActorLocation() - Enemy.GetActorLocation();
	const FVector Direction = Offset.GetSafeNormal2D();
	Enemy.SetActorRotation(FMath::RInterpTo(Enemy.GetActorRotation(), Direction.Rotation(), DeltaSeconds, 10.f));
	if (Offset.Size2D() > Enemy.GetMeleeAttackRange())
	{
		if (MoveToActor(&Target, Enemy.GetMeleeAttackRange() * 0.7f, false, true, true, nullptr, true) == EPathFollowingRequestResult::Failed)
			MoveInDirection(Enemy, Direction);
		return;
	}
	StopMovement();
	Enemy.GetCharacterMovement()->StopMovementImmediately();
	if (FMath::Abs(Offset.Z) < 100.f && GetWorld()->GetTimeSeconds() >= NextAttackTime && LineOfSightTo(&Target)
		&& FVector::DotProduct(Enemy.GetActorForwardVector(), Direction) > 0.8f)
	{
		Enemy.SetActorRotation(Direction.Rotation());
		if (Enemy.MeleeCombat->RequestAttack(false))
		{
			NextAttackTime = GetWorld()->GetTimeSeconds() + Enemy.GetMeleeAttackCooldown() / Enemy.GetCombatAttackSpeed();
		}
	}
}

void AAscendEnemyAIController::UpdateRangedCombat(AAscendEnemyCharacter& Enemy, APawn& Target, float DeltaSeconds)
{
	if (Enemy.IsRangedAttacking()) { StopMovement(); return; }
	const FVector ToTarget = Target.GetActorLocation() - Enemy.GetActorLocation();
	const float Distance = ToTarget.Size2D();
	const FVector TargetDirection = ToTarget.GetSafeNormal2D();
	if (!TargetDirection.IsNearlyZero())
	{
		Enemy.SetActorRotation(FMath::RInterpTo(
			Enemy.GetActorRotation(), TargetDirection.Rotation(), DeltaSeconds, 12.0f));
	}

	if (Distance < Enemy.GetRetreatDistance())
	{
		MoveInDirection(Enemy, -TargetDirection);
	}
	else if (Distance > Enemy.GetPreferredAttackDistance())
	{
		if (MoveToActor(&Target, Enemy.GetPreferredAttackDistance() * 0.9f, true, true, true, nullptr, true) ==
			EPathFollowingRequestResult::Failed)
		{
			MoveInDirection(Enemy, TargetDirection);
		}
	}
	else
	{
		StopMovement();
		Enemy.GetCharacterMovement()->StopMovementImmediately();
	}

	if (Distance <= Enemy.GetAttackRange() && GetWorld()->GetTimeSeconds() >= NextAttackTime && LineOfSightTo(&Target))
	{
		FireProjectile(Enemy, Target);
		NextAttackTime = GetWorld()->GetTimeSeconds() + Enemy.GetAttackCooldown() / Enemy.GetCombatAttackSpeed();
	}
}

bool AAscendEnemyAIController::MoveInDirection(AAscendEnemyCharacter& Enemy, const FVector& Direction)
{
	const FVector SafeDirection = Direction.GetSafeNormal2D();
	if (SafeDirection.IsNearlyZero())
	{
		return false;
	}

	const FVector DesiredLocation = Enemy.GetActorLocation() + SafeDirection * 500.0f;
	if (UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation ProjectedLocation;
		if (Navigation->ProjectPointToNavigation(DesiredLocation, ProjectedLocation, FVector(300.0f, 300.0f, 300.0f)) &&
			MoveToLocation(ProjectedLocation.Location, 50.0f, true, true, true, true, nullptr, true) !=
			EPathFollowingRequestResult::Failed)
		{
			return true;
		}
	}

	StopMovement();
	if (UCharacterMovementComponent* Movement = Enemy.GetCharacterMovement())
	{
		// RequestDirectMove drives CharacterMovement even when the level has no
		// usable NavMesh. This keeps collision and acceleration while avoiding a
		// path-following request that can be accepted but never produce velocity.
		Movement->RequestDirectMove(SafeDirection * Movement->MaxWalkSpeed, false);
		return true;
	}

	Enemy.AddMovementInput(SafeDirection, 1.0f, true);
	return false;
}

void AAscendEnemyAIController::FireProjectile(AAscendEnemyCharacter& Enemy, APawn& Target)
{
	if (Enemy.IsRangedAttacking()) { return; }
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = Enemy.GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
	const FVector AimPoint = Target.GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FVector Direction = (AimPoint - Origin).GetSafeNormal();
	const FVector Facing = Direction.GetSafeNormal2D();
	if (Facing.IsNearlyZero()) { return; }
	Enemy.SetActorRotation(Facing.Rotation());
	if (!Enemy.PlayRangedAttackAnimation()) { return; }
	// Authored ranged profiles emit from their release notify (or their explicit
	// no-animation fallback). Keep the legacy projectile path for unconfigured AI.
	if (Enemy.MeleeCombat && Enemy.MeleeCombat->Profile) { return; }

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = &Enemy;
	SpawnParameters.Instigator = &Enemy;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AAscendRangedProjectile* Projectile = World->SpawnActor<AAscendRangedProjectile>(
		AAscendRangedProjectile::StaticClass(), Origin, Direction.Rotation(), SpawnParameters))
	{
		Projectile->InitializeProjectile(
			Direction,
			Enemy.MeleeCombat && Enemy.MeleeCombat->Profile ? Enemy.MeleeCombat->Profile->EnemyRangedAttack.Damage : Enemy.GetProjectileDamage(),
			Enemy.MeleeCombat && Enemy.MeleeCombat->Profile ? Enemy.MeleeCombat->Profile->EnemyRangedAttack.ProjectileSpeed : Enemy.GetProjectileSpeed(),
			Enemy.MeleeCombat && Enemy.MeleeCombat->Profile ? Enemy.MeleeCombat->Profile->EnemyRangedAttack.ProjectileRadius : Enemy.GetProjectileRadius());
	}
}
