#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AscendImpactFXNotify.generated.h"
class UAscendImpactFXProfile;

UCLASS(meta=(DisplayName="Ascend Impact FX"))
class PROJECTASCEND_API UAscendImpactFXNotify : public UAnimNotify
{
 GENERATED_BODY()
public:
 UAscendImpactFXNotify();
 // Empty uses the character combat profile. Set a DA to override per notify.
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ImpactFX") TObjectPtr<UAscendImpactFXProfile> Profile;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ImpactFX") bool bSpawnVFX = true;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ImpactFX") bool bSpawnDecal = true;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="ImpactFX") bool bUseHitReactionContext = true;
 virtual FString GetNotifyName_Implementation() const override;
 virtual void Notify(USkeletalMeshComponent* Mesh,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& Reference) override;
 virtual void BranchingPointNotify(FBranchingPointNotifyPayload& Payload) override;
};
