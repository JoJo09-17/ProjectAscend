#include "Combat/AscendMeleeCombatComponent.h"
#include "Combat/AscendMeleeAbility.h"
#include "Combat/AscendMeleeDamageEffect.h"
#include "Combat/AscendMeleeWindow.h"
#include "Character/Base/AscendCharacterBase.h"
#include "Character/Enemy/AscendEnemyCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AscendGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarMeleeDebug(TEXT("ascend.Melee.Debug"), 0, TEXT("Draw authoritative melee swept boxes and hits."));

UAscendMeleeCombatComponent::UAscendMeleeCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
	SetIsReplicatedByDefault(true);
}

void UAscendMeleeCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<AAscendCharacterBase>(GetOwner());
	if (Character.IsValid() && Profile)
	{
		USkeletalMeshComponent* Mesh = Character->GetMesh();
		Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		Mesh->bEnableUpdateRateOptimizations = false;
		AddTickPrerequisiteComponent(Mesh);
		if (Profile->CombatStyle == EAscendCombatProfileStyle::Saber) { CreateWeapon(); }
	}
}

void UAscendMeleeCombatComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	CancelAttack();
	if (Character.IsValid() && Character->HasAuthority())
		if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent()) { ASC->ClearAbility(AbilityHandle); }
	Super::EndPlay(Reason);
}

bool UAscendMeleeCombatComponent::RequestAttack(bool bHeavy)
{
	if (const auto* Enemy = Cast<AAscendEnemyCharacter>(GetOwner()); Enemy && Enemy->IsHitReacting()) { return false; }
	if (!Character.IsValid() || Character->IsDead() || !Profile || Profile->CombatStyle != EAscendCombatProfileStyle::Saber) { return false; }
	if (!GetOwner()->HasAuthority()) { ServerRequestAttack(bHeavy); return true; }
	if (IsAttacking())
	{
		// Buffer one light input late in recovery; never restart a live hit window.
		UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance();
		if (!bHeavy && Anim && CurrentAttack.Montage &&
			Anim->Montage_GetPosition(CurrentAttack.Montage) >= CurrentAttack.Montage->GetPlayLength() * 0.55f)
		{
			bQueuedLight = true;
			return true;
		}
		return false;
	}
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!ASC || !ASC->AbilityActorInfo.IsValid() || !ASC->AbilityActorInfo->AvatarActor.IsValid()) { return false; }
	if (!ASC->FindAbilitySpecFromHandle(AbilityHandle))
	{
		AbilityHandle = ASC->GiveAbility(FGameplayAbilitySpec(UAscendMeleeAbility::StaticClass(), 1, INDEX_NONE, this));
	}
	bHeavyRequested = bHeavy;
	return ASC->TryActivateAbility(AbilityHandle);
}

void UAscendMeleeCombatComponent::ServerRequestAttack_Implementation(bool bHeavy) { RequestAttack(bHeavy); }

bool UAscendMeleeCombatComponent::StartAttack(UAscendMeleeAbility* Ability, FAscendMeleeAttack& OutAttack)
{
	if (const auto* Enemy = Cast<AAscendEnemyCharacter>(GetOwner()); Enemy && Enemy->IsHitReacting()) { return false; }
	if (!Character.IsValid() || Character->IsDead() || !Profile || IsAttacking()) { return false; }
	if (bHeavyRequested) { CurrentAttack = Profile->HeavyAttack; ComboIndex = 0; }
	else
	{
		if (Profile->LightAttacks.IsEmpty()) { return false; }
		if (GetWorld()->GetTimeSeconds() - LastAttackEnd > 0.8) { ComboIndex = 0; }
		CurrentAttack = Profile->LightAttacks[ComboIndex % Profile->LightAttacks.Num()];
		ComboIndex = (ComboIndex + 1) % Profile->LightAttacks.Num();
	}
	if (!CurrentAttack.Montage || !Character->GetMesh()->GetAnimInstance() ||
		CurrentAttack.Montage->GetSkeleton() != Character->GetMesh()->GetSkeletalMeshAsset()->GetSkeleton()) { return false; }
	ActiveAbility = Ability;
	bQueuedLight = false;
	OutAttack = CurrentAttack;
	LockAttackMovement();
	UE_LOG(LogTemp, Display, TEXT("[AscendMelee] %s started %s."),
		*GetOwner()->GetName(), *CurrentAttack.Montage->GetName());
	return true;
}

