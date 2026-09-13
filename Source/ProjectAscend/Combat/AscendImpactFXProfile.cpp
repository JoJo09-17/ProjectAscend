#include "Combat/AscendImpactFXProfile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UAscendImpactFXProfile::SpawnEffects(AActor* Target,FVector ImpactPoint,FVector ImpactNormal,bool bSpawnVFX,bool bSpawnDecal,FVector ProjectileDirection) const
{
 if (!IsValid(Target) || !Target->GetWorld() || Target->GetNetMode()==NM_DedicatedServer) { return; }
 UWorld* World = Target->GetWorld();
 USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>();
 const FVector Normal = ImpactNormal.GetSafeNormal(SMALL_NUMBER,FVector::UpVector);
 FVector Position = ImpactPoint;
 FRotator Rotation = Normal.Rotation();
 if (bUseMeshSocket && Mesh && Mesh->DoesSocketExist(SocketName))
 {
  const FTransform Socket = Mesh->GetSocketTransform(SocketName);
  Position = Socket.GetLocation();
  Rotation = Socket.Rotator();
 }
 Position += Rotation.RotateVector(VFXOffset);
 auto ConfigureVFX = [&](USceneComponent* Component)
 {
  if (!Component) { return; }
  if (bAttachVFX && Mesh) { Component->AttachToComponent(Mesh,FAttachmentTransformRules::KeepWorldTransform,bUseMeshSocket ? SocketName : NAME_None); }
  TWeakObjectPtr<USceneComponent> Weak = Component;
  FTimerHandle Cleanup;
  World->GetTimerManager().SetTimer(Cleanup,[Weak]() { if (Weak.IsValid()) { Weak->DestroyComponent(); } },FMath::Max(VFXLifetime,.1f),false);
 };
 if (bSpawnVFX)
 {
  if (NiagaraSystem)
  {
   // Burst emitters must receive user inputs BEFORE their initial activation.
   if (UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target,NiagaraSystem,Position,Rotation,VFXScale,true,false))
   {
    ConfigureVFX(Niagara);
    FVector Direction = ProjectileDirection.GetSafeNormal(SMALL_NUMBER,-Normal);
    if (bNiagaraDirectionInLocalSpace) { Direction = Niagara->GetComponentTransform().InverseTransformVectorNoScale(Direction).GetSafeNormal(); }
    if (!NiagaraDirectionParameter.IsNone()) { Niagara->SetVariableVec3(NiagaraDirectionParameter,Direction); }
    Niagara->Activate(true);
   }
  }
  if (ParticleSystem) { ConfigureVFX(UGameplayStatics::SpawnEmitterAtLocation(World,ParticleSystem,Position,Rotation,VFXScale,true)); }
 }
 if (!bSpawnDecal || !DecalMaterial || DecalMaterial->GetMaterial()->MaterialDomain != MD_DeferredDecal) { return; }
 FVector DecalPoint = ImpactPoint;
 FVector DecalNormal = Normal;
 if (DecalPlacement == EAscendDecalPlacement::Ground)
 {
  FHitResult Ground;
  FCollisionQueryParams Params(SCENE_QUERY_STAT(AscendImpactDecal),false,Target);
  const FVector Start = ImpactPoint + FVector(0,0,50);
  if (!World->LineTraceSingleByChannel(Ground,Start,Start-FVector(0,0,FMath::Max(GroundTraceDistance,1.f)),ECC_Visibility,Params) || Ground.ImpactNormal.Z < .25f) { return; }
  DecalPoint = Ground.ImpactPoint;
  DecalNormal = Ground.ImpactNormal;
 }
 if (UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(Target,DecalMaterial,DecalSize,DecalPoint+DecalNormal*.5f,(-DecalNormal).Rotation(),0.f))
 {
  Decal->SetFadeOut(FMath::Max(DecalFadeDelay,0.f),FMath::Max(DecalFadeDuration,.01f),false);
 }
}
