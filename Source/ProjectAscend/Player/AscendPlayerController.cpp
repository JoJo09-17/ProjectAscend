#include "AscendPlayerController.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "Input/AscendInputComponent.h"
#include "Player/AscendAbilitySlotComponent.h"

AAscendPlayerController::AAscendPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	AbilitySlotComponent = CreateDefaultSubobject<UAscendAbilitySlotComponent>(TEXT("AbilitySlotComponent"));
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
	Super::OnUnPossess();
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

UAscendAbilitySystemComponent* AAscendPlayerController::GetASC() const
{
	if (const IAbilitySystemInterface* IAS = Cast<IAbilitySystemInterface>(GetPawn()))
	{
		return Cast<UAscendAbilitySystemComponent>(IAS->GetAbilitySystemComponent());
	}
	return nullptr;
}
