#include "AscendAttributeSet.h"

#include "Character/Base/AscendCharacterBase.h"
#include "GameplayEffectExtension.h"
#include "AscendGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAttributeSet)

UAscendAttributeSet::UAscendAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMana(100.0f);
	InitMaxMana(100.0f);
	InitAttackSpeed(1.0f);
	InitDamage(0.0f);
	InitIncomingDamage(0.0f);
	InitHealing(0.0f);
}

bool UAscendAttributeSet::TryGetAttributeForTag(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute)
{
	OutAttribute = FGameplayAttribute();

	if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_Health))
	{
		OutAttribute = GetHealthAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_MaxHealth))
	{
		OutAttribute = GetMaxHealthAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_Mana))
	{
		OutAttribute = GetManaAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_MaxMana))
	{
		OutAttribute = GetMaxManaAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_AttackSpeed))
	{
		OutAttribute = GetAttackSpeedAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_Damage))
	{
		OutAttribute = GetDamageAttribute();
	}
	else if (AttributeTag.MatchesTagExact(AscendGameplayTags::Attribute_Healing))
	{
		OutAttribute = GetHealingAttribute();
	}

	return OutAttribute.IsValid();
}

void UAscendAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
		return;
	}

	if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
		return;
	}
}

void UAscendAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FAscendAttributeSetExecutionData ExecutionData;
	GetExecutionDataFromMod(Data, ExecutionData);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamageAttribute(ExecutionData);
	}
	else if (Data.EvaluatedData.Attribute == GetHealingAttribute())
	{
		HandleHealingAttribute();
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		HandleHealthAttribute();
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		HandleManaAttribute();
	}
}

void UAscendAttributeSet::ClampAttributes(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::ClampAttributes(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
		return;
	}

	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
		return;
	}

	if (Attribute == GetAttackSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 10.0f);
		return;
	}

	if (Attribute == GetDamageAttribute() || Attribute == GetIncomingDamageAttribute() || Attribute == GetHealingAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

float UAscendAttributeSet::GetClampMinimumValueFor(const FGameplayAttribute& Attribute)
{
	if (Attribute == GetMaxHealthAttribute() || Attribute == GetMaxManaAttribute())
	{
		return 1.0f;
	}

	if (Attribute == GetAttackSpeedAttribute())
	{
		return 0.1f;
	}

	return 0.0f;
}

float UAscendAttributeSet::GetClampMaximumValueFor(const FGameplayAttribute& Attribute) const
{
	if (Attribute == GetHealthAttribute())
	{
		return GetMaxHealth();
	}

	if (Attribute == GetManaAttribute())
	{
		return GetMaxMana();
	}

	if (Attribute == GetAttackSpeedAttribute())
	{
		return 10.0f;
	}

	return Super::GetClampMaximumValueFor(Attribute);
}

void UAscendAttributeSet::SetAttributeClamped(const FGameplayAttribute& Attribute, float Value, float MaxValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const float Min = GetClampMinimumValueFor(Attribute);
	const float NewValue = FMath::Clamp(Value, Min, MaxValue);

	ASC->SetNumericAttributeBase(Attribute, NewValue);
}

void UAscendAttributeSet::HandleIncomingDamageAttribute(const FAscendAttributeSetExecutionData& ExecutionData)
{
	const float LocalDamageDone = GetIncomingDamage();
	SetIncomingDamage(0.0f);

	if (LocalDamageDone <= 0.0f)
	{
		return;
	}

	const float PreviousHealth = GetHealth();
	const float NewHealth = PreviousHealth - LocalDamageDone;
	SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
	HandleZeroHealthReached(ExecutionData, PreviousHealth);
}

void UAscendAttributeSet::HandleHealingAttribute()
{
	const float LocalHealingDone = GetHealing();
	SetHealing(0.0f);

	if (LocalHealingDone <= 0.0f)
	{
		return;
	}

	SetHealth(FMath::Clamp(GetHealth() + LocalHealingDone, 0.0f, GetMaxHealth()));
}

void UAscendAttributeSet::HandleHealthAttribute()
{
	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
}

void UAscendAttributeSet::HandleManaAttribute()
{
	SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
}

void UAscendAttributeSet::HandleZeroHealthReached(
	const FAscendAttributeSetExecutionData& ExecutionData,
	float PreviousHealth) const
{
	if (PreviousHealth <= 0.0f || GetHealth() > 0.0f)
	{
		return;
	}

	if (AAscendCharacterBase* TargetCharacter = Cast<AAscendCharacterBase>(ExecutionData.TargetActor))
	{
		TargetCharacter->HandleOutOfHealth(ExecutionData);
	}
}
