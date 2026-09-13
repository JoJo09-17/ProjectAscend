#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AscendImpactFXProfile.generated.h"
class UNiagaraSystem;
class UParticleSystem;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EAscendDecalPlacement : uint8 { Ground, ImpactSurface };

/** Reusable cosmetic preset. Timing is authored by animation notifies. */
UCLASS(BlueprintType)
class PROJECTASCEND_API UAscendImpactFXProfile : public UDataAsset
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") TObjectPtr<UNiagaraSystem> NiagaraSystem;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX|Direction") FName NiagaraDirectionParameter = TEXT("User.ProjectileDirection");
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX|Direction") bool bNiagaraDirectionInLocalSpace = false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") TObjectPtr<UParticleSystem> ParticleSystem;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") FVector VFXScale = FVector(1.f);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") FVector VFXOffset = FVector::ZeroVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") bool bUseMeshSocket = false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX",meta=(EditCondition="bUseMeshSocket")) FName SocketName = TEXT("spine_03");
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX") bool bAttachVFX = false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="VFX",meta=(ClampMin="0.1")) float VFXLifetime = 2.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal") TObjectPtr<UMaterialInterface> DecalMaterial;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal") EAscendDecalPlacement DecalPlacement = EAscendDecalPlacement::Ground;
 // X is projection depth; Y/Z are the decal footprint.
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal",meta=(ClampMin="1")) FVector DecalSize = FVector(20,35,35);
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal",meta=(ClampMin="0")) float DecalFadeDelay = 3.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal",meta=(ClampMin="0.01")) float DecalFadeDuration = 1.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Decal",meta=(ClampMin="1")) float GroundTraceDistance = 500.f;
 UFUNCTION(BlueprintCallable,Category="Combat|ImpactFX")
 void SpawnEffects(AActor* Target, FVector ImpactPoint, FVector ImpactNormal, bool bSpawnVFX = true, bool bSpawnDecal = true, FVector ProjectileDirection = FVector::ZeroVector) const;
};
