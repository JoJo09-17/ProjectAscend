#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AscendTimeDilation.generated.h"

/**
 * Applies temporary time dilation while the notify state is active and restores the previous value on exit.
 */
UCLASS(DisplayName = "Ascend Time Dilation")
class PROJECTASCEND_API UAscendTimeDilation : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void NotifyBegin(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", meta = (ClampMin = "0.01"))
	float TimeDilation = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bAffectGlobalTimeDilation = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bRestoreOnEnd = true;

private:
	void ApplyActorTimeDilation(AActor* OwnerActor);
	void RestoreActorTimeDilation(AActor* OwnerActor);
	void ApplyGlobalTimeDilation(UWorld* World);
	void RestoreGlobalTimeDilation(UWorld* World);

	TMap<TWeakObjectPtr<AActor>, float> PreviousActorTimeDilations;
	TMap<TWeakObjectPtr<UWorld>, float> PreviousGlobalTimeDilations;
};
