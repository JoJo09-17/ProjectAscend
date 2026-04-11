#pragma once

#include "AscendAbilityActivationGroup.generated.h"

/**
 * Defines how an ability participates in activation concurrency on the owner's ASC.
 */
UENUM(BlueprintType)
enum class EAscendAbilityActivationGroup : uint8
{
	Independent,
	ExclusiveReplaceable,
	ExclusiveBlocking,
	MAX UMETA(Hidden)
};
