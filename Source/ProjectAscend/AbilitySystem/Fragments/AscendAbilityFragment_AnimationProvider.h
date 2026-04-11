#pragma once

#include "CoreMinimal.h"
#include "AscendAbilityFragment.h"
#include "AscendAbilityFragment_AnimationProvider.generated.h"

class UAscendAnimationProvider;

/**
 * References an animation provider object that supplies montage or motion data
 * to the ability at runtime.
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Ability Fragment - Animation Provider"))
struct PROJECTASCEND_API FAscendAbilityFragment_AnimationProvider : public FAscendAbilityFragment
{
	GENERATED_BODY()

public:
	FAscendAbilityFragment_AnimationProvider()
	{
		FragmentTag = AscendGameplayTags::Fragment_AnimationProvider;
	}
	/** Instanced animation provider responsible for supplying animation data. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Ascend|Animation")
	TObjectPtr<UAscendAnimationProvider> Provider = nullptr;
};
