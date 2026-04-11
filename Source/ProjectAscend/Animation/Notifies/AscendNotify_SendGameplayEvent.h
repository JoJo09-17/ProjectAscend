#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "AscendNotify_SendGameplayEvent.generated.h"

/**
 * Sends a gameplay event to the owning ASC so montage-authored windows can trigger ability logic.
 */
UCLASS(editinlinenew, Const, hideCategories = Object, collapseCategories, meta = (DisplayName = "Ascend Gameplay Event"))
class PROJECTASCEND_API UAscendNotify_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAscendNotify_SendGameplayEvent();

	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override;
#endif

protected:
	/** Event tag forwarded to GAS. Abilities typically listen for this tag inside montage tasks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	FGameplayTag EventTag;

	/** Optional magnitude forwarded with the event payload. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event", meta = (ClampMin = "0.0"))
	float EventMagnitude = 1.0f;

	/** Adds the animation asset as the context source object so the receiving ability can inspect it if needed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	bool bAddAnimationAsSourceObject = true;

	/** Stores the animation asset in OptionalObject for notify-authored consumers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	bool bStoreAnimationInOptionalObject = true;

	/** Stores the skeletal mesh component in OptionalObject2 for notify-authored consumers. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	bool bStoreMeshInOptionalObject2 = true;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	FName CustomDescription;
#endif

private:
	void BuildEventPayload(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		class UAscendAbilitySystemComponent* AbilitySystemComponent,
		FGameplayEventData& OutEventData) const;
};
