#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Character/Player/AscendPlayerState.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AAscendEnemyCharacter::AAscendEnemyCharacter()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAscendAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

void AAscendEnemyCharacter::HandleDeathFinished()
{
	Super::HandleDeathFinished();
	// Ragdoll the mesh and disable navigation collision
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	SetLifeSpan(3.0f);
}

void AAscendEnemyCharacter::InitializeGAS()
{
	Super::InitializeGAS();
}
