#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "AscendTargetType_SphereOverlapFromEventData.generated.h"

/** Performs a sphere overlap centered on the event origin. */
USTRUCT(BlueprintType, meta = (DisplayName = "Target Type - Sphere Overlap From Event Data"))
struct PROJECTASCEND_API FAscendTargetType_SphereOverlapFromEventData : public FAscendTargetType
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
	float Radius = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting")
	TEnumAsByte<ECollisionChannel> QueryChannel = ECC_Pawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting")
	bool bIgnoreSourceActor = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug")
	bool bDrawDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	FLinearColor DebugColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Targeting|Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugThickness = 1.5f;
};
