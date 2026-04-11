#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "StructUtils/InstancedStruct.h"
#include "AscendGameplayEffectTypes.generated.h"

class AActor;
class UGameplayEffect;

/**
 * Defines a set of gameplay effects and their target resolution strategy.
 * Stored in definition fragments and looked up by tag at runtime.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendGameplayEffectContainer
{
	GENERATED_BODY()

public:
	/** Targeting strategy authored inline on the container. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Effects")
	TInstancedStruct<FAscendTargetType> TargetType;

	/** Gameplay effect classes to apply to resolved targets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Effects")
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;
};

/**
 * Runtime-instantiated spec version of FAscendGameplayEffectContainer.
 * Holds resolved target data and constructed effect specs, ready for application.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendGameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Effects")
	FGameplayAbilityTargetDataHandle TargetData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ascend|Effects")
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	bool HasValidEffects() const;
	bool HasValidTargets() const;

	void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
	void AddTargetData(const FGameplayAbilityTargetDataHandle& InTargetData);
};
