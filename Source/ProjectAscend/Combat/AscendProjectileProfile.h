#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AscendProjectileProfile.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UNiagaraSystem;

/** Projectile body and flight presentation. Attack damage, speed and radius live on the combat profile. */
UCLASS(BlueprintType)
class PROJECTASCEND_API UAscendProjectileProfile : public UDataAsset
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh") TObjectPtr<UStaticMesh> Mesh;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh") TObjectPtr<UMaterialInterface> MaterialOverride;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Mesh") FTransform MeshTransform;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FlightFX") TObjectPtr<UNiagaraSystem> FlightNiagara;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FlightFX") FTransform FlightFXTransform;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FlightFX") FName DirectionParameter = TEXT("User.ProjectileDirection");
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="FlightFX") bool bDirectionInLocalSpace = false;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Flight",meta=(ClampMin="0.1")) float LifeSpan = 3.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Flight",meta=(ClampMin="0")) float GravityScale = 0.f;
};
