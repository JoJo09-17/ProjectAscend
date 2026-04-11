#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "AnimationContextProviderInterface.generated.h"

/**
 * Interface for objects that supply animation context data (context tags,
 * effect context handles, event magnitude) used by the animation system
 * to select and drive montage playback.
 */
UINTERFACE()
class UAnimationContextProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTASCEND_API IAnimationContextProviderInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	bool GetAnimationContext(FGameplayTagContainer& OutContextTags) const;
	virtual bool GetAnimationContext_Implementation(FGameplayTagContainer& OutContextTags) const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	bool GetEffectContext(FGameplayEffectContextHandle& OutHandle) const;
	virtual bool GetEffectContext_Implementation(FGameplayEffectContextHandle& OutHandle) const { return false; }

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Ascend|Animation")
	float GetLastEventMagnitude() const;
	virtual float GetLastEventMagnitude_Implementation() const { return 0.0f; }
};
