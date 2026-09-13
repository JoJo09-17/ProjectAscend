#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AscendMaterialControllerComponent.generated.h"

class UAscendMaterialController;
class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

USTRUCT()
struct FAscendActiveMaterialController
{
	GENERATED_BODY()
	UPROPERTY() TObjectPtr<UAscendMaterialController> Controller;
	int32 Handle = 0;
	float RemainingTime = 0.0f;
	bool bTimed = false;
};

/** Owns private MIDs and composes DA effects. Runtime state never lives in the DA. */
UCLASS(ClassGroup = (Ascend), meta = (BlueprintSpawnableComponent))
class PROJECTASCEND_API UAscendMaterialControllerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UAscendMaterialControllerComponent();

	/** Rebind after replacing a mesh/material loadout. Restores the old mesh first. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	bool InitializeMaterials(UMeshComponent* Mesh);

	/** Returns a removal handle, or zero on failure. Reapplying creates a new layer. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	int32 ApplyController(UAscendMaterialController* Controller);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	bool RemoveController(int32 Handle);

	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	void RemoveControllersByAsset(UAscendMaterialController* Controller);

	/** Removes effects but retains the reusable private MIDs. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	void ClearControllers();

	/** Removes effects and puts the original materials back. */
	UFUNCTION(BlueprintCallable, Category = "Ascend|Materials")
	void RestoreOriginalMaterials();

	UFUNCTION(BlueprintPure, Category = "Ascend|Materials")
	int32 GetActiveControllerCount() const { return ActiveControllers.Num(); }

	UFUNCTION(BlueprintPure, Category = "Ascend|Materials")
	UMaterialInstanceDynamic* GetDynamicMaterial(int32 Slot) const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void RebuildParameters();
	void UpdateTickEnabled();
	UPROPERTY(Transient) TObjectPtr<UMeshComponent> TargetMesh;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> DynamicMaterials;
	UPROPERTY(Transient) TArray<FAscendActiveMaterialController> ActiveControllers;
	int32 NextHandle = 1;
};
