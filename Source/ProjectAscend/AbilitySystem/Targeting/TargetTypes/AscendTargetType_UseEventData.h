#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "AscendTargetType_UseEventData.generated.h"

/** Reads hit results and actors from the supplied gameplay event payload. */
USTRUCT(BlueprintType, meta = (DisplayName = "Target Type - Use Event Data"))
struct PROJECTASCEND_API FAscendTargetType_UseEventData : public FAscendTargetType
{
	GENERATED_BODY()

public:
	virtual void GetTargets(
		AAscendCharacterBase* TargetingCharacter,
		AActor* TargetingActor,
		const FGameplayEventData& EventData,
		TArray<FHitResult>& OutHitResults,
		TArray<AActor*>& OutActors) const override;
};
