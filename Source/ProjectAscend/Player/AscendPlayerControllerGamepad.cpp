#include "Player/AscendPlayerController.h"

#include "AscendGameplayTags.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "InputCoreTypes.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"

FVector AAscendPlayerController::GamepadDirectionToWorld(const FVector2D& Stick) const
{
	const float CameraYaw = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation().Yaw : 0.0f;
	const FRotationMatrix Basis(FRotator(0.0f, CameraYaw, 0.0f));
	return Basis.GetUnitAxis(EAxis::X) * Stick.Y + Basis.GetUnitAxis(EAxis::Y) * Stick.X;
}

void AAscendPlayerController::ProcessGamepadInput(float DeltaTime)
{
	const AAscendCharacterBase* Avatar = Cast<AAscendCharacterBase>(GetPawn());
	if (!GetPawn() || (Avatar && Avatar->IsDead())) { LockedTarget.Reset(); return; }
	float MouseX = 0.0f, MouseY = 0.0f;
	GetInputMouseDelta(MouseX, MouseY);
	if (FMath::Abs(MouseX) + FMath::Abs(MouseY) > 1.0f) { bUsingGamepad = false; }
	if (!bUsingGamepad) { return; }
	if (!GamepadMoveValue.IsNearlyZero())
	{
		const FVector Direction = GamepadDirectionToWorld(GamepadMoveValue).GetSafeNormal2D();
		GetPawn()->AddMovementInput(Direction, FMath::Min(GamepadMoveValue.Size(), 1.0f));
		GetPawn()->SetActorRotation(Direction.Rotation());
	}
	const bool Switch = GamepadSwitchValue.Size() > 0.6f && !bTargetSwitchLatched;
	RefreshLockTarget(Switch ? GamepadSwitchValue.GetSafeNormal() : FVector2D::ZeroVector);
	if (GamepadSwitchValue.Size() > 0.6f) { bTargetSwitchLatched = true; }
	else if (GamepadSwitchValue.Size() < 0.3f) { bTargetSwitchLatched = false; }
	if (LockedTarget.IsValid())
	{
		DrawDebugSphere(GetWorld(), LockedTarget->GetActorLocation() + FVector(0, 0, 120), 18, 12, FColor::Yellow, false, -1, 0, 2);
	}
}

void AAscendPlayerController::BindCombatActions()
{
	UEnhancedInputComponent* Enhanced = CastChecked<UEnhancedInputComponent>(InputComponent);
	// Asset references are kept by the mapping context; physical keys live exclusively in IMC_Default.
	const TCHAR* Buttons[] = {
		TEXT("IA_LightAttack"), TEXT("IA_HeavyAttack"),
		TEXT("IA_GamepadLightAttack"), TEXT("IA_GamepadHeavyAttack"), TEXT("IA_GamepadDodge"),
		TEXT("IA_GamepadSkill1"), TEXT("IA_GamepadSkill2"), TEXT("IA_GamepadSkill3")
	};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Buttons); ++Index)
	{
		const FString Path = FString(TEXT("/Game/Input/Actions/")) + Buttons[Index];
		UInputAction* Action = LoadObject<UInputAction>(nullptr, *Path);
		if (!ensureMsgf(Action, TEXT("Missing input asset: %s"), *Path)) { continue; }
		const int32 Command = Index < 2 ? Index : Index - 2;
		const bool bGamepad = Index >= 2;
		Enhanced->BindAction(Action, ETriggerEvent::Started, this, &ThisClass::HandleMappedButton, Command, bGamepad, true);
		Enhanced->BindAction(Action, ETriggerEvent::Completed, this, &ThisClass::HandleMappedButton, Command, bGamepad, false);
		Enhanced->BindAction(Action, ETriggerEvent::Canceled, this, &ThisClass::HandleMappedButton, Command, bGamepad, false);
	}
	const TCHAR* Sticks[] = { TEXT("IA_GamepadMove"), TEXT("IA_GamepadSwitchTarget") };
	for (int32 Index = 0; Index < 2; ++Index)
	{
		UInputAction* Action = LoadObject<UInputAction>(nullptr, *(FString(TEXT("/Game/Input/Actions/")) + Sticks[Index]));
		if (!ensure(Action)) { continue; }
		for (ETriggerEvent Event : { ETriggerEvent::Triggered, ETriggerEvent::Completed, ETriggerEvent::Canceled })
		{
			Enhanced->BindAction(Action, Event, this, &ThisClass::HandleGamepadStick, Index == 0);
		}
	}
	if (UInputAction* Move = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_Move")))
	{
		Enhanced->BindAction(Move, ETriggerEvent::Triggered, this, &ThisClass::HandleKeyboardActivity);
	}
}

void AAscendPlayerController::HandleKeyboardActivity(const FInputActionValue& Value)
{
	bUsingGamepad = false;
}

