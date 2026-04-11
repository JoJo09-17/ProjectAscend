#include "Core/AscendHUD.h"

#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UI/AscendAbilitySystemWidget.h"

void AAscendHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* OwnerPlayerController = GetOwningPlayerController();
	if (!MainWidgetClass || !OwnerPlayerController)
	{
		return;
	}

	MainWidget = CreateWidget<UUserWidget>(OwnerPlayerController, MainWidgetClass);
	if (!MainWidget)
	{
		return;
	}

	if (UAscendAbilitySystemWidget* AbilitySystemWidget = Cast<UAscendAbilitySystemWidget>(MainWidget))
	{
		UAscendAbilitySystemComponent* AbilitySystem = nullptr;
		if (APawn* Pawn = OwnerPlayerController->GetPawn())
		{
			UAscendAbilitySystemComponent::TryGetAscendAbilitySystem(Pawn, AbilitySystem);
		}

		if (!AbilitySystem && OwnerPlayerController->PlayerState)
		{
			UAscendAbilitySystemComponent::TryGetAscendAbilitySystem(
				static_cast<const UObject*>(OwnerPlayerController->PlayerState.Get()),
				AbilitySystem);
		}

		if (AbilitySystem)
		{
			AbilitySystemWidget->InitializeAbilitySystemWidget(AbilitySystem);
		}
	}

	MainWidget->AddToViewport();
}
