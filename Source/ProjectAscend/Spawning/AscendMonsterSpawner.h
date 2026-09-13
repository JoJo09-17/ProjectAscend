#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AscendMonsterSpawner.generated.h"

class AAscendCharacterBase;
class AAscendEnemyCharacter;
class USceneComponent;
class USphereComponent;

USTRUCT(BlueprintType)
struct PROJECTASCEND_API FAscendMonsterSpawnEntry
{
	GENERATED_BODY()

	/** Enemy Blueprint or native class to create. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	TSubclassOf<AAscendEnemyCharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "1", ClampMax = "200"))
	int32 Count = 1;

	/** Radius around this entry's local offset used for random placement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0"))
	float SpawnRadius = 500.0f;

	/** Prevents monsters from being placed on top of one another. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "0.0"))
	float MinimumSpacing = 150.0f;

	/** Per-entry center relative to the spawner, useful for mixed encounter groups. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	FVector LocalOffset = FVector::ZeroVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAscendMonsterSpawned,
	AAscendEnemyCharacter*, SpawnedMonster,
	AActor*, Spawner);

/**
 * Data-driven encounter spawner placed directly in a level.
 * Each map owns its SpawnEntries, so level designers can compose different
 * monster groups without adding level-name switches to gameplay code.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Ascend Monster Spawner"))
class PROJECTASCEND_API AAscendMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	AAscendMonsterSpawner();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Creates every configured monster. Safe to call from level Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Spawning")
	void SpawnConfiguredMonsters();

	UFUNCTION(BlueprintPure, Category = "Ascend|Spawning")
	int32 GetAliveMonsterCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Ascend|Spawning")
	FOnAscendMonsterSpawned OnMonsterSpawned;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	bool bSpawnOnBeginPlay = true;

	/** Prevents repeated trigger calls from duplicating an encounter. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	bool bSpawnOnlyOnce = true;

	/** Zero derives a stable seed from this actor's path. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning", meta = (TitleProperty = "MonsterClass"))
	TArray<FAscendMonsterSpawnEntry> SpawnEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement", meta = (ClampMin = "1", ClampMax = "100"))
	int32 MaxPlacementAttempts = 16;

private:
	bool FindSpawnLocation(
		const FAscendMonsterSpawnEntry& Entry,
		FRandomStream& RandomStream,
		const TArray<FVector>& ReservedLocations,
		FVector& OutLocation) const;

	UFUNCTION()
	void HandleSpawnedMonsterDied(AAscendCharacterBase* Victim);

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Selection preview for the largest configured spawn radius. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> RadiusPreview;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AAscendEnemyCharacter>> SpawnedMonsters;

	bool bHasSpawned = false;
};
