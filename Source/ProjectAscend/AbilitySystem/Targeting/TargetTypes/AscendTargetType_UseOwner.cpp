#include "AbilitySystem/Targeting/TargetTypes/AscendTargetType_UseOwner.h"

#include "Character/Base/AscendCharacterBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendTargetType_UseOwner)

void FAscendTargetType_UseOwner::GetTargets(
	AAscendCharacterBase* TargetingCharacter,
	AActor* TargetingActor,
	const FGameplayEventData& EventData,
	TArray<FHitResult>& OutHitResults,
	TArray<AActor*>& OutActors) const
{
	if (TargetingCharacter)
	{
		OutActors.Add(Cast<AActor>(TargetingCharacter));
	}
	else if (TargetingActor)
	{
		OutActors.Add(TargetingActor);
	}
}
