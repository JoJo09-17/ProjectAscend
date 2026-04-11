#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Effects/AscendGameplayEffectTypes.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment.h"
#include "AscendAbilityFragment_UIData.generated.h"


USTRUCT(BlueprintType, meta = (DisplayName = "Ability Fragment - UI Data"))
struct PROJECTASCEND_API FAscendAbilityFragment_UIData : public FAscendAbilityFragment
{
	GENERATED_BODY()

public:
	FAscendAbilityFragment_UIData()
	{
		FragmentTag = AscendGameplayTags::Fragment_UIData;
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|UI")
	TSoftObjectPtr<UTexture2D> AbilityIcon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|UI")
	FText AbilityName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ascend|UI")
	FText AbilityDescription;
	
};
