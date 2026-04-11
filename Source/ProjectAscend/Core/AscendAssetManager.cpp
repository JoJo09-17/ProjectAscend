#include "AscendAssetManager.h"
#include "AbilitySystemGlobals.h"
#include "AscendLogChannels.h"

void UAscendAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// Required for TargetData and other GAS subsystems to function at runtime
	UE_LOG(LogAscend, Log, TEXT("AscendAssetManager: Initializing AbilitySystemGlobals global data for GAS runtime."));
	UAbilitySystemGlobals::Get().InitGlobalData();
}
