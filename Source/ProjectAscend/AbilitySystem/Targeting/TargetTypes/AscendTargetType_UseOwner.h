#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "AscendTargetType_UseOwner.generated.h"

/** Targets the actor that is executing the container. */
USTRUCT(BlueprintType, meta = (DisplayName = "Target Type - Use Owner"))
struct PROJECTASCEND_API FAscendTargetType_UseOwner : public FAscendTargetType
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
