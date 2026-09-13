#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Combat/AscendMeleeDamageEffect.h"
#include "Combat/AscendRangedDamageEffect.h"
#include "Combat/AscendRangedProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AscendGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Combat/AscendImpactFXProfile.h"
#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "UObject/UObjectIterator.h"
#include "NiagaraComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAscendEnemyHitReactionTest,"Ascend.Combat.Enemy.HitReaction",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAscendEnemyHitReactionTest::RunTest(const FString&)
{
 const FName Name = MakeUniqueObjectName(nullptr,UWorld::StaticClass(),TEXT("HitReactionWorld"),EUniqueObjectNameOptions::GloballyUnique);
 FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
 UWorld* World = UWorld::CreateWorld(EWorldType::Game,false,Name,GetTransientPackage());
 Context.SetCurrentWorld(World);
 World->InitializeActorsForPlay(FURL());
 USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
 UClass* AnimClass = LoadClass<UAnimInstance>(nullptr,TEXT("/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"));
 AActor* Source = World->SpawnActor<AActor>();
 AActor* Floor = World->SpawnActor<AActor>();
 UBoxComponent* FloorBox = NewObject<UBoxComponent>(Floor);
 Floor->SetRootComponent(FloorBox);
 FloorBox->SetBoxExtent(FVector(2000,2000,10));
 FloorBox->SetCollisionProfileName(TEXT("BlockAll"));
 FloorBox->RegisterComponent();
 Floor->SetActorLocation(FVector(0,0,390));
 auto CountDecals = [&]()
 {
  int32 Count = 0;
  for (TObjectIterator<UDecalComponent> It; It; ++It) { if (It->GetWorld()==World && It->IsRegistered()) { ++Count; } }
  return Count;
 };
 for (const TCHAR* Style : {TEXT("Saber"),TEXT("Archer")})
 {
  UAscendMeleeProfile* Authored = LoadObject<UAscendMeleeProfile>(nullptr,*FString::Printf(TEXT("/Game/Combat/Melee/Profiles/DA_%s_Manny.DA_%s_Manny"),Style,Style));
  TestNotNull(TEXT("Enemy profile is authored"),Authored);
  if (!Authored || !Mesh || !AnimClass) { AddError(TEXT("Hit reaction assets are missing")); continue; }
  AAscendEnemyCharacter* Enemy = World->SpawnActor<AAscendEnemyCharacter>(FVector(0,0,500),FRotator(0,90,0));
  Enemy->MeleeCombat->Profile = DuplicateObject<UAscendMeleeProfile>(Authored,Enemy);
  auto* Profile = Enemy->MeleeCombat->Profile.Get();
  Enemy->GetMesh()->SetSkeletalMesh(Mesh);
  Enemy->GetMesh()->SetAnimInstanceClass(AnimClass);
  UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent();
  ASC->AddAttributeSetSubobject(NewObject<UAscendAttributeSet>(Enemy));
  Enemy->DispatchBeginPlay();
  Enemy->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
  ASC->SetNumericAttributeBase(UAscendAttributeSet::GetMaxHealthAttribute(),100.f);
  ASC->SetNumericAttributeBase(UAscendAttributeSet::GetHealthAttribute(),100.f);
  const FVector Position = Enemy->GetActorLocation();
  TestEqual(TEXT("Front damage chooses front reaction"),Enemy->SelectHitReaction(Position+Enemy->GetActorForwardVector()*100),Profile->HitFrontMontage.Get());
  TestEqual(TEXT("Back damage chooses back reaction"),Enemy->SelectHitReaction(Position-Enemy->GetActorForwardVector()*100),Profile->HitBackMontage.Get());
  TestEqual(TEXT("Right damage chooses right reaction"),Enemy->SelectHitReaction(Position+Enemy->GetActorRightVector()*100),Profile->HitRightMontage.Get());
  TestEqual(TEXT("Left damage chooses left reaction"),Enemy->SelectHitReaction(Position-Enemy->GetActorRightVector()*100),Profile->HitLeftMontage.Get());
  auto Damage = [&](UClass* Effect,float Amount,AActor* Causer = nullptr)
  {
   FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
   EffectContext.AddInstigator(Source,Causer ? Causer : Source);
   auto Spec = ASC->MakeOutgoingSpec(Effect,1.f,EffectContext);
   if (!Spec.IsValid()) { AddError(TEXT("Damage spec failed")); return; }
   Spec.Data->SetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_Damage,Amount);
   ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
  };
  // A root component is required for an actor's position to be meaningful.
  Source->SetRootComponent(NewObject<USceneComponent>(Source));
  Source->GetRootComponent()->RegisterComponent();
  Source->SetActorLocation(Position+Enemy->GetActorForwardVector()*100);
  Damage(UAscendMeleeDamageEffect::StaticClass(),0.f);
  TestFalse(TEXT("Zero damage does not trigger hit reaction"),Enemy->IsHitReacting());
  if (FCString::Strcmp(Style,TEXT("Saber"))==0)
  {
   TestTrue(TEXT("Enemy begins its melee attack"),Enemy->MeleeCombat->RequestAttack());
  }
  if (FCString::Strcmp(Style,TEXT("Archer"))==0)
  {
   TestTrue(TEXT("Enemy begins its ranged attack"),Enemy->PlayRangedAttackAnimation());
  }
  Damage(UAscendMeleeDamageEffect::StaticClass(),5.f);
  TestTrue(TEXT("Melee damage triggers enemy hit reaction"),Enemy->IsHitReacting());
  TestFalse(TEXT("Hit reaction cancels pending ranged attack"),Enemy->IsRangedAttacking());
  TestFalse(TEXT("Hit reaction cancels melee attack and hit window"),Enemy->MeleeCombat->IsAttacking() || Enemy->MeleeCombat->IsDamageWindowOpen());
  TestTrue(TEXT("Hit reaction blocks movement input"),Enemy->MeleeCombat->IsAttackMovementLocked());
  TestFalse(TEXT("Enemy cannot start ranged attack while hit reacting"),Enemy->PlayRangedAttackAnimation());
  TestFalse(TEXT("Enemy cannot start melee attack while hit reacting"),Enemy->MeleeCombat->RequestAttack());
  UAnimInstance* Anim = Enemy->GetMesh()->GetAnimInstance();
  FAnimMontageInstance* First = Anim->GetActiveInstanceForMontage(Enemy->GetActiveHitMontage());
  const int32 FirstID = First ? First->GetInstanceID() : INDEX_NONE;
  TestNotNull(TEXT("Enemy has an impact FX preset"),Profile->HitFXProfile.Get());
  TestFalse(TEXT("FX waits for its notify"),Enemy->bHitVFXEmitted);
  TestFalse(TEXT("Decal waits for its own notify"),Enemy->bHitDecalEmitted);
  if (First)
  {
   First->UpdateWeight(.065f);
   First->Advance(.065f,nullptr,false);
   TestTrue(TEXT("VFX notify fires first without spawning decal"),Enemy->bHitVFXEmitted && !Enemy->bHitDecalEmitted);
   const int32 BeforeDecals = CountDecals();
   First->UpdateWeight(.06f);
   First->Advance(.06f,nullptr,false);
   TestTrue(TEXT("Later decal notify fires separately"),Enemy->bHitDecalEmitted);
   TestEqual(TEXT("Ground decal notify creates one decal on the floor"),CountDecals(),BeforeDecals+1);
   Enemy->EmitHitEffects(Enemy->GetActiveHitMontage(),FirstID,nullptr,true,true);
   TestEqual(TEXT("Duplicate notify cannot spawn a second decal"),CountDecals(),BeforeDecals+1);
  }
  Damage(UAscendRangedDamageEffect::StaticClass(),5.f);
  TestEqual(TEXT("Rapid successive hits still apply damage"),ASC->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute()),90.f);
  TestEqual(TEXT("Reaction debounce preserves the active animation instance"),Anim->GetActiveInstanceForMontage(Enemy->GetActiveHitMontage())->GetInstanceID(),FirstID);
  Profile->HitReactionMinInterval = 0.f;
  // Advance only the test world's clock, avoiding AI simulation.
  World->Tick(LEVELTICK_TimeOnly,.2f);
  Source->SetActorLocation(Position+Enemy->GetActorRightVector()*100);
  Damage(UAscendRangedDamageEffect::StaticClass(),5.f);
  TestEqual(TEXT("Ranged damage selects the new reaction direction"),Enemy->GetActiveHitMontage(),Profile->HitRightMontage.Get());
  Enemy->EmitHitEffects(Enemy->GetActiveHitMontage(),FirstID,nullptr,true,true);
  TestFalse(TEXT("Stale montage instance cannot emit new hit FX"),Enemy->bHitVFXEmitted || Enemy->bHitDecalEmitted);
  AAscendRangedProjectile* Arrow = World->SpawnActor<AAscendRangedProjectile>(Position+Enemy->GetActorRightVector()*1000,FRotator::ZeroRotator);
  Arrow->InitializeProjectile(Enemy->GetActorForwardVector(),5.f,2600.f,5.f,true);
  Damage(UAscendRangedDamageEffect::StaticClass(),5.f,Arrow);
  TestEqual(TEXT("Arrow reaction uses travel direction instead of deferred overlap position"),Enemy->GetActiveHitMontage(),Profile->HitBackMontage.Get());
  TestTrue(TEXT("Impact snapshots the projectile travel vector for delayed Niagara bursts"),Enemy->HitProjectileDirection.Equals(Enemy->GetActorForwardVector(),.001f));
  Arrow->Destroy();
  // The source projectile is gone when the delayed burst starts.
  if (FAnimMontageInstance* BurstInstance=Anim->GetActiveInstanceForMontage(Enemy->GetActiveHitMontage()))
  {
   BurstInstance->UpdateWeight(.065f); BurstInstance->Advance(.065f,nullptr,false);
   bool FoundDirection=false;
   for (TObjectIterator<UNiagaraComponent> It; It; ++It)
   {
    if (It->GetWorld()!=World || It->GetAsset()!=Profile->HitFXProfile->NiagaraSystem) { continue; }
    bool Valid=false;
    const FVector Value=It->GetVariableVec3(Profile->HitFXProfile->NiagaraDirectionParameter,Valid);
    if (Valid && Value.Equals(Enemy->GetActorForwardVector(),.001f)) { FoundDirection=true; }
   }
   TestTrue(TEXT("Delayed burst receives the destroyed projectile's world direction"),FoundDirection);
  }
  Anim->Montage_Stop(0.f,Enemy->GetActiveHitMontage());
  Enemy->GetMesh()->TickAnimation(.01f,false);
  Enemy->GetMesh()->RefreshBoneTransforms();
  TestFalse(TEXT("Reaction completion releases its state"),Enemy->IsHitReacting());
  TestFalse(TEXT("Reaction completion releases movement lock"),Enemy->MeleeCombat->IsAttackMovementLocked());
  Damage(UAscendRangedDamageEffect::StaticClass(),5.f);
  TestTrue(TEXT("Subsequent damage can start another reaction"),Enemy->IsHitReacting());
  Damage(UAscendMeleeDamageEffect::StaticClass(),200.f);
  TestTrue(TEXT("Lethal damage enters death"),Enemy->IsDead());
  TestFalse(TEXT("Death cancels hit reaction"),Enemy->IsHitReacting());
  const int32 DecalsBeforeDeathNotify = CountDecals();
  Enemy->EmitHitEffects(Profile->HitBackMontage,INDEX_NONE,nullptr,true,true);
  TestEqual(TEXT("Cancelled or dead reaction cannot emit a late decal"),CountDecals(),DecalsBeforeDeathNotify);
  TestFalse(TEXT("Death clears reaction movement lock"),Enemy->MeleeCombat->IsAttackMovementLocked());
  Enemy->Destroy();
 }
 World->DestroyWorld(false);
 GEngine->DestroyWorldContext(World);
 return true;
}
#endif
