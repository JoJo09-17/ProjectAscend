#include "Materials/AscendToonSettings.h"
#include "Engine/JoJoToonProfileLibrary.h"
#include "Engine/Engine.h"
#include "Containers/Ticker.h"
#if WITH_EDITOR
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#endif

void UAscendToonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (!IsRunningCommandlet())
	{
		StartupHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float)
		{
			ReloadProfiles();
			return false;
		}));
	}
}
void UAscendToonSubsystem::ReloadProfiles()
{
	ActiveLibrary = GetDefault<UAscendToonSettings>()->ProfileLibrary.LoadSynchronous();
	if (ActiveLibrary)
	{
		ActiveLibrary->ApplyProfiles();
		UE_LOG(LogTemp, Display, TEXT("JoJo Toon: applied profile library %s"), *ActiveLibrary->GetPathName());
	}
	else { UJoJoToonProfileLibrary::ResetProfiles(); }
}
void UAscendToonSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(StartupHandle);
	ActiveLibrary = nullptr;
	Super::Deinitialize();
}
#if WITH_EDITOR
bool UAscendToonSettings::ConnectToonInputs(UMaterial* Material, UMaterialExpression* Profile, UMaterialExpression* DetailScale)
{
	if (!Material || !Profile || !DetailScale || Profile->GetOuter() != Material || DetailScale->GetOuter() != Material) { return false; }
	Material->Modify();
	Material->GetExpressionInputForProperty(MP_CustomData0)->Connect(0, Profile);
	Material->GetExpressionInputForProperty(MP_CustomData1)->Connect(0, DetailScale);
	Material->PostEditChange();
	return true;
}

void UAscendToonSettings::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	if (GEngine)
	{
		if (UAscendToonSubsystem* Subsystem = GEngine->GetEngineSubsystem<UAscendToonSubsystem>()) { Subsystem->ReloadProfiles(); }
	}
}
#endif
