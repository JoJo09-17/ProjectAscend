#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AscendRangedNotify.generated.h"

UENUM(BlueprintType)
enum class EAscendRangedEvent : uint8 { Release, ComboOpen, ComboClose };

/** Montage-authored shot timing and combo permission; state stays on the character. */
UCLASS(meta=(DisplayName="Ascend Ranged Combat Event"))
class PROJECTASCEND_API UAscendRangedNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
	UAscendRangedNotify();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat") EAscendRangedEvent Event = EAscendRangedEvent::Release;
	virtual void Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference) override;
	virtual void BranchingPointNotify(FBranchingPointNotifyPayload& Payload) override;
};
