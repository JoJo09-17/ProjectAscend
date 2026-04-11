#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AscendCharacterMovementComponent.generated.h"

/**
 * Extended character movement component for custom movement modes and GAS integration.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTASCEND_API UAscendCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UAscendCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
};