void UAscendMeleeCombatComponent::FinishAttack(UAscendMeleeAbility* Ability, bool bCancelled)
{
	if (ActiveAbility.Get() != Ability) { return; }
	WindowToken.Reset();
	ActiveAbility.Reset();
	UnlockAttackMovement();
	LastAttackEnd = GetWorld()->GetTimeSeconds();
	if (bCancelled)
	{
		ComboIndex = 0;
		bQueuedLight = false;
		TrailSamples.Reset();
		if (Trail) { Trail->ClearAllMeshSections(); }
	}
	if (bQueuedLight && Character.IsValid() && !Character->IsDead())
	{
		bQueuedLight = false;
		QueuedAttackTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() { RequestAttack(false); }));
	}
}

void UAscendMeleeCombatComponent::CancelAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(QueuedAttackTimer);
	bQueuedLight = false;
	if (ActiveAbility.IsValid())
	{
		if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent()) { ASC->CancelAbilityHandle(AbilityHandle); }
	}
	WindowToken.Reset();
	ActiveAbility.Reset();
	UnlockAttackMovement();
	TrailSamples.Reset();
	if (Trail) { Trail->ClearAllMeshSections(); }
}

void UAscendMeleeCombatComponent::LockAttackMovement()
{
	if (bAttackMovementLocked || !Character.IsValid()) { return; }
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	MovementModeBeforeAttack = Movement->MovementMode;
	CustomMovementModeBeforeAttack = Movement->CustomMovementMode;
	bAttackMovementLocked = true;
	if (AController* Controller = Character->GetController()) { Controller->StopMovement(); }
	Character->ConsumeMovementInputVector();
	Movement->StopMovementImmediately();
	Movement->ClearAccumulatedForces();
	// Keep floor/gravity/collision simulation running. The movement component
	// blocks input and path velocity while allowing authored montage root motion.
	if (UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance())
	{
		Anim->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
	}
}

void UAscendMeleeCombatComponent::UnlockAttackMovement()
{
	if (!bAttackMovementLocked) { return; }
	bAttackMovementLocked = false;
	if (!Character.IsValid()) { return; }
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	Character->ConsumeMovementInputVector();
	Movement->StopMovementImmediately();
	Movement->ClearAccumulatedForces();
	if (!Character->IsDead() && Movement->MovementMode == MOVE_None)
	{
		Movement->SetMovementMode(static_cast<EMovementMode>(MovementModeBeforeAttack), CustomMovementModeBeforeAttack);
	}
}

void UAscendMeleeCombatComponent::BeginDamageWindow(const UAnimNotifyState* Token)
{
	if (!Profile || !Character.IsValid() || Character->IsDead() || (!IsAttacking() && GetOwner()->HasAuthority())) { return; }
	WindowToken = Token;
	HitActors.Reset();
	TrailSamples.Reset();
	PreviousBlade = GetBladeTransform();
	UE_LOG(LogTemp, Verbose, TEXT("[AscendMelee] %s opened its damage window at %s facing %s."),
		*GetOwner()->GetName(), *PreviousBlade.GetLocation().ToCompactString(),
		*PreviousBlade.GetRotation().Rotator().ToCompactString());
}

