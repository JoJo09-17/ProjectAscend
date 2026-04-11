#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AscendAbilitySystemSettings.generated.h"

class UAscendAbilityTagRelationshipMapping;

/**
 * Global project settings for shared GAS authoring assets.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Ascend Gameplay Abilities Settings"))
class PROJECTASCEND_API UAscendAbilitySystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Returns the default global ability tag relationship mapping asset. */
	const UAscendAbilityTagRelationshipMapping* GetDefaultAbilityTagRelationshipMapping() const;

protected:
	/** Project-wide default relationship mapping used by ASC instances that do not override it. */
	UPROPERTY(Config, EditAnywhere, Category = "Ability Tags")
	TSoftObjectPtr<UAscendAbilityTagRelationshipMapping> DefaultAbilityTagRelationshipMapping;
};
