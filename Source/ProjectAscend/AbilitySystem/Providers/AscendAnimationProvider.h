#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "AscendAnimationProvider.generated.h"

class UAnimMontage;
class UAscendGameplayAbility;

/**
 * Strategy object for resolving animation playback data for abilities.
 * Implements a priority chain: source object interface -> ability interface -> provider defaults.
 * Configured per-ability via the AnimationProvider fragment on the definition.
 */
UCLASS(Const, Blueprintable, BlueprintType, EditInlineNew, DefaultToInstanced, meta = (DisplayName = "Ascend Animation Provider"))
class PROJECTASCEND_API UAscendAnimationProvider : public UObject
{
	GENERATED_BODY()

public:
	UAscendAnimationProvider();

	/** Returns the montage to play for the given ability context. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ascend|Animation|Provider")
	UAnimMontage* GetMontageToPlay(UAscendGameplayAbility* AscendGameplayAbility) const;

	/** Returns the starting montage section for the given ability context. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Animation|Provider")
	FName GetSectionName(UAscendGameplayAbility* AscendGameplayAbility) const;

	/** Returns the play rate multiplier for the given ability context. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Animation|Provider")
	float GetPlayRate(UAscendGameplayAbility* AscendGameplayAbility) const;

	/** Returns the root motion translation scale for the given ability context. */
	UFUNCTION(BlueprintNativeEvent, Category = "Ascend|Animation|Provider")
	float GetRootMotionScale(UAscendGameplayAbility* AscendGameplayAbility) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Provider")
	TObjectPtr<UAnimMontage> DefaultAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Provider")
	FName DefaultSectionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Provider")
	float DefaultPlayRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Provider")
	float RootMotionScale;

	/** Queries the ability's source object for its animation montage (via IAnimationMontageProviderInterface). */
	UFUNCTION(BlueprintPure, Category = "Ascend|Animation|Provider")
	static UAnimMontage* GetAbilityAnimationMontage(const UAscendGameplayAbility* AscendGameplayAbility);

	/** Queries the ability's source object for its section name (via IAnimationMontageProviderInterface). */
	UFUNCTION(BlueprintPure, Category = "Ascend|Animation|Provider")
	static FName GetAbilitySectionName(const UAscendGameplayAbility* AscendGameplayAbility);

	/** Queries the ability's source object for its play rate (via IAnimationMontageProviderInterface). */
	UFUNCTION(BlueprintPure, Category = "Ascend|Animation|Provider")
	static float GetAbilityPlayRate(const UAscendGameplayAbility* AscendGameplayAbility);

	/** Collects ability tags and animation context tags for provider interface queries. */
	UFUNCTION(BlueprintPure, Category = "Ascend|Animation|Provider")
	static FGameplayTagContainer GetAbilityTags(const UAscendGameplayAbility* AscendGameplayAbility);

	static UAnimMontage* GetAnimationMontageFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags);
	static FName GetSectionNameFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags);
	static float GetPlayRateFromObject(const UObject* CandidateObject, const FGameplayTagContainer& AbilityTags);
	static void AppendAnimationContextTagsFromObject(const UObject* CandidateObject, FGameplayTagContainer& InOutTags);
};