void AAscendPlayerController::HandleGamepadStick(const FInputActionValue& Value, bool bMove)
{
	FVector2D& StoredValue = bMove ? GamepadMoveValue : GamepadSwitchValue;
	StoredValue = Value.Get<FVector2D>();
	if (!StoredValue.IsNearlyZero()) { bUsingGamepad = true; }
}

void AAscendPlayerController::HandleMappedButton(const FInputActionValue& Value, int32 Command, bool bGamepad, bool bPressed)
{
	if (bPressed) { bUsingGamepad = bGamepad; }
	if (Command >= 3)
	{
		const FGameplayTag Slots[] = { AscendGameplayTags::InputTag_Ability_Slot1, AscendGameplayTags::InputTag_Ability_Slot2, AscendGameplayTags::InputTag_Ability_Slot3 };
		if (bPressed) { Input_AbilityInputTagPressed(Slots[Command - 3]); }
		else { Input_AbilityInputTagReleased(Slots[Command - 3]); }
		return;
	}
	if (!bPressed) { return; }
	const AAscendCharacterBase* Avatar = Cast<AAscendCharacterBase>(GetPawn());
	if (!GetPawn() || (Avatar && Avatar->IsDead())) { return; }
	if (Command == 0) { FireLightAttack(); }
	else if (Command == 1) { FireHeavyAttack(); }
	else if (Command == 2) { PerformGamepadDodge(); }
}

void AAscendPlayerController::RefreshLockTarget(const FVector2D& SwitchDirection)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) { LockedTarget.Reset(); return; }
	const FVector Origin = ControlledPawn->GetActorLocation();
	auto IsCandidate = [&](AAscendEnemyCharacter* Enemy)
	{
		if (!IsValid(Enemy) || Enemy->IsDead() || Enemy->IsActorBeingDestroyed() ||
			FVector::DistSquared(Origin, Enemy->GetActorLocation()) > FMath::Square(LockRange)) { return false; }
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AscendLockTarget), false, ControlledPawn);
		// Only static geometry occludes lock-on; our projectiles must not break it.
		const bool Blocked = GetWorld()->LineTraceSingleByObjectType(Hit, Origin, Enemy->GetActorLocation(),
			FCollisionObjectQueryParams(ECC_WorldStatic), Params);
		return !Blocked;
	};
	if (!IsCandidate(LockedTarget.Get())) { LockedTarget.Reset(); }
	if (LockedTarget.IsValid() && SwitchDirection.IsNearlyZero()) { return; }
	const FVector Reference = LockedTarget.IsValid() ? LockedTarget->GetActorLocation() : Origin;
	const FVector Desired = GamepadDirectionToWorld(SwitchDirection).GetSafeNormal2D();
	AAscendEnemyCharacter* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();
	for (TActorIterator<AAscendEnemyCharacter> It(GetWorld()); It; ++It)
	{
		AAscendEnemyCharacter* Enemy = *It;
		if (Enemy == LockedTarget.Get() || !IsCandidate(Enemy)) { continue; }
		float Score = FVector::DistSquared(Origin, Enemy->GetActorLocation());
		if (!SwitchDirection.IsNearlyZero() && LockedTarget.IsValid())
		{
			const FVector Offset = Enemy->GetActorLocation() - Reference;
			const float Alignment = FVector::DotProduct(Offset.GetSafeNormal2D(), Desired);
			if (Alignment < 0.25f) { continue; }
			Score = (1.0f - Alignment) * LockRange + Offset.Size2D() * 0.1f;
		}
		if (Score < BestScore) { Best = Enemy; BestScore = Score; }
	}
	if (Best) { LockedTarget = Best; }
}

void AAscendPlayerController::PerformGamepadDodge()
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter || GetWorld()->GetTimeSeconds() < NextDodgeTime) { return; }
	UCharacterMovementComponent* Movement = ControlledCharacter->GetCharacterMovement();
	if (!Movement->IsMovingOnGround()) { return; }
	TSharedPtr<FRootMotionSource_ConstantForce> Dodge = MakeShared<FRootMotionSource_ConstantForce>();
	Dodge->InstanceName = TEXT("GamepadDodge");
	Dodge->Priority = 500;
	Dodge->AccumulateMode = ERootMotionAccumulateMode::Override;
	Dodge->Force = ControlledCharacter->GetActorForwardVector() * FMath::Max(DodgeSpeed, 0.0f);
	Dodge->Duration = FMath::Max(DodgeDuration, 0.01f);
	Dodge->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
	Dodge->FinishVelocityParams.ClampVelocity = Movement->MaxWalkSpeed;
	Movement->ApplyRootMotionSource(Dodge);
	NextDodgeTime = GetWorld()->GetTimeSeconds() + FMath::Max(DodgeCooldown, Dodge->Duration);
}
