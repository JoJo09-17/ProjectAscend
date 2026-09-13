#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AscendMeleeWindow.generated.h"

/** Opens authoritative blade collision and its matching visual trail. No mutable per-character state is stored here. */
UCLASS(meta=(DisplayName="Ascend Melee Hit Window"))
class PROJECTASCEND_API UAscendMeleeWindow : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, float Duration, const FAnimNotifyEventReference& Reference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference) override;
};
