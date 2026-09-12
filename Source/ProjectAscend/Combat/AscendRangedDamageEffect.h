#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AscendRangedDamageEffect.generated.h"

/** Instant native effect that routes projectile damage through the standard Ascend damage execution. */
UCLASS()
class PROJECTASCEND_API UAscendRangedDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAscendRangedDamageEffect();
};