void UAscendMeleeCombatComponent::EndDamageWindow(const UAnimNotifyState* Token)
{
	if (WindowToken.Get() == Token)
	{
		// Capture the last portion of the swing before closing its damage gate.
		if (Character.IsValid() && !Character->IsDead() && GetOwner()->HasAuthority() && IsAttacking()) { TraceBlade(PreviousBlade, GetBladeTransform()); }
		WindowToken.Reset();
	}
}

bool UAscendMeleeCombatComponent::AreHostile(const AActor* Source, const AActor* Target)
{
	const AAscendCharacterBase* Attacker = Cast<AAscendCharacterBase>(Source);
	const AAscendCharacterBase* Victim = Cast<AAscendCharacterBase>(Target);
	return IsValid(Attacker) && IsValid(Victim) && Source != Target && !Attacker->IsDead() && !Victim->IsDead()
		&& !Victim->IsActorBeingDestroyed() && (Attacker->IsA<AAscendEnemyCharacter>() != Victim->IsA<AAscendEnemyCharacter>());
}

void UAscendMeleeCombatComponent::SweepBlade(UWorld* World, const FTransform& From, const FTransform& To,
	const FVector& Extent, AActor* Source, TArray<FHitResult>& OutHits)
{
	if (!World || FVector::DistSquared(From.GetLocation(), To.GetLocation()) > FMath::Square(500.f)) { return; }
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AscendMeleeBlade), false, Source);
	const float Angle = From.GetRotation().AngularDistance(To.GetRotation());
	const float Travel = FVector::Distance(From.GetLocation(), To.GetLocation()) + Angle * Extent.X;
	const int32 Steps = FMath::Clamp(FMath::CeilToInt(Travel / FMath::Max(Extent.Y, 1.f)), 1, 128);
	for (int32 Step = 0; Step < Steps; ++Step)
	{
		const float A = float(Step) / Steps, B = float(Step + 1) / Steps;
		const FQuat Rotation = FQuat::Slerp(From.GetRotation(), To.GetRotation(), (A + B) * 0.5f);
		TArray<FHitResult> Hits;
		// The small padding covers the angular arc between adjacent orientation samples.
		const float Padding = Extent.X * FMath::Sin(Angle / (2.f * Steps));
		World->SweepMultiByObjectType(Hits, FMath::Lerp(From.GetLocation(), To.GetLocation(), A),
			FMath::Lerp(From.GetLocation(), To.GetLocation(), B), Rotation, Objects,
			FCollisionShape::MakeBox(Extent + FVector(0, Padding, Padding)), Params);
		OutHits.Append(Hits);
	}
}

FTransform UAscendMeleeCombatComponent::GetBladeTransform() const
{
	if (!Character.IsValid() || !Profile) { return FTransform::Identity; }
	const FTransform Weapon = Sword
		? Sword->GetComponentTransform()
		: Profile->WeaponOffset * Character->GetMesh()->GetSocketTransform(Profile->WeaponBone);
	const FVector Center = Weapon.TransformPosition(FVector(10.f + Profile->BladeLength * 0.5f, 0, 0));
	return FTransform(Weapon.GetRotation(), Center, FVector::OneVector);
}

