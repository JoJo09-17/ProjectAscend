#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AscendAttributeTypes.generated.h"

class AActor;
class APawn;
class APlayerController;
class UAbilitySystemComponent;

/**
 * Lightweight payload describing the source, target, and tag state involved in an attribute modification.
 */
USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendAttributeSetExecutionData
{
	GENERATED_BODY()

	/** Effect context that produced the current attribute modification. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayEffectContextHandle Context;

	/** Tags captured from the source when the effect spec was created. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayTagContainer SourceTags;

	/** Tags captured from the target when the effect spec was created. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayTagContainer TargetTags;

	/** Asset tags authored on the effect spec being executed. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayTagContainer SpecAssetTags;

	/** Magnitude being applied by the evaluated modifier. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	float DeltaValue = 0.0f;

	/** Ability system component that originated the modification. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	/** Ability system component receiving the modification. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<UAbilitySystemComponent> TargetASC = nullptr;

	/** Actor that should be treated as the logical source. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<AActor> SourceActor = nullptr;

	/** Actor receiving the attribute modification. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** Explicit effect causer from the context, if any. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<AActor> EffectCauser = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<APlayerController> SourceController = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<APlayerController> TargetController = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<APawn> SourcePawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<APawn> TargetPawn = nullptr;

	/** Source object passed by the granting ability or gameplay effect. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	TObjectPtr<UObject> SourceObject = nullptr;

	/** @return True when at least one valid actor context was resolved. */
	bool HasResolvedActors() const
	{
		return SourceActor != nullptr || TargetActor != nullptr;
	}
};
