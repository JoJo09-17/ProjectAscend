#include "AscendAbilitySet.h"
#include "AbilitySystemLog.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "AbilitySystem/Attributes/AscendAttributeSet.h"
#include "AbilitySystem/Attributes/AscendAttributeSetBase.h"
#include "AbilitySystem/Definition/AscendAbilityDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilitySet)

namespace
{
	UAttributeSet* GetOrCreateAttributeSetInstance(UAscendAbilitySystemComponent* ASC, TSubclassOf<UAttributeSet> AttributeSetClass, bool* bOutCreated = nullptr)
	{
		if (bOutCreated)
		{
			*bOutCreated = false;
		}

		if (!ASC || !AttributeSetClass)
		{
			return nullptr;
		}

		UAttributeSet* ExistingAttributeSet = ASC->FindSpawnedAttributeSet(AttributeSetClass);
		if (ExistingAttributeSet)
		{
			return ExistingAttributeSet;
		}

		UAttributeSet* CreatedAttributeSet = ASC->GetOrCreateSpawnedAttributeSet(AttributeSetClass);
		if (bOutCreated)
		{
			*bOutCreated = CreatedAttributeSet != nullptr;
		}

		return CreatedAttributeSet;
	}

	UAttributeSet* FindAttributeSetInstanceForAttribute(UAscendAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute)
	{
		if (!ASC || !Attribute.IsValid())
		{
			return nullptr;
		}

		UClass* AttributeSetClass = Attribute.GetAttributeSetClass();
		if (!AttributeSetClass)
		{
			return nullptr;
		}

		return GetOrCreateAttributeSetInstance(ASC, AttributeSetClass);
	}

	float ClampInitializerValue(UAttributeSet* AttributeSet, const FGameplayAttribute& Attribute, float Value)
	{
		if (const UAscendAttributeSetBase* AscendAttributeSet = Cast<UAscendAttributeSetBase>(AttributeSet))
		{
			const float MinValue = const_cast<UAscendAttributeSetBase*>(AscendAttributeSet)->GetClampMinimumValueFor(Attribute);
			const float MaxValue = AscendAttributeSet->GetClampMaximumValueFor(Attribute);
			return FMath::Clamp(Value, MinValue, MaxValue);
		}

		return Value;
	}
}

void FAscendAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAscendAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAscendAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	GrantedAttributeSets.Add(Set);
}

bool FAscendAbilitySet_GrantedHandles::HasAny() const
{
	return AbilitySpecHandles.Num() > 0 || GameplayEffectHandles.Num() > 0 || GrantedAttributeSets.Num() > 0;
}

bool FAscendAbilitySet_GrantedHandles::HasAbilityHandle(const FGameplayAbilitySpecHandle& Handle) const
{
	return AbilitySpecHandles.Contains(Handle);
}

int32 FAscendAbilitySet_GrantedHandles::GetAbilityHandleCount() const
{
	return AbilitySpecHandles.Num();
}

bool FAscendAbilitySet_GrantedHandles::GetFirstAbilityHandle(FGameplayAbilitySpecHandle& OutHandle) const
{
	OutHandle = AbilitySpecHandles.Num() > 0 ? AbilitySpecHandles[0] : FGameplayAbilitySpecHandle();
	return OutHandle.IsValid();
}

bool FAscendAbilitySet_GrantedHandles::GetLastAbilityHandle(FGameplayAbilitySpecHandle& OutHandle) const
{
	OutHandle = AbilitySpecHandles.Num() > 0 ? AbilitySpecHandles.Last() : FGameplayAbilitySpecHandle();
	return OutHandle.IsValid();
}

void FAscendAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAscendAbilitySystemComponent* AscendASC)
{
	check(AscendASC);

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			AscendASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			AscendASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		AscendASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UAscendAbilitySet::UAscendAbilitySet(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UAscendAbilitySet::HasAttributeInitializers() const
{
	return GrantedAttributeInitializers.Num() > 0;
}

void UAscendAbilitySet::GiveToAbilitySystem(UAscendAbilitySystemComponent* ASC,
	FAscendAbilitySet_GrantedHandles* OutGrantedHandles,
	UObject* SourceObject,
	bool bAutoAssignRuntimeSlots) const
{
	check(ASC);

	// Grant definition-based abilities ( AscendAbilityDefinition -> Ability spec with definition association ).
	for (int32 Index = 0; Index < GrantedGameplayAbilities.Num(); ++Index)
	{
		const UAscendAbilityDefinition* AbilityDefinition = GrantedGameplayAbilities[Index];
		if (!AbilityDefinition || !AbilityDefinition->Ability)
		{
			continue;
		}

		UAscendGameplayAbility* AbilityCDO = AbilityDefinition->Ability->GetDefaultObject<UAscendGameplayAbility>();
		if (!AbilityCDO)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityDefinition->DefaultLevel);

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAscendAbility(AbilitySpec, AbilityDefinition, SourceObject);
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	if (bAutoAssignRuntimeSlots)
	{
		ASC->AutoAssignDefaultSlotsForGrantedAbilities(false);
	}

	// Grant plain abilities (no definition association, input tag via dynamic spec tags).
	for (int32 Index = 0; Index < GrantedDefaultGameplayAbilities.Num(); ++Index)
	{
		const FAscendAbilitySet_GameplayAbility& AbilityToGrant = GrantedDefaultGameplayAbilities[Index];
		if (!IsValid(AbilityToGrant.Ability))
		{
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>();
		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;

		if (AbilityToGrant.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
		}

		const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	// Grant attribute sets before effects so attributes exist for modifier calculations.
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FAscendAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] is not valid"), SetIndex, *GetNameSafe(this));
			continue;
		}

		bool bCreatedAttributeSet = false;
		UAttributeSet* AttributeSetInstance = GetOrCreateAttributeSetInstance(ASC, SetToGrant.AttributeSet, &bCreatedAttributeSet);
		if (!AttributeSetInstance)
		{
			UE_LOG(LogAbilitySystem, Error, TEXT("GrantedAttributes[%d] on ability set [%s] could not create attribute set [%s]."), SetIndex, *GetNameSafe(this), *GetNameSafe(SetToGrant.AttributeSet));
			continue;
		}

		if (OutGrantedHandles && bCreatedAttributeSet)
		{
			OutGrantedHandles->AddAttributeSet(AttributeSetInstance);
		}
	}

	// AbilitySet-driven attribute initialization replaces the old startup GE path for simple base/current values.
	for (const FAscendAbilitySet_AttributeInitializer& AttributeInitializer : GrantedAttributeInitializers)
	{
		FGameplayAttribute Attribute;
		if (!UAscendAttributeSet::TryGetAttributeForTag(AttributeInitializer.AttributeTag, Attribute))
		{
			UE_LOG(
				LogAbilitySystem,
				Warning,
				TEXT("GrantedAttributeInitializers on ability set [%s] references unsupported attribute tag [%s]."),
				*GetNameSafe(this),
				*AttributeInitializer.AttributeTag.ToString());
			continue;
		}

		UAttributeSet* AttributeSetInstance = FindAttributeSetInstanceForAttribute(ASC, Attribute);
		if (!AttributeSetInstance)
		{
			UE_LOG(
				LogAbilitySystem,
				Warning,
				TEXT("GrantedAttributeInitializers on ability set [%s] could not find attribute set instance for [%s]."),
				*GetNameSafe(this),
				*Attribute.GetName());
			continue;
		}

		const float ClampedBaseValue = ClampInitializerValue(AttributeSetInstance, Attribute, AttributeInitializer.BaseValue);
		ASC->SetNumericAttributeBase(Attribute, ClampedBaseValue);
	}

	for (const FAscendAbilitySet_AttributeInitializer& AttributeInitializer : GrantedAttributeInitializers)
	{
		if (!AttributeInitializer.bOverrideCurrentValue)
		{
			continue;
		}

		FGameplayAttribute Attribute;
		if (!UAscendAttributeSet::TryGetAttributeForTag(AttributeInitializer.AttributeTag, Attribute))
		{
			continue;
		}

		UAttributeSet* AttributeSetInstance = FindAttributeSetInstanceForAttribute(ASC, Attribute);
		if (!AttributeSetInstance)
		{
			continue;
		}

		float CurrentValue = ClampInitializerValue(AttributeSetInstance, Attribute, AttributeInitializer.CurrentValue);
		Attribute.SetNumericValueChecked(CurrentValue, AttributeSetInstance);
	}

	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FAscendAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogAbilitySystem, Error, TEXT("GrantedGameplayEffects[%d] on ability set [%s] is not valid"), EffectIndex, *GetNameSafe(this));
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}

void UAscendAbilitySet::BP_GiveToAbilitySystem(UAscendAbilitySystemComponent* ASC,
	struct FAscendAbilitySet_GrantedHandles& OutGrantedHandles,
	UObject* SourceObject,
	bool bAutoAssignRuntimeSlots)
{
	GiveToAbilitySystem(ASC, &OutGrantedHandles, SourceObject, bAutoAssignRuntimeSlots);
}

void UAscendAbilitySet::BP_TakeFromAbilitySystem(UAscendAbilitySystemComponent* ASC,
	FAscendAbilitySet_GrantedHandles GrantedHandles)
{
	GrantedHandles.TakeFromAbilitySystem(ASC);
}