void UAscendMeleeCombatComponent::TraceBlade(const FTransform& From, const FTransform& To)
{
	if (!GetOwner()->HasAuthority() || !Profile || !Character.IsValid()) { return; }
	const FVector Extent(Profile->BladeLength * 0.5f, Profile->HitHalfWidth, Profile->HitHalfWidth);
	TArray<FHitResult> Hits;
	SweepBlade(GetWorld(), From, To, Extent, GetOwner(), Hits);
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!AreHostile(GetOwner(), Target) || HitActors.Contains(Target)) { continue; }
		FCollisionObjectQueryParams Obstacles;
		Obstacles.AddObjectTypesToQuery(ECC_WorldStatic);
		Obstacles.AddObjectTypesToQuery(ECC_WorldDynamic);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AscendMeleeOcclusion), false, GetOwner());
		Params.AddIgnoredActor(Target);
		FHitResult Wall;
		// Check the blade segment and attacker-to-target path, including closed dynamic doors.
		const FVector Impact = Hit.bStartPenetrating ? Target->GetActorLocation() : FVector(Hit.ImpactPoint);
		const FVector BladeBase = To.TransformPosition(FVector(-Extent.X, 0, 0));
		if (GetWorld()->LineTraceSingleByObjectType(Wall, GetOwner()->GetActorLocation(), Impact, Obstacles, Params) ||
			GetWorld()->LineTraceSingleByObjectType(Wall, BladeBase, Impact, Obstacles, Params)) { continue; }
		UAbilitySystemComponent* SourceASC = Character->GetAbilitySystemComponent();
		UAbilitySystemComponent* TargetASC = CastChecked<AAscendCharacterBase>(Target)->GetAbilitySystemComponent();
		if (!SourceASC || !TargetASC) { continue; }
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(GetOwner(), GetOwner());
		Context.AddHitResult(Hit);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(UAscendMeleeDamageEffect::StaticClass(), 1.f, Context);
		if (!Spec.IsValid()) { continue; }
		HitActors.Add(Target); // Before applying: death/cancellation callbacks may be re-entrant.
		Spec.Data->SetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_Damage, FMath::Max(0.f, CurrentAttack.Damage));
		Spec.Data->SetSetByCallerMagnitude(AscendGameplayTags::SetByCaller_DamageMultiplier, FMath::Max(0.f, CurrentAttack.DamageMultiplier));
		SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
		UE_LOG(LogTemp, Display, TEXT("[AscendMelee] %s hit %s; health=%.1f"), *GetOwner()->GetName(), *Target->GetName(), TargetASC->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute()));
		if (bDrawHitBoxes || CVarMeleeDebug.GetValueOnGameThread()) { DrawDebugSphere(GetWorld(), Impact, 9.f, 8, FColor::Red, false, 0.3f); }
		if (!IsAttacking() || Character->IsDead()) { break; }
	}
	if (bDrawHitBoxes || CVarMeleeDebug.GetValueOnGameThread())
	{
		DrawDebugBox(GetWorld(), To.GetLocation(), Extent, To.GetRotation(), FColor::Cyan, false, 0.12f);
		DrawDebugLine(GetWorld(), From.GetLocation(), To.GetLocation(), FColor::Yellow, false, 0.12f);
	}
}

void UAscendMeleeCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick)
{
	Super::TickComponent(DeltaTime, TickType, Tick);
	if (!Character.IsValid() || !Profile) { return; }
	if (Character->IsDead()) { if (IsAttacking() || WindowToken.IsValid()) { CancelAttack(); } return; }
	if (WindowToken.IsValid())
	{
		const FTransform Current = GetBladeTransform();
		if (IsAttacking() && GetOwner()->HasAuthority()) { TraceBlade(PreviousBlade, Current); }
		if (HitBox) { HitBox->SetWorldTransform(Current); }
		PreviousBlade = Current;
	}
	UpdateTrail(DeltaTime);
}

