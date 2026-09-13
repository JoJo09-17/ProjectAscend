#include "Combat/AscendMeleeWindow.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAscendMeleeWindow::NotifyBegin(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, float Duration, const FAnimNotifyEventReference& Reference)
{
	if (Mesh && Mesh->GetOwner())
		if (auto* Combat = Mesh->GetOwner()->FindComponentByClass<UAscendMeleeCombatComponent>()) { Combat->BeginDamageWindow(this); }
}

void UAscendMeleeWindow::NotifyEnd(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference)
{
	if (Mesh && Mesh->GetOwner())
		if (auto* Combat = Mesh->GetOwner()->FindComponentByClass<UAscendMeleeCombatComponent>()) { Combat->EndDamageWindow(this); }
}
