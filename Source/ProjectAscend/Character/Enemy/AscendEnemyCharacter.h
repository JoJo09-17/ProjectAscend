#pragma once

#include "CoreMinimal.h"
#include "Character/Base/AscendCharacterBase.h"
#include "AscendEnemyCharacter.generated.h"

/**
 * AI-controlled enemy character with a self-contained ASC.
 */
UCLASS()
class PROJECTASCEND_API AAscendEnemyCharacter : public AAscendCharacterBase
{
	GENERATED_BODY()

public:
	AAscendEnemyCharacter();

protected:
	virtual void InitializeGAS() override;
	virtual void HandleDeathFinished() override;
};
