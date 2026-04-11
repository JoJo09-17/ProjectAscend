#include "AbilitySystem/Targeting/TargetTypes/AscendTargetType_UseEventData.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbilityTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendTargetType_UseEventData)

void FAscendTargetType_UseEventData::GetTargets(
	AAscendCharacterBase* TargetingCharacter,
	AActor* TargetingActor,
	const FGameplayEventData& EventData,
	TArray<FHitResult>& OutHitResults,
	TArray<AActor*>& OutActors) const
{
	if (const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult())
	{
		OutHitResults.Add(*FoundHitResult);
	}

	for (int32 TargetDataIndex = 0; TargetDataIndex < EventData.TargetData.Num(); ++TargetDataIndex)
	{
		const FGameplayAbilityTargetData* TargetData = EventData.TargetData.Get(TargetDataIndex);
		if (!TargetData)
		{
			continue;
		}

		if (TargetData->HasHitResult())
		{
			if (const FHitResult* TargetHitResult = TargetData->GetHitResult())
			{
				OutHitResults.Add(*TargetHitResult);
			}
		}

		const TArray<TWeakObjectPtr<AActor>> TargetActors = TargetData->GetActors();
		for (const TWeakObjectPtr<AActor>& TargetActor : TargetActors)
		{
			if (TargetActor.IsValid())
			{
				OutActors.AddUnique(TargetActor.Get());
			}
		}
	}

	if (EventData.Target)
	{
		OutActors.AddUnique(const_cast<AActor*>(EventData.Target.Get()));
	}
}
