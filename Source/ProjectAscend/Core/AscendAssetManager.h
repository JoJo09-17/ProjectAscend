#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AscendAssetManager.generated.h"

/**
 * Project-specific asset manager that initializes GAS global data during early loading.
 */
UCLASS()
class PROJECTASCEND_API UAscendAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	virtual void StartInitialLoading() override;
};
