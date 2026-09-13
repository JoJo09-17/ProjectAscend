#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AscendMeleeEditorLibrary.generated.h"
class UAnimBlueprint;
class UAnimMontage;
class UNiagaraSystem;
class UAnimSequence;

UCLASS()
class PROJECTASCEND_API UAscendMeleeEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable,Category="Combat|Editor") static bool EnsureNiagaraDirectionParameter(UNiagaraSystem* System,FName ParameterName = TEXT("User.ProjectileDirection"));
 UFUNCTION(BlueprintPure,Category="Combat|Editor") static TArray<FName> GetNiagaraUserParameterNames(UNiagaraSystem* System);
	UFUNCTION(BlueprintCallable,Category="Combat|Editor")
	static bool SetImpactFXTiming(UAnimMontage* Montage,float VFXTime,float DecalTime);
	UFUNCTION(BlueprintCallable, Category="Combat|Editor")
	static bool SetRangedReleaseTime(UAnimMontage* Montage, float ReleaseTime);
	UFUNCTION(BlueprintCallable, Category="Combat|Editor")
	static bool BakePelvisRootMotion(UAnimSequence* Sequence);
	/** Adds a montage slot between the existing locomotion graph and its output. Editor only. */
	UFUNCTION(BlueprintCallable, Category="Melee|Editor")
	static bool EnsureMontageSlot(UAnimBlueprint* Blueprint, FName SlotName = TEXT("DefaultSlot"));

	/** Replaces the named hit-window notify and validates its range. Editor only. */
	UFUNCTION(BlueprintCallable, Category="Melee|Editor")
	static bool SetMeleeWindow(UAnimMontage* Montage, float StartTime, float Duration);

	/** Routes every montage track through the dedicated graph slot. */
	UFUNCTION(BlueprintCallable, Category="Melee|Editor")
	static bool SetMontageSlot(UAnimMontage* Montage, FName SlotName);

	UFUNCTION(BlueprintPure, Category="Melee|Editor")
	static FName GetMontageSlot(const UAnimMontage* Montage);

	UFUNCTION(BlueprintPure, Category="Melee|Editor")
	static int32 GetMeleeWindowCount(const UAnimMontage* Montage);
	/** Authors one release and a combo-open/close pair, preserving unrelated notifies. */
	UFUNCTION(BlueprintCallable, Category="Combat|Editor")
	static bool SetRangedTiming(UAnimMontage* Montage, float ReleaseTime, float ComboOpenTime, float ComboCloseTime, float BlendTime = 0.1f);
	UFUNCTION(BlueprintPure, Category="Combat|Editor")
	static FVector GetRangedTiming(const UAnimMontage* Montage);
};
