#include "AscendPlayerController.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "Combat/AscendRangedProjectile.h"
#include "Input/AscendInputComponent.h"
#include "Player/AscendAbilitySlotComponent.h"
#include "InputCoreTypes.h"
#include "Character/Enemy/AscendEnemyCharacter.h"

AAscendPlayerController::AAscendPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	AbilitySlotComponent = CreateDefaultSubobject<UAscendAbilitySlotComponent>(TEXT("AbilitySlotComponent"));
}

void AAscendPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BindCombatActions();
}

void AAscendPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	if (!InputConfig.IsEmpty())
	{
		BindingInput(InputConfig);
	}
}

void AAscendPlayerController::OnUnPossess()
{
	LockedTarget.Reset();
	bTargetSwitchLatched = false;
	GamepadMoveValue = FVector2D::ZeroVector;
	GamepadSwitchValue = FVector2D::ZeroVector;
	NextDodgeTime = 0.0;
	Super::OnUnPossess();
}

void AAscendPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateMouseFacing();
}

void AAscendPlayerController::UpdateMouseFacing()
{
	APawn* ControlledPawn = GetPawn();
	if (bUsingGamepad || !IsLocalController() || !ControlledPawn || IsPaused())
	{
		return;
	}

	FVector RayOrigin, RayDirection;
	if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection) || FMath::IsNearlyZero(RayDirection.Z))
	{
		return;
	}

	// Aim on the character's horizontal plane, so props and projectiles do not
	// make the aim jump when the cursor crosses their visibility collision.
	const FVector PawnLocation = ControlledPawn->GetActorLocation();
	const float Distance = (PawnLocation.Z - RayOrigin.Z) / RayDirection.Z;
	if (Distance <= 0.0f)
	{
		return;
	}
	const FVector Facing = (RayOrigin + Distance * RayDirection - PawnLocation).GetSafeNormal2D();
	if (!Facing.IsNearlyZero())
	{
		ControlledPawn->SetActorRotation(FRotator(0.0f, Facing.Rotation().Yaw, 0.0f));
	}
}

FVector AAscendPlayerController::GetMouseHitLocation()
{
	FHitResult Hit;
	GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, Hit);

	if (Hit.bBlockingHit)
	{
		return Hit.ImpactPoint;
	}

	return FVector::ZeroVector;
}

void AAscendPlayerController::BindingInput(const TMap<FGameplayTag, UInputAction*> InInputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* ControlPawn = GetPawn<APawn>();
	if (!ControlPawn)
	{
		return;
	}

	UAscendInputComponent* AscendIC = ControlPawn->FindComponentByClass<UAscendInputComponent>();
	check(AscendIC);
	AscendIC->BindAbilityActions(InInputConfig, this,
		&ThisClass::Input_AbilityInputTagPressed,
		&ThisClass::Input_AbilityInputTagReleased,
		BindHandles);
}

void AAscendPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (!bGamePaused && IsLocalController())
	{
		ProcessGamepadInput(DeltaTime);
	}
	// Let the ASC consume buffered input after the Enhanced Input stack has finished.
	if (UAscendAbilitySystemComponent* ASC = GetASC())
	{
		ASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	}
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AAscendPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UAscendAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagPressed(InputTag);
	}
}

void AAscendPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UAscendAbilitySystemComponent* ASC = GetASC())
	{
		ASC->AbilityInputTagReleased(InputTag);
	}
}

void AAscendPlayerController::FireLightAttack()
{
	FireRangedProjectile(LightAttackDamage, LightProjectileSpeed, LightProjectileRadius);
}

void AAscendPlayerController::FireHeavyAttack(float ChargeAlpha)
{
	const float ClampedChargeAlpha = FMath::Clamp(ChargeAlpha, 0.0f, 1.0f);
	const float Damage = HeavyAttackDamage * FMath::Lerp(1.0f, 2.0f, ClampedChargeAlpha);
	const float Radius = HeavyProjectileRadius * FMath::Lerp(1.0f, 1.5f, ClampedChargeAlpha);
	FireRangedProjectile(Damage, HeavyProjectileSpeed, Radius);
}

void AAscendPlayerController::FireRangedProjectile(float InDamage, float InSpeed, float InRadius)
{
	APawn* const ControlledPawn = GetPawn();
	UWorld* const World = GetWorld();
	if (!ControlledPawn || !World)
	{
		return;
	}

	UpdateMouseFacing();
	const FVector Origin = ControlledPawn->GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
	// Top-down attacks travel horizontally along the character's current facing.
	FVector Direction = FRotator(0.0f, ControlledPawn->GetActorRotation().Yaw, 0.0f).Vector();
	if (bUsingGamepad)
	{
		RefreshLockTarget();
		if (LockedTarget.IsValid())
		{
			const FVector TargetDirection = (LockedTarget->GetActorLocation() - Origin).GetSafeNormal2D();
			if (!TargetDirection.IsNearlyZero())
			{
				Direction = TargetDirection;
			}
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ControlledPawn;
	SpawnParams.Instigator = ControlledPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAscendRangedProjectile* Projectile = World->SpawnActor<AAscendRangedProjectile>(
		AAscendRangedProjectile::StaticClass(),
		Origin,
		Direction.Rotation(),
		SpawnParams);
	if (Projectile)
	{
		UE_LOG(LogTemp, Display, TEXT("[AscendCombat] Spawned projectile: damage=%.1f speed=%.0f radius=%.1f"), InDamage, InSpeed, InRadius);
		Projectile->InitializeProjectile(Direction, InDamage, InSpeed, InRadius);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AscendCombat] Failed to spawn projectile."));
	}
}

UAscendAbilitySystemComponent* AAscendPlayerController::GetASC() const
{
	if (const IAbilitySystemInterface* IAS = Cast<IAbilitySystemInterface>(GetPawn()))
	{
		return Cast<UAscendAbilitySystemComponent>(IAS->GetAbilitySystemComponent());
	}
	return nullptr;
}
