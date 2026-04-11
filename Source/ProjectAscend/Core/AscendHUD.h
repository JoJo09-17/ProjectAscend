#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AscendHUD.generated.h"

class UUserWidget;

/**
 * Heads-up display for the ARPG overlay. Creates and manages the main HUD widget.
 */
UCLASS()
class PROJECTASCEND_API AAscendHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainWidget;
};
