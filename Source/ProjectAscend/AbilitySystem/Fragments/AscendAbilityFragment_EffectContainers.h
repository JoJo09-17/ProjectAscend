#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/AscendGameplayEffectTypes.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment.h"
#include "AscendAbilityFragment_EffectContainers.generated.h"

/**
 * Maps gameplay tags to gameplay effect containers, allowing the ability
 * to apply different effects based on context (e.g. hit, miss, charge level).
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Ability Fragment - Effect Containers"))
struct PROJECTASCEND_API FAscendAbilityFragment_EffectContainers : public FAscendAbilityFragment
{
	GENERATED_BODY()

public:
	FAscendAbilityFragment_EffectContainers()
	{
		FragmentTag = AscendGameplayTags::Fragment_EffectContainer;
	}
	
	/** Tag-to-container map. The ability logic selects which tag key to apply. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|Effects")
	TMap<FGameplayTag, FAscendGameplayEffectContainer> EffectContainerMap;
};
