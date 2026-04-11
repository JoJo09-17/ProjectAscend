#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "AscendDeathAbility.generated.h"

/**
 * Minimal death ability that finalizes death on the owning character.
 * This keeps death in the GAS lane when a project wants to add montage, cues, or delays later.
 */
UCLASS()
class PROJECTASCEND_API UAscendDeathAbility : public UAscendGameplayAbility
{
	GENERATED_BODY()

public:
	UAscendDeathAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
