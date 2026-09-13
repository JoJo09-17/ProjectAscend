#include "AscendCharacterMovementComponent.h"

#include "Combat/AscendMeleeCombatComponent.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendCharacterMovementComponent)

UAscendCharacterMovementComponent::UAscendCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAscendCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}

FVector UAscendCharacterMovementComponent::ConstrainInputAcceleration(const FVector& InputAcceleration) const
{
 const auto* Combat = CharacterOwner ? CharacterOwner->FindComponentByClass<UAscendMeleeCombatComponent>() : nullptr;
 return Combat && Combat->IsAttackMovementLocked() ? FVector::ZeroVector : Super::ConstrainInputAcceleration(InputAcceleration);
}

void UAscendCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
 const auto* Combat = CharacterOwner ? CharacterOwner->FindComponentByClass<UAscendMeleeCombatComponent>() : nullptr;
 if (Combat && Combat->IsAttackMovementLocked())
 {
  Acceleration = FVector::ZeroVector;
  bHasRequestedVelocity = false;
  Velocity.X = Velocity.Y = 0.f;
 }
 Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
}
