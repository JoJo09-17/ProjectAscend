#include "Combat/AscendRangedNotify.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"

UAscendRangedNotify::UAscendRangedNotify() { bIsNativeBranchingPoint = true; }

void UAscendRangedNotify::BranchingPointNotify(FBranchingPointNotifyPayload& Payload)
{
	AAscendCharacterBase* Character = Payload.SkelMeshComponent ? Cast<AAscendCharacterBase>(Payload.SkelMeshComponent->GetOwner()) : nullptr;
	if (!Character || !Character->IsActiveRangedMontageInstance(Payload.MontageInstanceID)) { return; }
	if (Event == EAscendRangedEvent::Release) { Character->ReleaseRangedShot(Payload.SequenceAsset); }
	else { Character->SetRangedComboWindow(Payload.SequenceAsset, Event == EAscendRangedEvent::ComboOpen); }
}

void UAscendRangedNotify::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference)
{
	AAscendCharacterBase* Character = Mesh ? Cast<AAscendCharacterBase>(Mesh->GetOwner()) : nullptr;
	if (!Character) { return; }
	if (Event == EAscendRangedEvent::Release) { Character->ReleaseRangedShot(Animation); }
	else { Character->SetRangedComboWindow(Animation, Event == EAscendRangedEvent::ComboOpen); }
}
