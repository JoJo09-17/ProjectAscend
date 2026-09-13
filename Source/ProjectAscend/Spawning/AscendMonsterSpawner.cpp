#include "Spawning/AscendMonsterSpawner.h"

#include "Character/Enemy/AscendEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"

AAscendMonsterSpawner::AAscendMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	RadiusPreview = CreateDefaultSubobject<USphereComponent>(TEXT("RadiusPreview"));
	RadiusPreview->SetupAttachment(SceneRoot);
	RadiusPreview->SetSphereRadius(500.0f);
	RadiusPreview->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RadiusPreview->SetCanEverAffectNavigation(false);
	RadiusPreview->SetHiddenInGame(true);
	RadiusPreview->ShapeColor = FColor(255, 96, 32);
}

void AAscendMonsterSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	float LargestRadius = 100.0f;
	for (const FAscendMonsterSpawnEntry& Entry : SpawnEntries)
	{
		LargestRadius = FMath::Max(LargestRadius, Entry.SpawnRadius + Entry.LocalOffset.Size2D());
	}
	RadiusPreview->SetSphereRadius(LargestRadius);
}

void AAscendMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && bSpawnOnBeginPlay)
	{
		SpawnConfiguredMonsters();
	}
}

void AAscendMonsterSpawner::SpawnConfiguredMonsters()
{
	if (!HasAuthority() || (bSpawnOnlyOnce && bHasSpawned))
	{
		return;
	}

	bHasSpawned = true;
	SpawnedMonsters.RemoveAll([](const AAscendEnemyCharacter* Monster) { return !IsValid(Monster); });

	const int32 EffectiveSeed = RandomSeed != 0 ? RandomSeed : static_cast<int32>(GetTypeHash(GetPathName()));
	FRandomStream RandomStream(EffectiveSeed);
	TArray<FVector> ReservedLocations;

	for (const FAscendMonsterSpawnEntry& Entry : SpawnEntries)
	{
		if (!Entry.MonsterClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] %s has an entry with no MonsterClass."), *GetName());
			continue;
		}

		for (int32 SpawnIndex = 0; SpawnIndex < FMath::Clamp(Entry.Count, 1, 200); ++SpawnIndex)
		{
			FVector SpawnLocation;
			if (!FindSpawnLocation(Entry, RandomStream, ReservedLocations, SpawnLocation))
			{
				UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] %s could not find room for %s."),
					*GetName(), *Entry.MonsterClass->GetName());
				continue;
			}

			const AAscendEnemyCharacter* MonsterDefault = Entry.MonsterClass->GetDefaultObject<AAscendEnemyCharacter>();
			const float CapsuleHalfHeight = MonsterDefault && MonsterDefault->GetCapsuleComponent()
				? MonsterDefault->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
				: 96.0f;
			SpawnLocation.Z += CapsuleHalfHeight + 2.0f;

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Owner = this;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			const FRotator SpawnRotation(0.0f, RandomStream.FRandRange(-180.0f, 180.0f), 0.0f);
			AAscendEnemyCharacter* Monster = GetWorld()->SpawnActor<AAscendEnemyCharacter>(
				Entry.MonsterClass, SpawnLocation, SpawnRotation, SpawnParameters);
			if (!Monster)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MonsterSpawner] %s failed to spawn %s at %s."),
					*GetName(), *Entry.MonsterClass->GetName(), *SpawnLocation.ToCompactString());
				continue;
			}

			Monster->Tags.AddUnique(TEXT("SpawnedByLevelSpawner"));
			Monster->OnCharacterDied.AddDynamic(this, &ThisClass::HandleSpawnedMonsterDied);
			SpawnedMonsters.Add(Monster);
			ReservedLocations.Add(SpawnLocation);
			UE_LOG(LogTemp, Display, TEXT("[MonsterSpawner] Spawned %s at %s."),
				*Monster->GetClass()->GetName(), *SpawnLocation.ToCompactString());
			OnMonsterSpawned.Broadcast(Monster, this);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("[MonsterSpawner] %s spawned %d monsters."), *GetName(), GetAliveMonsterCount());
}

bool AAscendMonsterSpawner::FindSpawnLocation(
	const FAscendMonsterSpawnEntry& Entry,
	FRandomStream& RandomStream,
	const TArray<FVector>& ReservedLocations,
	FVector& OutLocation) const
{
	const FVector Center = GetActorTransform().TransformPosition(Entry.LocalOffset);
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	for (int32 Attempt = 0; Attempt < FMath::Clamp(MaxPlacementAttempts, 1, 100); ++Attempt)
	{
		const float Angle = RandomStream.FRandRange(0.0f, 2.0f * UE_PI);
		const float Radius = FMath::Sqrt(RandomStream.FRand()) * FMath::Max(Entry.SpawnRadius, 0.0f);
		FVector Candidate = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

		if (Navigation)
		{
			FNavLocation ProjectedLocation;
			if (!Navigation->ProjectPointToNavigation(Candidate, ProjectedLocation, FVector(250.0f, 250.0f, 500.0f)))
			{
				continue;
			}
			Candidate = ProjectedLocation.Location;
		}
		else
		{
			FHitResult FloorHit;
			const FVector TraceStart = Candidate + FVector(0.0f, 0.0f, 1500.0f);
			const FVector TraceEnd = Candidate - FVector(0.0f, 0.0f, 3000.0f);
			if (GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_WorldStatic))
			{
				Candidate = FloorHit.ImpactPoint;
			}
		}

		const bool bHasSpacing = ReservedLocations.ContainsByPredicate([&](const FVector& ReservedLocation)
		{
			return FVector::DistSquared2D(Candidate, ReservedLocation) < FMath::Square(Entry.MinimumSpacing);
		});
		if (!bHasSpacing)
		{
			OutLocation = Candidate;
			return true;
		}
	}

	return false;
}

void AAscendMonsterSpawner::HandleSpawnedMonsterDied(AAscendCharacterBase* Victim)
{
	SpawnedMonsters.Remove(Cast<AAscendEnemyCharacter>(Victim));
}

int32 AAscendMonsterSpawner::GetAliveMonsterCount() const
{
	int32 AliveCount = 0;
	for (const AAscendEnemyCharacter* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster) && !Monster->IsDead())
		{
			++AliveCount;
		}
	}
	return AliveCount;
}
