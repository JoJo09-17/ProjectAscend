#include "Combat/AscendImpactFXNotify.h"
#include "Combat/AscendImpactFXProfile.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"

UAscendImpactFXNotify::UAscendImpactFXNotify() { bIsNativeBranchingPoint = true; }
FString UAscendImpactFXNotify::GetNotifyName_Implementation() const
{
 return bSpawnVFX && bSpawnDecal ? TEXT("Impact VFX + Decal") : (bSpawnVFX ? TEXT("Impact VFX") : TEXT("Impact Decal"));
}
void UAscendImpactFXNotify::BranchingPointNotify(FBranchingPointNotifyPayload& Payload)
{
 if (!Payload.SkelMeshComponent) { return; }
 if (bUseHitReactionContext)
 {
  if (auto* Enemy = Cast<AAscendEnemyCharacter>(Payload.SkelMeshComponent->GetOwner())) { Enemy->EmitHitEffects(Payload.SequenceAsset,Payload.MontageInstanceID,Profile,bSpawnVFX,bSpawnDecal); }
  return;
 }
 Notify(Payload.SkelMeshComponent,Payload.SequenceAsset,FAnimNotifyEventReference());
}
void UAscendImpactFXNotify::Notify(USkeletalMeshComponent* Mesh,UAnimSequenceBase* Animation,const FAnimNotifyEventReference&)
{
 if (!Mesh || !Mesh->GetOwner()) { return; }
 if (bUseHitReactionContext)
 {
  if (auto* Enemy = Cast<AAscendEnemyCharacter>(Mesh->GetOwner())) { Enemy->EmitHitEffects(Animation,INDEX_NONE,Profile,bSpawnVFX,bSpawnDecal); }
  return;
 }
 if (Profile) { Profile->SpawnEffects(Mesh->GetOwner(),Mesh->GetComponentLocation(),Mesh->GetForwardVector(),bSpawnVFX,bSpawnDecal,Mesh->GetForwardVector()); }
}