void UAscendMeleeCombatComponent::CreateWeapon()
{
	if (!Character->GetMesh()->DoesSocketExist(Profile->WeaponBone)) { return; }
	Sword = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("MeleeSword"));
	Sword->SetupAttachment(Character->GetMesh(), Profile->WeaponBone);
	Sword->SetRelativeTransform(Profile->WeaponOffset);
	Sword->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sword->SetCanEverAffectNavigation(false);
	Sword->RegisterComponent();
	// A steel blade, guard and handle, in centimeters. Authoring an external sword is optional.
	TArray<FVector> V;
	TArray<int32> I;
	auto AddBox = [&](FVector C, FVector E)
	{
		const int32 N = V.Num();
		for (int Z : {-1, 1}) for (int Y : {-1, 1}) for (int X : {-1, 1}) { V.Add(C + E * FVector(X,Y,Z)); }
		const int32 Faces[] = {0,2,3,0,3,1,4,5,7,4,7,6,0,1,5,0,5,4,2,6,7,2,7,3,0,4,6,0,6,2,1,3,7,1,7,5};
		for (int32 Index : Faces) { I.Add(N + Index); }
	};
	AddBox(FVector(2,0,0), FVector(8,1.8f,1.8f));
	AddBox(FVector(10,0,0), FVector(2,12,2));
	const int32 N = V.Num();
	V.Append({FVector(12,-4,0),FVector(12,0,-1.2f),FVector(12,4,0),FVector(12,0,1.2f),FVector(10+Profile->BladeLength,0,0)});
	const int32 BladeFaces[] = {0,1,4,1,2,4,2,3,4,3,0,4,0,3,2,0,2,1};
	for (int32 Index : BladeFaces) { I.Add(N+Index); }
	TArray<FVector2D> UV; UV.Init(FVector2D::ZeroVector, V.Num());
	TArray<FVector> Normals; TArray<FProcMeshTangent> Tangents;
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(V, I, UV, Normals, Tangents);
	Sword->CreateMeshSection_LinearColor(0, V, I, Normals, UV, TArray<FLinearColor>(), Tangents, false);
	Sword->SetMaterial(0, Profile->SwordMaterial);
	Trail = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("MeleeBladeTrail"));
	Trail->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Trail->SetCastShadow(false);
	Trail->SetCanEverAffectNavigation(false);
	Trail->RegisterComponent(); // World-space vertices; do not attach to the moving hand.
	Trail->SetMaterial(0, Profile->TrailMaterial);
	HitBox = NewObject<UBoxComponent>(GetOwner(), TEXT("MeleeBladeHitBox"));
	HitBox->SetBoxExtent(FVector(Profile->BladeLength * 0.5f, Profile->HitHalfWidth, Profile->HitHalfWidth));
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Queried explicitly as a continuous sweep, not overlap events.
	HitBox->SetCanEverAffectNavigation(false);
	HitBox->RegisterComponent();
}

void UAscendMeleeCombatComponent::UpdateTrail(float DeltaTime)
{
	if (!Trail) { return; }
	for (FTrailSample& Sample : TrailSamples) { Sample.Age += DeltaTime; }
	TrailSamples.RemoveAll([](const FTrailSample& Sample) { return Sample.Age > 0.14f; });
	if (WindowToken.IsValid())
	{
		const FTransform Pose = GetBladeTransform();
		TrailSamples.Add({Pose.TransformPosition(FVector(-Profile->BladeLength * 0.35f,0,0)), Pose.TransformPosition(FVector(Profile->BladeLength * 0.5f,0,0)), 0.f});
	}
	if (TrailSamples.Num() < 2) { Trail->ClearAllMeshSections(); return; }
	TArray<FVector> V, Normals;
	TArray<FVector2D> UV;
	TArray<int32> I;
	TArray<FLinearColor> Colors;
	for (int32 Index = 0; Index < TrailSamples.Num(); ++Index)
	{
		const FTrailSample& Sample = TrailSamples[Index];
		V.Append({Sample.Base, Sample.Tip});
		UV.Append({FVector2D(0, float(Index)/TrailSamples.Num()), FVector2D(1,float(Index)/TrailSamples.Num())});
		FLinearColor Color = Profile->TrailColor;
		Color.A *= 1.f - Sample.Age / 0.14f;
		Colors.Append({Color, Color});
		Normals.Append({FVector::UpVector, FVector::UpVector});
		if (Index > 0) { const int32 P = Index*2; I.Append({P-2,P-1,P,P-1,P+1,P}); }
	}
	Trail->CreateMeshSection_LinearColor(0, V, I, Normals, UV, Colors, TArray<FProcMeshTangent>(), false);
}
