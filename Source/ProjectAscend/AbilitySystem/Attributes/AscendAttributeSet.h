#pragma once

#include "CoreMinimal.h"

#include "AbilitySystemComponent.h"
#include "AscendAttributeSetBase.h"
#include "AscendAttributeSet.generated.h"


/**
 * Core attribute set for the Ascend ARPG framework.
 * Defines vital stats and combat-scaling attributes used by abilities and effects.
 */
UCLASS()
class PROJECTASCEND_API UAscendAttributeSet : public UAscendAttributeSetBase
{
	GENERATED_BODY()

public:
	UAscendAttributeSet();

	/** Resolves a gameplay attribute from the project-level attribute tag mapping. */
	static bool TryGetAttributeForTag(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute);

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, MaxMana)

	/** Scales animation play rate and movement speed for attack abilities (1.0 = base speed). */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, AttackSpeed)
	
	/** Outgoing damage stat captured by damage executions. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, Damage)

	/** Meta attribute that carries resolved incoming damage into PostGameplayEffectExecute. */
	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, IncomingDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Ascend|Attributes")
	FGameplayAttributeData Healing;
	ATTRIBUTE_ACCESSORS(UAscendAttributeSet, Healing)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	virtual void ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual float GetClampMinimumValueFor(const FGameplayAttribute& Attribute) override;
	virtual float GetClampMaximumValueFor(const FGameplayAttribute& Attribute) const override;

	void SetAttributeClamped(const FGameplayAttribute& Attribute, float Value, float MaxValue);
	void HandleIncomingDamageAttribute(const FAscendAttributeSetExecutionData& ExecutionData);
	void HandleHealingAttribute();
	void HandleHealthAttribute();
	void HandleManaAttribute();
	void HandleZeroHealthReached(const FAscendAttributeSetExecutionData& ExecutionData, float PreviousHealth) const;
};
