#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Ticker.h"
#include "AscendToonSettings.generated.h"

class UJoJoToonProfileLibrary;
class UMaterial;
class UMaterialExpression;

UCLASS(Config=Engine, DefaultConfig, meta=(DisplayName="JoJo Toon BRDF"))
class PROJECTASCEND_API UAscendToonSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }
	UPROPERTY(Config, EditAnywhere, Category="Style") TSoftObjectPtr<UJoJoToonProfileLibrary> ProfileLibrary;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	/** Connect the custom material inputs hidden from Unreal's Python enum. */
	UFUNCTION(BlueprintCallable, Category="Ascend|Toon|Authoring")
	static bool ConnectToonInputs(UMaterial* Material, UMaterialExpression* Profile, UMaterialExpression* DetailScale);
#endif
};

UCLASS()
class PROJECTASCEND_API UAscendToonSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION(BlueprintCallable, Category="Ascend|Toon") void ReloadProfiles();
private:
	UPROPERTY(Transient) TObjectPtr<UJoJoToonProfileLibrary> ActiveLibrary;
	FTSTicker::FDelegateHandle StartupHandle;
};
