#pragma once

#include "CoreMinimal.h"
#include "Character/Base/AscendCharacterBase.h"
#include "AscendPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

/**
 * Player-controlled character with a top-down camera and Enhanced Input support.
 * Uses AscendPlayerState as the ASC owner so abilities persist across possession changes.
 */
UCLASS()
class PROJECTASCEND_API AAscendPlayerCharacter : public AAscendCharacterBase
{
	GENERATED_BODY()

public:
	AAscendPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return CameraComponent; }
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return SpringArmComponent; }
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> IMC_Default;

	virtual void InitializeGAS() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	/** Applies or removes the matching GAS gameplay tag for the given movement mode. */
	void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComponent;
};
