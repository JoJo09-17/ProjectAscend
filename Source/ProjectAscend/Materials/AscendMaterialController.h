#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AscendMaterialController.generated.h"

class UTexture;

/** Immutable description of a buff, hit flash, dissolve or other MID effect. */
UCLASS(BlueprintType, meta = (DisplayName = "MaterialController"))
class PROJECTASCEND_API UAscendMaterialController : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Seconds; zero means persistent until explicitly removed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lifetime", meta = (ClampMin = "0.0"))
	float Duration = 0.2f;

	/** Higher priority wins for parameters shared by multiple controllers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition")
	int32 Priority = 0;

	/** Empty applies to every material slot on the target mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Composition")
	TArray<int32> MaterialSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FName, float> ScalarParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FName, FLinearColor> VectorParameters;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
	TMap<FName, TObjectPtr<UTexture>> TextureParameters;
};
