#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AscendDataLayerTrigger.generated.h"

class UBoxComponent;
class UDataLayerAsset;

/**
 * Player overlap volume that activates one runtime Data Layer and optionally
 * unloads other scene variants. Place one per transition area and configure
 * the layer assets directly in the level Details panel.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Ascend Data Layer Trigger"))
class PROJECTASCEND_API AAscendDataLayerTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAscendDataLayerTrigger();

	/** Applies the configured layer transition without requiring an overlap. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Data Layers")
	bool SwitchDataLayers();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> TriggerVolume;

	/** Scene layer activated when the player enters this volume. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Data Layer Transition")
	TObjectPtr<UDataLayerAsset> TargetDataLayer;

	/** Other variants to unload after TargetDataLayer has been activated. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Data Layer Transition")
	TArray<TObjectPtr<UDataLayerAsset>> DataLayersToDeactivate;

	/** Loaded keeps actors in memory but hidden; Unloaded releases them. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Data Layer Transition")
	bool bUnloadDeactivatedLayers = true;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Trigger")
	bool bTriggerOnlyOnce = true;

private:
	bool bHasTriggered = false;
};
