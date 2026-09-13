#include "Combat/AscendMeleeEditorLibrary.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimMontage.h"
#include "Combat/AscendRangedNotify.h"
#include "Combat/AscendImpactFXNotify.h"
#include "NiagaraSystem.h"
#if WITH_EDITOR
#include "AnimationBlueprintLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Combat/AscendMeleeWindow.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_Slot.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#endif

bool UAscendMeleeEditorLibrary::EnsureMontageSlot(UAnimBlueprint* Blueprint,FName SlotName)
{
#if WITH_EDITOR
 if (!Blueprint || !Blueprint->TargetSkeleton) { return false; }
 TArray<UEdGraph*> Graphs;
 Blueprint->GetAllGraphs(Graphs);
 for (UEdGraph* Graph : Graphs)
 {
  if (Graph->GetFName()!=TEXT("AnimGraph")) { continue; }
  UAnimGraphNode_Root* Root = nullptr;
  for (UEdGraphNode* Node : Graph->Nodes)
  {
   if (auto* Slot=Cast<UAnimGraphNode_Slot>(Node); Slot && Slot->Node.SlotName==SlotName) { return true; }
   if (auto* Candidate=Cast<UAnimGraphNode_Root>(Node)) { Root=Candidate; }
  }
  if (!Root) { continue; }
  UEdGraphPin* Input = nullptr;
  for (UEdGraphPin* Pin : Root->Pins) { if (Pin->Direction==EGPD_Input && Pin->LinkedTo.Num()) { Input=Pin; break; } }
  if (!Input) { return false; }
  UEdGraphPin* Previous = Input->LinkedTo[0];
  Blueprint->Modify(); Graph->Modify(); Root->Modify();
  FGraphNodeCreator<UAnimGraphNode_Slot> Creator(*Graph);
  UAnimGraphNode_Slot* Slot=Creator.CreateNode();
  Slot->Node.SlotName=SlotName; Slot->NodePosX=Root->NodePosX-220; Slot->NodePosY=Root->NodePosY;
  Creator.Finalize();
  UEdGraphPin* SlotInput=nullptr; UEdGraphPin* SlotOutput=nullptr;
  for (UEdGraphPin* Pin : Slot->Pins) { if (Pin->Direction==EGPD_Input) { SlotInput=Pin; } else { SlotOutput=Pin; } }
  if (!SlotInput || !SlotOutput) { return false; }
  Input->BreakAllPinLinks();
  const UEdGraphSchema* Schema=Graph->GetSchema();
  if (!Schema->TryCreateConnection(Previous,SlotInput) || !Schema->TryCreateConnection(SlotOutput,Input)) { return false; }
  Blueprint->TargetSkeleton->RegisterSlotNode(SlotName);
  Blueprint->TargetSkeleton->MarkPackageDirty();
  FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
  FKismetEditorUtilities::CompileBlueprint(Blueprint);
  Blueprint->MarkPackageDirty();
  return true;
 }
 return false;
#else
 return false;
#endif
}
bool UAscendMeleeEditorLibrary::SetMontageSlot(UAnimMontage* Montage,FName SlotName)
{
 if (!Montage || SlotName.IsNone()) { return false; }
#if WITH_EDITOR
 Montage->Modify();
 for (auto& Track : Montage->SlotAnimTracks) { Track.SlotName=SlotName; }
 if (Montage->GetSkeleton()) { Montage->GetSkeleton()->RegisterSlotNode(SlotName); Montage->GetSkeleton()->MarkPackageDirty(); }
 Montage->PostEditChange(); Montage->MarkPackageDirty(); return true;
#else
 return false;
#endif
}
FName UAscendMeleeEditorLibrary::GetMontageSlot(const UAnimMontage* Montage)
{ return Montage && Montage->SlotAnimTracks.Num() ? Montage->SlotAnimTracks[0].SlotName : NAME_None; }
int32 UAscendMeleeEditorLibrary::GetMeleeWindowCount(const UAnimMontage* Montage)
{
 int32 Count=0;
#if WITH_EDITOR
 if (Montage) { for (const auto& Event : Montage->Notifies) { if (Event.NotifyStateClass && Event.NotifyStateClass->IsA<UAscendMeleeWindow>() && Event.GetDuration()>0) { ++Count; } } }
#endif
 return Count;
}
bool UAscendMeleeEditorLibrary::SetMeleeWindow(UAnimMontage* Montage,float StartTime,float Duration)
{
#if WITH_EDITOR
 if (!Montage || StartTime<0 || Duration<=0 || StartTime+Duration>Montage->GetPlayLength()) { return false; }
 Montage->Modify();
 Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& E){ return E.NotifyStateClass && E.NotifyStateClass->IsA<UAscendMeleeWindow>(); });
 TArray<FName> Tracks; UAnimationBlueprintLibrary::GetAnimationNotifyTrackNames(Montage,Tracks);
 if (!Tracks.Contains(TEXT("MeleeCombat"))) { UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Montage,TEXT("MeleeCombat")); }
 auto* State=UAnimationBlueprintLibrary::AddAnimationNotifyStateEvent(Montage,TEXT("MeleeCombat"),StartTime,Duration,UAscendMeleeWindow::StaticClass());
 Montage->PostEditChange(); Montage->MarkPackageDirty(); return State!=nullptr;
