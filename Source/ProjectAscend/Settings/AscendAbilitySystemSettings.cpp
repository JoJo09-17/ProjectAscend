#include "Settings/AscendAbilitySystemSettings.h"

#include "AbilitySystem/Data/AscendAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilitySystemSettings)

const UAscendAbilityTagRelationshipMapping* UAscendAbilitySystemSettings::GetDefaultAbilityTagRelationshipMapping() const
{
	return DefaultAbilityTagRelationshipMapping.LoadSynchronous();
}
