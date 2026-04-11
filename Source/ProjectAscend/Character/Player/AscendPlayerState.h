#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AscendPlayerState.generated.h"

/**
 * Player state that owns the player ASC and default attribute set.
 */
UCLASS()
class PROJECTASCEND_API AAscendPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AAscendPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	class UAscendAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Abilities")
	TObjectPtr<class UAscendAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<class UAscendAttributeSet> AttributeSet;
};
