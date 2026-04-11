#include "UI/AscendAbilitySystemWidget.h"

#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilitySystemWidget)

bool UAscendAbilitySystemWidget::InitializeAbilitySystemWidget(UAbilitySystemComponent* InAbilitySystemComponent)
{
	UAscendAbilitySystemComponent* NewASC = Cast<UAscendAbilitySystemComponent>(InAbilitySystemComponent);
	UnbindFromCurrentASC();
	AbilitySystemComponent = NewASC;

	if (!AbilitySystemComponent.IsValid())
	{
		K2_InitializeAbilitySystemWidget(false);
		return false;
	}

	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UAscendAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);
	MaxHealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UAscendAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleMaxHealthChanged);
	ManaChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UAscendAttributeSet::GetManaAttribute()).AddUObject(this, &ThisClass::HandleManaChanged);
	MaxManaChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UAscendAttributeSet::GetMaxManaAttribute()).AddUObject(this, &ThisClass::HandleMaxManaChanged);
	AttackSpeedChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		UAscendAttributeSet::GetAttackSpeedAttribute()).AddUObject(this, &ThisClass::HandleAttackSpeedChanged);

	AbilitySystemComponent->OnAbilitySlotChanged.AddDynamic(this, &ThisClass::HandleAbilitySlotChangedEvent);

	BroadcastCurrentAttributeValues();
	K2_InitializeAbilitySystemWidget(true);
	return true;
}

UAscendAbilitySystemComponent* UAscendAbilitySystemWidget::GetOwnerAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

float UAscendAbilitySystemWidget::GetCurrentHealth() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(UAscendAttributeSet::GetHealthAttribute())
		: 0.0f;
}

float UAscendAbilitySystemWidget::GetMaxHealth() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(UAscendAttributeSet::GetMaxHealthAttribute())
		: 0.0f;
}

float UAscendAbilitySystemWidget::GetCurrentMana() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(UAscendAttributeSet::GetManaAttribute())
		: 0.0f;
}

float UAscendAbilitySystemWidget::GetMaxMana() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(UAscendAttributeSet::GetMaxManaAttribute())
		: 0.0f;
}

float UAscendAbilitySystemWidget::GetAttackSpeed() const
{
	return AbilitySystemComponent.IsValid()
		? AbilitySystemComponent->GetNumericAttribute(UAscendAttributeSet::GetAttackSpeedAttribute())
		: 0.0f;
}

void UAscendAbilitySystemWidget::NativeDestruct()
{
	UnbindFromCurrentASC();
	Super::NativeDestruct();
}

void UAscendAbilitySystemWidget::UnbindFromCurrentASC()
{
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAscendAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAscendAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAscendAttributeSet::GetManaAttribute()).Remove(ManaChangedHandle);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAscendAttributeSet::GetMaxManaAttribute()).Remove(MaxManaChangedHandle);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAscendAttributeSet::GetAttackSpeedAttribute()).Remove(AttackSpeedChangedHandle);
	AbilitySystemComponent->OnAbilitySlotChanged.RemoveDynamic(this, &ThisClass::HandleAbilitySlotChangedEvent);

	HealthChangedHandle.Reset();
	MaxHealthChangedHandle.Reset();
	ManaChangedHandle.Reset();
	MaxManaChangedHandle.Reset();
	AttackSpeedChangedHandle.Reset();
	AbilitySystemComponent.Reset();
}

void UAscendAbilitySystemWidget::BroadcastCurrentAttributeValues() const
{
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	const float CurrentHealth = GetCurrentHealth();
	const float CurrentMaxHealth = GetMaxHealth();
	const float CurrentMana = GetCurrentMana();
	const float CurrentMaxMana = GetMaxMana();

	const_cast<UAscendAbilitySystemWidget*>(this)->OnMaxHealthChanged(CurrentMaxHealth, 0.0f, GetSafePercentage(CurrentHealth, CurrentMaxHealth));
	const_cast<UAscendAbilitySystemWidget*>(this)->OnHealthChanged(CurrentHealth, 0.0f, GetSafePercentage(CurrentHealth, CurrentMaxHealth));
	const_cast<UAscendAbilitySystemWidget*>(this)->OnMaxManaChanged(CurrentMaxMana, 0.0f, GetSafePercentage(CurrentMana, CurrentMaxMana));
	const_cast<UAscendAbilitySystemWidget*>(this)->OnManaChanged(CurrentMana, 0.0f, GetSafePercentage(CurrentMana, CurrentMaxMana));
	const_cast<UAscendAbilitySystemWidget*>(this)->OnAttackSpeedChanged(GetAttackSpeed(), 0.0f);

	TArray<FGameplayTag> SlotTags;
	AbilitySystemComponent->GetSupportedSlotTags(SlotTags);
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		FGameplayAbilitySpecHandle AbilityHandle;
		const bool bOccupied = AbilitySystemComponent->GetAbilityHandleForSlot(SlotTag, AbilityHandle);
		const_cast<UAscendAbilitySystemWidget*>(this)->OnAbilitySlotChanged(SlotTag, AbilityHandle, bOccupied);
	}
}

void UAscendAbilitySystemWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged(Data.NewValue, Data.OldValue, GetSafePercentage(Data.NewValue, GetMaxHealth()));
}

void UAscendAbilitySystemWidget::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChanged(Data.NewValue, Data.OldValue, GetSafePercentage(GetCurrentHealth(), Data.NewValue));
}

void UAscendAbilitySystemWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	OnManaChanged(Data.NewValue, Data.OldValue, GetSafePercentage(Data.NewValue, GetMaxMana()));
}

void UAscendAbilitySystemWidget::HandleMaxManaChanged(const FOnAttributeChangeData& Data)
{
	OnMaxManaChanged(Data.NewValue, Data.OldValue, GetSafePercentage(GetCurrentMana(), Data.NewValue));
}

void UAscendAbilitySystemWidget::HandleAttackSpeedChanged(const FOnAttributeChangeData& Data)
{
	OnAttackSpeedChanged(Data.NewValue, Data.OldValue);
}

void UAscendAbilitySystemWidget::HandleAbilitySlotChangedEvent(
	const FGameplayTag& SlotTag,
	FGameplayAbilitySpecHandle AbilityHandle,
	bool bOccupied)
{
	OnAbilitySlotChanged(SlotTag, AbilityHandle, bOccupied);
}

float UAscendAbilitySystemWidget::GetSafePercentage(float CurrentValue, float MaxValue) const
{
	return MaxValue > 0.0f ? CurrentValue / MaxValue : 0.0f;
}
