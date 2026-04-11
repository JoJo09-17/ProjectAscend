#include "AscendPlayerState.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"

AAscendPlayerState::AAscendPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAscendAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	AttributeSet = CreateDefaultSubobject<UAscendAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAscendPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
