#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Combat/AscendMeleeCombatComponent.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"
#include "Combat/AscendMeleeAbility.h"
#include "Player/AscendPlayerController.h"
#include "Combat/AscendRangedProjectile.h"
#include "Combat/AscendProjectileProfile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "EngineUtils.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Combat/AscendMeleeEditorLibrary.h"
#include "Combat/AscendRangedNotify.h"
#include "Character/Player/AscendCharacterMovementComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAscendMeleeSweepTest, "Ascend.Combat.Melee.ContinuousSweep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAscendMeleeSweepTest::RunTest(const FString& Parameters)
{
	const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("MeleeSweepWorld"), EUniqueObjectNameOptions::GloballyUnique);
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AAscendCharacterBase* Source = World->SpawnActor<AAscendCharacterBase>(FVector(-500,0,0), FRotator::ZeroRotator);
	AAscendEnemyCharacter* Target = World->SpawnActor<AAscendEnemyCharacter>(FVector(0,0,0), FRotator::ZeroRotator);
	Target->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Target->GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	Target->GetCapsuleComponent()->SetCapsuleSize(8,20);
	TArray<FHitResult> Hits;
	const FVector Extent(45,3,3);
	UAscendMeleeCombatComponent::SweepBlade(World, FTransform(FQuat::Identity,FVector(0,-100,0)), FTransform(FQuat::Identity,FVector(0,100,0)), Extent, Source, Hits);
	TestTrue(TEXT("A fast translated blade cannot tunnel through a narrow target"), Hits.ContainsByPredicate([Target](const FHitResult& H) { return H.GetActor()==Target; }));
	Target->SetActorLocation(FVector(60,60,0));
	Hits.Reset();
	UAscendMeleeCombatComponent::SweepBlade(World, FTransform(FQuat::Identity,FVector::ZeroVector), FTransform(FRotator(0,90,0),FVector::ZeroVector), FVector(100,3,3), Source, Hits);
	TestTrue(TEXT("Rotation is swept, not just blade center translation"), Hits.ContainsByPredicate([Target](const FHitResult& H) { return H.GetActor()==Target; }));
	Hits.Reset();
	UAscendMeleeCombatComponent::SweepBlade(World, FTransform(FQuat::Identity,FVector(0,500,0)), FTransform(FQuat::Identity,FVector(100,500,0)), Extent, Source, Hits);
	TestTrue(TEXT("An out-of-range target is not hit"), Hits.IsEmpty());
	TestTrue(TEXT("Player and enemy are hostile"), UAscendMeleeCombatComponent::AreHostile(Source,Target));
	TestFalse(TEXT("Self hits are rejected"), UAscendMeleeCombatComponent::AreHostile(Target,Target));
	AAscendEnemyCharacter* Ally = World->SpawnActor<AAscendEnemyCharacter>(FVector(500,0,0), FRotator::ZeroRotator);
	TestFalse(TEXT("Enemies do not damage their allies"), UAscendMeleeCombatComponent::AreHostile(Ally,Target));
	for (AAscendCharacterBase* Actor : {Source, static_cast<AAscendCharacterBase*>(Target)})
	{
		UAscendMeleeCombatComponent* Combat = Actor->FindComponentByClass<UAscendMeleeCombatComponent>();
		Combat->Character = Actor;
		UCharacterMovementComponent* Movement = Actor->GetCharacterMovement();
		Movement->SetMovementMode(MOVE_Walking);
		Movement->Velocity = FVector(300,0,0);
		Actor->AddMovementInput(FVector::ForwardVector, 1.f, true);
		Combat->LockAttackMovement();
		Combat->LockAttackMovement();
		TestTrue(TEXT("Shared melee lock disables movement for player and enemy"), Combat->IsAttackMovementLocked() && Movement->MovementMode == MOVE_Walking);
		TestTrue(TEXT("Attack lock clears existing velocity"), Movement->Velocity.IsNearlyZero());
		Actor->AddMovementInput(FVector::ForwardVector, 1.f, true);
		Combat->CancelAttack();
		TestTrue(TEXT("Cancellation restores the original movement mode even after repeated lock calls"), Movement->MovementMode == MOVE_Walking);
		TestTrue(TEXT("Recovery clears buffered movement input"), Actor->GetPendingMovementInputVector().IsNearlyZero());
		for (bool bInterrupted : {false, true})
		{
			UAnimMontage* RangedMontage = NewObject<UAnimMontage>(Actor);
			Actor->ActiveRangedMontage = RangedMontage;
			Combat->LockAttackMovement();
			Actor->AddMovementInput(FVector::ForwardVector, 1.f, true);
			TestTrue(TEXT("Ranged montage keeps player and enemy movement disabled"), Combat->IsAttackMovementLocked() && Movement->MovementMode == MOVE_Walking);
			Actor->OnRangedAttackEnded(RangedMontage, bInterrupted);
			TestFalse(TEXT("Ranged completion or interruption clears attack state"), Actor->IsRangedAttacking());
			TestTrue(TEXT("Ranged completion or interruption restores walking"), Movement->MovementMode == MOVE_Walking);
			TestTrue(TEXT("Ranged recovery discards buffered movement"), Actor->GetPendingMovementInputVector().IsNearlyZero());
		}
		Combat->Profile = NewObject<UAscendMeleeProfile>(Actor);
		Combat->Profile->AttackSpeedMultiplier = 2.f;
		TestEqual(TEXT("Combat profile configures double attack speed"), Actor->GetCombatAttackSpeed(), 2.f);
		Combat->Profile->AttackSpeedMultiplier = 0.f;
		TestEqual(TEXT("Invalid profile speed cannot stop the montage indefinitely"), Actor->GetCombatAttackSpeed(), 0.1f);
		UAnimMontage* RangedMontage = NewObject<UAnimMontage>(Actor);
		Actor->ActiveRangedMontage = RangedMontage;
		Actor->bRangedHeavyAttack = true;
		Combat->LockAttackMovement();
		TestFalse(TEXT("Dodge refuses to cancel ranged heavy attack"), Actor->TryInterruptNormalAttack());
		TestTrue(TEXT("Rejected heavy cancellation keeps the movement lock"), Combat->IsAttackMovementLocked() && Movement->MovementMode == MOVE_Walking);
		Actor->bRangedHeavyAttack = false;
		TestTrue(TEXT("Dodge can cancel ranged normal attack"), Actor->TryInterruptNormalAttack());
		TestFalse(TEXT("Dodge removes ranged attack state"), Actor->IsRangedAttacking());
		TestTrue(TEXT("Dodge releases ranged lock before applying movement"), Movement->MovementMode == MOVE_Walking);
		UAscendMeleeAbility* MeleeAbility = NewObject<UAscendMeleeAbility>(Actor);
		Combat->ActiveAbility = MeleeAbility;
		Combat->bHeavyRequested = true;
		Combat->LockAttackMovement();
		TestFalse(TEXT("Dodge refuses to cancel melee heavy attack"), Actor->TryInterruptNormalAttack());
		Combat->bHeavyRequested = false;
		Combat->bQueuedLight = true;
		TestTrue(TEXT("Dodge can cancel melee normal attack"), Actor->TryInterruptNormalAttack());
		TestFalse(TEXT("Dodge removes melee attack state"), Combat->IsAttacking());
		TestFalse(TEXT("Dodge clears buffered melee follow-up attack"), Combat->bQueuedLight);
		TestTrue(TEXT("Dodge releases melee lock before applying movement"), Movement->MovementMode == MOVE_Walking);
	}
	AAscendPlayerController* Controller = World->SpawnActor<AAscendPlayerController>();
	Controller->Possess(Source);
	Controller->bUsingGamepad = true;
	Controller->LockedTarget = Target;
	Source->MeleeCombat->Profile->CombatStyle = EAscendCombatProfileStyle::Archer;
	Target->SetActorLocation(Source->GetActorLocation() + FVector(0,200,100));
	Source->SetActorRotation(FRotator(0,180,0));
	Controller->FireLightAttack();
	TestTrue(TEXT("Ranged gamepad attack turns the character toward the locked target on the horizontal plane"), Source->GetActorForwardVector().Equals(FVector::RightVector, 0.001f));
	bool bFoundProjectile = false;
	for (TActorIterator<AAscendRangedProjectile> It(World); It; ++It)
	{
		if (It->GetOwner() != Source) { continue; }
		bFoundProjectile = true;
		const auto* ProjectileMovement = It->FindComponentByClass<UProjectileMovementComponent>();
		TestTrue(TEXT("Ranged projectile travels along the character's attack facing"), ProjectileMovement && ProjectileMovement->Velocity.GetSafeNormal().Equals(Source->GetActorForwardVector(), 0.001f));
	}
	TestTrue(TEXT("Ranged attack emits a projectile"), bFoundProjectile);
	UAnimMontage* LockedMontage = NewObject<UAnimMontage>(Source);
	Source->ActiveRangedMontage = LockedMontage;
	Target->SetActorLocation(Source->GetActorLocation() + FVector(200,0,0));
	Controller->PrepareAttackFacing();
	TestTrue(TEXT("Further attacks cannot turn a character while a ranged montage is active"), Source->GetActorForwardVector().Equals(FVector::RightVector, 0.001f));
	Source->TryInterruptNormalAttack();
	Controller->FireHeavyAttack();
	TestTrue(TEXT("Ranged heavy attack updates facing for its new attack direction"), Source->GetActorForwardVector().Equals(FVector::ForwardVector, 0.001f));
	Source->ActiveRangedMontage = LockedMontage;
	Source->bRangedHeavyAttack = false;
	Source->MeleeCombat->LockAttackMovement();
	Controller->GamepadMoveValue = FVector2D(-1,-1);
	const FVector ExpectedDodgeDirection = Controller->GamepadDirectionToWorld(Controller->GamepadMoveValue).GetSafeNormal2D();
	Controller->PerformGamepadDodge();
	TestFalse(TEXT("Movement-directed dodge still cancels ranged normal attack"), Source->IsRangedAttacking());
	const TSharedPtr<FRootMotionSource> DodgeSource = Source->GetCharacterMovement()->GetRootMotionSource(TEXT("GamepadDodge"));
	TestTrue(TEXT("Dodge applies a movement source after releasing the attack lock"), DodgeSource.IsValid());
	if (DodgeSource.IsValid())
	{
		const auto Dodge = StaticCastSharedPtr<FRootMotionSource_ConstantForce>(DodgeSource);
		TestTrue(TEXT("Dodge follows movement input instead of the character's enemy-facing direction"), Dodge->Force.GetSafeNormal().Equals(ExpectedDodgeDirection, 0.001f));
		TestEqual(TEXT("Diagonal input keeps dodge speed constant"), Dodge->Force.Size(), static_cast<double>(Controller->DodgeSpeed));
	}
	Controller->GamepadMoveValue = FVector2D::ZeroVector;
	TestTrue(TEXT("Released stick uses the last movement direction rather than attack facing"), Controller->ResolveDodgeDirection().Equals(ExpectedDodgeDirection, 0.001f));
	Controller->bUsingGamepad = false;
	Source->AddMovementInput(FVector::RightVector, 1.f, true);
	TestTrue(TEXT("Pending movement takes priority over cached dodge direction"), Controller->ResolveDodgeDirection().Equals(FVector::RightVector, 0.001f));
	Source->ConsumeMovementInputVector();
	// Exercise the authored four-part combo with a real animation instance.
	UAscendMeleeProfile* ArcherProfile = LoadObject<UAscendMeleeProfile>(nullptr, TEXT("/Game/Combat/Melee/Profiles/DA_Archer_Manny.DA_Archer_Manny"));
	USkeletalMesh* MannyMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	UClass* MannyAnimClass = LoadClass<UAnimInstance>(nullptr, TEXT("/Game/Characters/Mannequins/Animations/ABP_Manny.ABP_Manny_C"));
	TestTrue(TEXT("Authored Archer profile contains all four normal attack montages"), ArcherProfile && ArcherProfile->RangedLightMontages.Num() == 4);
	if (ArcherProfile && ArcherProfile->RangedLightMontages.Num() == 4 && MannyMesh && MannyAnimClass)
	{
		Source->MeleeCombat->Profile = ArcherProfile;
		Source->GetMesh()->SetSkeletalMesh(MannyMesh);
		Source->GetMesh()->SetAnimInstanceClass(MannyAnimClass);
		UAnimInstance* Anim = Source->GetMesh()->GetAnimInstance();
		TestNotNull(TEXT("Manny animation instance initializes for combo playback"), Anim);
		Controller->bUsingGamepad = true;
		auto CountShots = [&]()
		{
			int32 Count = 0;
			for (TActorIterator<AAscendRangedProjectile> It(World); It; ++It) { if (It->GetOwner() == Source) { ++Count; } }
			return Count;
		};
		int32 ExpectedShots = CountShots();
		Controller->FireLightAttack();
		for (int32 Index = 0; Anim && Index < 4; ++Index)
		{
			UAnimMontage* ExpectedMontage = ArcherProfile->RangedLightMontages[Index];
			TestEqual(FString::Printf(TEXT("Normal combo stage %d plays its distinct authored montage"), Index + 1), Source->ActiveRangedMontage.Get(), ExpectedMontage);
			TestTrue(TEXT("Each combo stage keeps movement disabled"), Source->MeleeCombat->IsAttackMovementLocked() && Source->GetCharacterMovement()->MovementMode == MOVE_Walking);
			TestEqual(TEXT("Pressing attack does not emit before the release notify"), CountShots(), ExpectedShots);
			Controller->FireLightAttack();
			TestTrue(TEXT("A click during a normal attack buffers one follow-up"), Controller->bQueuedRangedLight);
			TestEqual(TEXT("Buffered input does not restart the active animation"), Source->ActiveRangedMontage.Get(), ExpectedMontage);
			const FVector Timing = UAscendMeleeEditorLibrary::GetRangedTiming(ExpectedMontage);
			FAnimMontageInstance* Instance = Anim->GetActiveInstanceForMontage(ExpectedMontage);
			TestNotNull(TEXT("Each combo stage has an active montage instance"), Instance);
			if (!Instance || Timing.X <= 0 || Timing.Y <= Timing.X) { AddError(TEXT("Combo stage is missing its release/window notifies")); break; }
			const int32 PreviousInstanceID = Instance->GetInstanceID();
			const float Rate = Source->GetCombatAttackSpeed();
			const float ReleaseDelta = (Timing.X - Instance->GetPosition()) / Rate + 0.002f;
			Instance->UpdateWeight(ReleaseDelta);
			Instance->Advance(ReleaseDelta, nullptr, false);
			++ExpectedShots;
			TestEqual(TEXT("The authored release notify emits exactly one shot"), CountShots(), ExpectedShots);
			Source->ReleaseRangedShot(ExpectedMontage);
			TestEqual(TEXT("Duplicate release events cannot emit another shot"), CountShots(), ExpectedShots);
			const float ComboDelta = (Timing.Y - Instance->GetPosition()) / Rate + 0.002f;
			Instance->UpdateWeight(ComboDelta);
			Instance->Advance(ComboDelta, nullptr, false);
			TestEqual(TEXT("A buffered attack advances at the combo window before the old montage ends"), Source->ActiveRangedMontage.Get(), ArcherProfile->RangedLightMontages[(Index+1)%4].Get());
			TestTrue(TEXT("Blending between attacks never releases movement"), Source->MeleeCombat->IsAttackMovementLocked() && Source->GetCharacterMovement()->MovementMode == MOVE_Walking);
			Source->OnRangedAttackEnded(ExpectedMontage, true);
			TestTrue(TEXT("The old montage ending cannot unlock the new combo stage"), Source->IsRangedAttacking() && Source->MeleeCombat->IsAttackMovementLocked() && Source->GetCharacterMovement()->MovementMode == MOVE_Walking);
			UAscendRangedNotify* StaleNotify = NewObject<UAscendRangedNotify>();
			FBranchingPointNotifyPayload Payload(Source->GetMesh(), Source->ActiveRangedMontage.Get(), nullptr, PreviousInstanceID);
			StaleNotify->BranchingPointNotify(Payload);
			TestEqual(TEXT("A stale montage instance cannot emit the new stage's shot"), CountShots(), ExpectedShots);
		}
		TestEqual(TEXT("The combo loops back to stage one after all four stages"), Source->ActiveRangedMontage.Get(), ArcherProfile->RangedLightMontages[0].Get());
		Controller->FireLightAttack();
		Source->TryInterruptNormalAttack();
		TestFalse(TEXT("Interruption discards the buffered combo request"), Controller->bQueuedRangedLight);
		Controller->ExecuteQueuedRangedAttack();
		TestFalse(TEXT("No queued shot restarts after a dodge interruption"), Source->IsRangedAttacking());
		Source->ReleaseRangedShot(ArcherProfile->RangedLightMontages[0]);
		TestEqual(TEXT("Cancelling before release prevents the pending shot"), CountShots(), ExpectedShots);
		TestTrue(TEXT("Cancelling the combo restores walking"), Source->GetCharacterMovement()->MovementMode == MOVE_Walking);
		Controller->FireLightAttack();
		Controller->FireLightAttack();
		Controller->QueuedRangedInputTime -= ArcherProfile->RangedInputBufferDuration + 0.01;
		Controller->ExecuteQueuedRangedAttack();
		TestFalse(TEXT("Expired input is discarded instead of firing an unwanted follow-up"), Controller->bQueuedRangedLight);
		TestEqual(TEXT("Expiring input leaves the current attack playing"), Source->ActiveRangedMontage.Get(), ArcherProfile->RangedLightMontages[0].Get());
		Source->TryInterruptNormalAttack();
		Controller->FireHeavyAttack(0.5f);
		TestEqual(TEXT("Heavy attack uses its own montage"), Source->ActiveRangedMontage.Get(), ArcherProfile->RangedHeavyMontage.Get());
		TestFalse(TEXT("Heavy timing is independent of normal combo montages"), ArcherProfile->RangedLightMontages.Contains(ArcherProfile->RangedHeavyMontage));
		TestEqual(TEXT("Heavy attack waits for its release event"), CountShots(), ExpectedShots);
		if (FAnimMontageInstance* HeavyInstance = Anim->GetActiveInstanceForMontage(ArcherProfile->RangedHeavyMontage))
		{
			const float HeavyRelease = UAscendMeleeEditorLibrary::GetRangedTiming(ArcherProfile->RangedHeavyMontage).X;
			const float Delta = HeavyRelease / Source->GetCombatAttackSpeed() + .002f;
			HeavyInstance->UpdateWeight(Delta);
			HeavyInstance->Advance(Delta, nullptr, false);
			++ExpectedShots;
			TestEqual(TEXT("Heavy release emits one arrow"), CountShots(), ExpectedShots);
			Source->ReleaseRangedShot(ArcherProfile->RangedHeavyMontage);
			TestEqual(TEXT("Heavy release cannot emit duplicate arrows"), CountShots(), ExpectedShots);
			bool FoundPiercingArrow = false;
			for (TActorIterator<AAscendRangedProjectile> It(World); It; ++It)
			{
				if (It->GetOwner() == Source && It->bPierceEnemies)
				{
				 FoundPiercingArrow = true;
				 TestNotNull(TEXT("Heavy shot uses a projectile body DA"),It->ProjectileProfile.Get());
				 TestEqual(TEXT("Deferred release assigns the selected projectile DA"),It->ProjectileProfile.Get(),ArcherProfile->RangedHeavyAttack.ProjectileProfile.Get());
				 if (It->ProjectileProfile)
				 {
				  TestTrue(TEXT("Arrow mesh comes from the projectile DA"),It->VisualMesh->GetStaticMesh()==It->ProjectileProfile->Mesh);
				  TestTrue(TEXT("Body transform is independent of charged collision radius"),It->VisualMesh->GetRelativeTransform().Equals(It->ProjectileProfile->MeshTransform,.001f));
				 }
				}
			}
			TestTrue(TEXT("Configured heavy attack emits a piercing arrow"), FoundPiercingArrow);
		}
		else { AddError(TEXT("Heavy montage did not initialize")); }
		Anim->Montage_Stop(0.f, ArcherProfile->RangedHeavyMontage);
		Source->OnRangedAttackEnded(ArcherProfile->RangedHeavyMontage, true);
		// Apply the actual fourth-stage root delta through character physics.
		const FAnimExtractContext ExtractContext;
		const FTransform RootDelta = ArcherProfile->RangedLightMontages[3]->ExtractRootMotionFromTrackRange(0.f, 1.5f, ExtractContext);
		TestTrue(TEXT("Archer roll retains authored root displacement"), RootDelta.GetTranslation().Size2D() > 50.f);
		auto* Movement = Source->GetCharacterMovement();
		Movement->RemoveRootMotionSource(TEXT("GamepadDodge"));
        Movement->SetMovementMode(MOVE_Flying);
		Source->MeleeCombat->LockAttackMovement();
		const FVector BeforeRootMotion = Source->GetActorLocation();
		Movement->RootMotionParams.Set(RootDelta);
		CastChecked<UAscendCharacterMovementComponent>(Movement)->PerformMovement(0.1f);
		TestTrue(TEXT("Attack root motion moves the capsule instead of only the mesh"), FVector::Dist2D(BeforeRootMotion, Source->GetActorLocation()) > 50.f);
		Source->MeleeCombat->UnlockAttackMovement();
	}
	// Sweep a fast arrow through two narrow enemies and into a wall.
	World->SetBegunPlay(true);
	for (AAscendEnemyCharacter* Enemy : {Target, Ally})
	{
		Enemy->GetCapsuleComponent()->SetCapsuleSize(8,20);
		Enemy->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Enemy->GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
		Enemy->GetCapsuleComponent()->SetGenerateOverlapEvents(true);
		Enemy->GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
		UAbilitySystemComponent* EnemyASC = Enemy->GetAbilitySystemComponent();
		EnemyASC->AddAttributeSetSubobject(NewObject<UAscendAttributeSet>(Enemy));
		Enemy->DispatchBeginPlay();
		EnemyASC->SetNumericAttributeBase(UAscendAttributeSet::GetMaxHealthAttribute(), 100.f);
		EnemyASC->SetNumericAttributeBase(UAscendAttributeSet::GetHealthAttribute(), 100.f);
	}
	Target->SetActorLocation(FVector(-300,2000,0));
	Ally->SetActorLocation(FVector(-150,2000,0));
	AActor* Wall = World->SpawnActor<AActor>(FVector(-50,2000,0), FRotator::ZeroRotator);
	UBoxComponent* WallBox = NewObject<UBoxComponent>(Wall);
	Wall->SetRootComponent(WallBox);
	WallBox->SetBoxExtent(FVector(10,100,100));
	WallBox->SetCollisionProfileName(TEXT("BlockAll"));
	WallBox->RegisterComponent();
	Wall->SetActorLocation(FVector(-50,2000,0));
	FActorSpawnParameters ArrowParams;
	ArrowParams.Owner = Source;
	ArrowParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AAscendRangedProjectile* Arrow = World->SpawnActor<AAscendRangedProjectile>(FVector(-500,2000,0), FRotator::ZeroRotator, ArrowParams);
	Arrow->DispatchBeginPlay();
	Arrow->InitializeProjectile(FVector::ForwardVector, 7.f, 4000.f, 5.f, true);
	Arrow->ProjectileMovement->TickComponent(.07f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Piercing the first enemy keeps the arrow moving"), Arrow->PiercedActors.Contains(Target) && !Arrow->bHasImpacted && !Arrow->ProjectileMovement->Velocity.IsNearlyZero());
	const int32 HitCount = Arrow->PiercedActors.Num();
	const float HealthAfterFirstHit = Target->GetAbilitySystemComponent()->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute());
	TestEqual(TEXT("Piercing arrow applies configured damage"), HealthAfterFirstHit, 93.f);
	Arrow->OnProjectileOverlap(Arrow->CollisionComponent, Target, Target->GetCapsuleComponent(), 0, false, FHitResult());
	TestEqual(TEXT("Repeated overlap does not register an enemy twice"), Arrow->PiercedActors.Num(), HitCount);
	TestEqual(TEXT("Repeated overlap does not damage an enemy twice"), Target->GetAbilitySystemComponent()->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute()), HealthAfterFirstHit);
	Arrow->ProjectileMovement->TickComponent(.13f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Fast piercing arrow hits both enemies before the wall"), Arrow->PiercedActors.Contains(Target) && Arrow->PiercedActors.Contains(Ally));
	TestTrue(TEXT("World geometry stops the piercing arrow"), Arrow->bHasImpacted && Arrow->IsActorBeingDestroyed());
	Controller->UnPossess();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif


