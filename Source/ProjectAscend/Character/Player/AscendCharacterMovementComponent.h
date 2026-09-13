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
	friend class FAscendMeleeSweepTest;

public:
	UAscendCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual FVector ConstrainInputAcceleration(const FVector& InputAcceleration) const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
};