#else
 return false;
#endif
}
bool UAscendMeleeEditorLibrary::SetRangedTiming(UAnimMontage* Montage,float ReleaseTime,float ComboOpenTime,float ComboCloseTime,float BlendTime)
{
#if WITH_EDITOR
 if (!Montage || ReleaseTime<=0 || ComboOpenTime<=ReleaseTime || ComboCloseTime<=ComboOpenTime || ComboCloseTime>=Montage->GetPlayLength() || BlendTime<=0) { return false; }
 Montage->Modify();
 Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& E){ return E.Notify && E.Notify->IsA<UAscendRangedNotify>(); });
 TArray<FName> Tracks; UAnimationBlueprintLibrary::GetAnimationNotifyTrackNames(Montage,Tracks);
 if (!Tracks.Contains(TEXT("RangedCombat"))) { UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Montage,TEXT("RangedCombat")); }
 const float Times[]={ReleaseTime,ComboOpenTime,ComboCloseTime};
 for (int32 Index=0;Index<3;++Index)
 {
  auto* Notify=Cast<UAscendRangedNotify>(UAnimationBlueprintLibrary::AddAnimationNotifyEvent(Montage,TEXT("RangedCombat"),Times[Index],UAscendRangedNotify::StaticClass()));
  if (!Notify) { return false; } Notify->Event=static_cast<EAscendRangedEvent>(Index);
 }
 Montage->BlendIn.SetBlendTime(BlendTime); Montage->BlendOut.SetBlendTime(BlendTime);
 Montage->PostEditChange(); Montage->MarkPackageDirty(); return true;
#else
 return false;
#endif
}
FVector UAscendMeleeEditorLibrary::GetRangedTiming(const UAnimMontage* Montage)
{
 FVector Times(-1,-1,-1);
 if (Montage) { for (const auto& E : Montage->Notifies) { if (const auto* N=Cast<UAscendRangedNotify>(E.Notify)) { Times[static_cast<int32>(N->Event)]=E.GetTime(); } } }
 return Times;
}
bool UAscendMeleeEditorLibrary::SetRangedReleaseTime(UAnimMontage* Montage,float Time)
{
#if WITH_EDITOR
 if (!Montage || Time<=0 || Time>=Montage->GetPlayLength()) { return false; }
 Montage->Modify();
 Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& E){ return E.Notify && E.Notify->IsA<UAscendRangedNotify>(); });
 TArray<FName> Tracks; UAnimationBlueprintLibrary::GetAnimationNotifyTrackNames(Montage,Tracks);
 if (!Tracks.Contains(TEXT("RangedCombat"))) { UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Montage,TEXT("RangedCombat")); }
 auto* N=Cast<UAscendRangedNotify>(UAnimationBlueprintLibrary::AddAnimationNotifyEvent(Montage,TEXT("RangedCombat"),Time,UAscendRangedNotify::StaticClass()));
 if (!N) { return false; } N->Event=EAscendRangedEvent::Release;
 Montage->PostEditChange(); Montage->MarkPackageDirty(); return true;
#else
 return false;
#endif
}
bool UAscendMeleeEditorLibrary::SetImpactFXTiming(UAnimMontage* Montage,float VFXTime,float DecalTime)
{
#if WITH_EDITOR
 if (!Montage || VFXTime<=0 || DecalTime<=0 || FMath::Max(VFXTime,DecalTime)>=Montage->GetPlayLength()) { return false; }
 Montage->Modify();
 Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& E){ return E.Notify && E.Notify->IsA<UAscendImpactFXNotify>(); });
 TArray<FName> Tracks; UAnimationBlueprintLibrary::GetAnimationNotifyTrackNames(Montage,Tracks);
 if (!Tracks.Contains(TEXT("ImpactFX"))) { UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Montage,TEXT("ImpactFX")); }
 for (int32 Index=0;Index<2;++Index)
 {
  auto* N=Cast<UAscendImpactFXNotify>(UAnimationBlueprintLibrary::AddAnimationNotifyEvent(Montage,TEXT("ImpactFX"),Index==0?VFXTime:DecalTime,UAscendImpactFXNotify::StaticClass()));
  if (!N) { return false; } N->bSpawnVFX=Index==0; N->bSpawnDecal=Index==1;
 }
 Montage->PostEditChange(); Montage->MarkPackageDirty(); return true;
