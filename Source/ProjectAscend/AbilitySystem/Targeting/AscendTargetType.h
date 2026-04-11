#pragma once

#include "CoreMinimal.h"
#include "AscendTargetType.generated.h"

class AActor;
class AAscendCharacterBase;
struct FGameplayEventData;

/**
 * Base struct for effect-container targeting strategies.
 * Stored inline via TInstancedStruct so each container can author its own targeting parameters.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendTargetType
{
	GENERATED_BODY()

public:
	virtual ~FAscendTargetType() = default;

	virtual void GetTargets(
		AAscendCharacterBase* TargetingCharacter,
		AActor* TargetingActor,
		const FGameplayEventData& EventData,
		TArray<FHitResult>& OutHitResults,
		TArray<AActor*>& OutActors) const;
};
