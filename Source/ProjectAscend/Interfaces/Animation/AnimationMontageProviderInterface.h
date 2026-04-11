#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "AnimationMontageProviderInterface.generated.h"

/**
 * Interface for objects that resolve animation montage data based on ability tags.
 * Implement on components or actors that drive ability animations (e.g. weapon
 * actors, stance components) so that gameplay abilities can query montage assets
 * without hard-coding animation references.
 */
UINTERFACE()
class UAnimationMontageProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTASCEND_API IAnimationMontageProviderInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	UAnimMontage* GetAnimationMontage(FGameplayTagContainer AbilityTags) const;
	virtual UAnimMontage* GetAnimationMontage_Implementation(FGameplayTagContainer AbilityTags) const { return nullptr; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	FName GetSectionName(FGameplayTagContainer AbilityTags) const;
	virtual FName GetSectionName_Implementation(FGameplayTagContainer AbilityTags) const { return NAME_None; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	float GetPlayRate(FGameplayTagContainer AbilityTags) const;
	virtual float GetPlayRate_Implementation(FGameplayTagContainer AbilityTags) const { return 1.f; }
};
