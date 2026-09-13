#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AscendEnemyAIController.generated.h"

class AAscendEnemyCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * Shared enemy brain. Perception and target ownership live here, while the
 * possessed enemy's combat style selects a ranged or future melee policy.
 */
UCLASS()
class PROJECTASCEND_API AAscendEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AAscendEnemyAIController();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UFUNCTION()
	void HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateRangedCombat(AAscendEnemyCharacter& Enemy, APawn& Target, float DeltaSeconds);
	void UpdateMeleeCombat(AAscendEnemyCharacter& Enemy, APawn& Target, float DeltaSeconds);
	bool MoveInDirection(AAscendEnemyCharacter& Enemy, const FVector& Direction);
	void FireProjectile(AAscendEnemyCharacter& Enemy, APawn& Target);
	bool IsValidCombatTarget(const AActor* Actor) const;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> SightPerception;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	TWeakObjectPtr<APawn> CombatTarget;
	double NextAttackTime = 0.0;
};