#else
 return false;
#endif
}
bool UAscendMeleeEditorLibrary::BakePelvisRootMotion(UAnimSequence* Sequence)
{
#if WITH_EDITOR
 if (!Sequence || !Sequence->GetSkeleton()) { return false; }
 const IAnimationDataModel* Model=Sequence->GetDataModel();
 TArray<FTransform> Roots,Pelvis;
 Model->GetBoneTrackTransforms(TEXT("root"),Roots); Model->GetBoneTrackTransforms(TEXT("pelvis"),Pelvis);
 if (Roots.IsEmpty() || Pelvis.Num()<2) { return false; }
 for (const FTransform& Root : Roots) { if (!Root.GetTranslation().Equals(Roots[0].GetTranslation(),.01f)) { return true; } }
 while (Roots.Num()<Pelvis.Num()) { Roots.Add(Roots[0]); }
 if (Roots.Num()!=Pelvis.Num()) { return false; }
 FVector Initial=Roots[0].TransformPosition(Pelvis[0].GetTranslation()); Initial.Z=0;
 TArray<FTransform> NewRoots=Roots;
 for (int32 I=0;I<Roots.Num();++I) { FVector Delta=Roots[I].TransformPosition(Pelvis[I].GetTranslation())-Initial; Delta.Z=0; NewRoots[I].AddToTranslation(Delta); }
 IAnimationDataController& Controller=Sequence->GetController();
 Controller.OpenBracket(FText::FromString(TEXT("Bake horizontal pelvis motion to root")),false);
 auto WriteTrack=[&](FName Name,const TArray<FTransform>& Keys)
 {
  TArray<FVector3f> Positions,Scales; TArray<FQuat4f> Rotations;
  for (const auto& Key : Keys) { Positions.Add(FVector3f(Key.GetTranslation())); Scales.Add(FVector3f(Key.GetScale3D())); Rotations.Add(FQuat4f(Key.GetRotation())); }
  return Controller.SetBoneTrackKeys(Name,Positions,Rotations,Scales,false);
 };
 TArray<FName> Names; Model->GetBoneTrackNames(Names);
 const FReferenceSkeleton& Skeleton=Sequence->GetSkeleton()->GetReferenceSkeleton();
 const int32 RootIndex=Skeleton.FindBoneIndex(TEXT("root")); bool Success=true;
 for (FName Name : Names)
 {
  const int32 Bone=Skeleton.FindBoneIndex(Name);
  if (Bone==INDEX_NONE || Skeleton.GetParentIndex(Bone)!=RootIndex) { continue; }
  TArray<FTransform> Keys; Model->GetBoneTrackTransforms(Name,Keys); if (Keys.IsEmpty()) { continue; }
  while (Keys.Num()<Roots.Num()) { Keys.Add(Keys[0]); }
  if (Keys.Num()!=Roots.Num()) { Success=false; break; }
  for (int32 I=0;I<Keys.Num();++I) { const FVector Original=Roots[I].TransformPosition(Keys[I].GetTranslation()); Keys[I].SetTranslation(NewRoots[I].InverseTransformPosition(Original-Initial)); }
  Success &= WriteTrack(Name,Keys);
 }
 Success &= WriteTrack(TEXT("root"),NewRoots); Controller.CloseBracket(false);
 Sequence->bEnableRootMotion=true; Sequence->bForceRootLock=true; Sequence->RootMotionRootLock=ERootMotionRootLock::AnimFirstFrame;
 Sequence->PostEditChange(); Sequence->MarkPackageDirty(); return Success;
#else
 return false;
#endif
}
bool UAscendMeleeEditorLibrary::EnsureNiagaraDirectionParameter(UNiagaraSystem* System,FName Name)
{
#if WITH_EDITOR
 if (!System || !Name.ToString().StartsWith(TEXT("User."))) { return false; }
 System->Modify(); FNiagaraVariable Variable(FNiagaraTypeDefinition::GetVec3Def(),Name);
 System->GetExposedParameters().AddParameter(Variable);
 System->GetExposedParameters().SetParameterValue(FVector3f(1,0,0),Variable);
 System->PostEditChange(); System->MarkPackageDirty(); return true;
#else
 return false;
#endif
}
TArray<FName> UAscendMeleeEditorLibrary::GetNiagaraUserParameterNames(UNiagaraSystem* System)
{
 TArray<FName> Names;
 if (System) { for (const auto& Variable : System->GetExposedParameters().ReadParameterVariables()) { Names.Add(Variable.GetName()); } }
 return Names;
}
