#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

/** General project log category. */
PROJECTASCEND_API DECLARE_LOG_CATEGORY_EXTERN(LogAscend, Log, All);

/** Log category for ability system events (activation, input, slots). */
PROJECTASCEND_API DECLARE_LOG_CATEGORY_EXTERN(LogAscendAbilitySystem, Log, All);

/** Returns a string describing the current net mode (Client / Server / Standalone) for log context. */
PROJECTASCEND_API FString GetClientServerContextString(UObject* ContextObject = nullptr);
