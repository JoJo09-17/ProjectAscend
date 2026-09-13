#pragma once
#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AscendAnimationAbility.h"
#include "AscendMeleeProfile.h"
#include "AscendMeleeAbility.generated.h"

class UAscendMeleeCombatComponent;

UCLASS()
class PROJECTASCEND_API UAscendMeleeAbility : public UAscendAnimationAbility
{
	GENERATED_BODY()
public:
	UAscendMeleeAbility();
	virtual bool CanActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags=nullptr, const FGameplayTagContainer* TargetTags=nullptr,
		FGameplayTagContainer* OptionalRelevantTags=nullptr) const override;
	virtual void ActivateAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool ResolveAnimationPlaybackData_Implementation(FAscendAbilityAnimationPlaybackData& OutData) const override;
private:
	TWeakObjectPtr<UAscendMeleeCombatComponent> Combat;
	FAscendMeleeAttack Attack;
};
