#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "AscendTargetType_BoxOverlapFromEventData.generated.h"

/** Performs a box overlap centered on the event origin. */
USTRUCT(BlueprintType, meta = (DisplayName = "Target Type - Box Overlap From Event Data"))
struct PROJECTASCEND_API FAscendTargetType_BoxOverlapFromEventData : public FAscendTargetType
{
	GENERATED_BODY()

public:
	virtual void GetTargets(
		AAscendCharacterBase* TargetingCharacter,
		AActor* TargetingActor,
		const FGameplayEventData& EventData,
		TArray<FHitResult>& OutHitResults,
		TArray<AActor*>& OutActors) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting")
	FVector HalfExtent = FVector(150.0f, 150.0f, 150.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting")
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_Pawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting")
	bool bIgnoreSourceActor = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	FLinearColor DebugColor = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugThickness = 1.5f;
};
