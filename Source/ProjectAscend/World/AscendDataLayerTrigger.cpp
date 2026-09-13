#include "World/AscendDataLayerTrigger.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"

AAscendDataLayerTrigger::AAscendDataLayerTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetGenerateOverlapEvents(true);
}

void AAscendDataLayerTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleTriggerBeginOverlap);
}

void AAscendDataLayerTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled() || (bTriggerOnlyOnce && bHasTriggered))
	{
		return;
	}

	if (SwitchDataLayers())
	{
		bHasTriggered = true;
	}
}

bool AAscendDataLayerTrigger::SwitchDataLayers()
{
	if (!TargetDataLayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DataLayerTrigger] %s has no TargetDataLayer."), *GetName());
		return false;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(this);
	if (!DataLayerManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DataLayerTrigger] %s requires a World Partition map."), *GetName());
		return false;
	}

	// Activate the destination first so transitions never intentionally enter a
	// state where every scene variant is inactive.
	if (!DataLayerManager->SetDataLayerRuntimeState(TargetDataLayer, EDataLayerRuntimeState::Activated))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DataLayerTrigger] %s could not activate %s."),
			*GetName(), *TargetDataLayer->GetName());
		return false;
	}

	const EDataLayerRuntimeState InactiveState = bUnloadDeactivatedLayers
		? EDataLayerRuntimeState::Unloaded
		: EDataLayerRuntimeState::Loaded;
	for (UDataLayerAsset* DataLayer : DataLayersToDeactivate)
	{
		if (DataLayer && DataLayer != TargetDataLayer)
		{
			DataLayerManager->SetDataLayerRuntimeState(DataLayer, InactiveState);
		}
	}

	UE_LOG(LogTemp, Display, TEXT("[DataLayerTrigger] %s activated %s."),
		*GetName(), *TargetDataLayer->GetName());
	return true;
}
