#include "AbilitySystem/AscendAbilityQueryLibrary.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Definition/AscendAbilityDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilityQueryLibrary)

bool UAscendAbilityQueryLibrary::IsAbilityHandleValid(FGameplayAbilitySpecHandle AbilityHandle)
{
	return AbilityHandle.IsValid();
}

bool UAscendAbilityQueryLibrary::GrantedHandlesHasAny(const FAscendAbilitySet_GrantedHandles& GrantedHandles)
{
	return GrantedHandles.HasAny();
}

int32 UAscendAbilityQueryLibrary::GetGrantedAbilityHandleCount(const FAscendAbilitySet_GrantedHandles& GrantedHandles)
{
	return GrantedHandles.GetAbilityHandleCount();
}

bool UAscendAbilityQueryLibrary::GrantedHandlesContainsAbilityHandle(
	const FAscendAbilitySet_GrantedHandles& GrantedHandles,
	FGameplayAbilitySpecHandle AbilityHandle)
{
	return GrantedHandles.HasAbilityHandle(AbilityHandle);
}

bool UAscendAbilityQueryLibrary::GetFirstGrantedAbilityHandle(
	const FAscendAbilitySet_GrantedHandles& GrantedHandles,
	FGameplayAbilitySpecHandle& OutHandle)
{
	return GrantedHandles.GetFirstAbilityHandle(OutHandle);
}

bool UAscendAbilityQueryLibrary::GetLastGrantedAbilityHandle(
	const FAscendAbilitySet_GrantedHandles& GrantedHandles,
	FGameplayAbilitySpecHandle& OutHandle)
{
	return GrantedHandles.GetLastAbilityHandle(OutHandle);
}

UAscendAbilityDefinition* UAscendAbilityQueryLibrary::GetAbilityDefinition(
	const UAscendAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle)
{
	return ASC ? const_cast<UAscendAbilityDefinition*>(ASC->GetDefinitionForHandle(AbilityHandle)) : nullptr;
}

UObject* UAscendAbilityQueryLibrary::GetAbilitySourceObject(
	const UAscendAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle)
{
	return ASC ? ASC->GetSourceObjectForHandle(AbilityHandle) : nullptr;
}

bool UAscendAbilityQueryLibrary::GetAbilityInputTags(
	const UAscendAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTagContainer& OutInputTags)
{
	OutInputTags.Reset();
	return ASC ? ASC->GetDynamicInputTagsForAbility(AbilityHandle, OutInputTags) : false;
}

bool UAscendAbilityQueryLibrary::GetAbilitySlot(
	const UAscendAbilitySystemComponent* ASC,
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTag& OutSlotInputTag)
{
	OutSlotInputTag = FGameplayTag();
	return ASC ? ASC->GetSlotForAbility(AbilityHandle, OutSlotInputTag) : false;
}

bool UAscendAbilityQueryLibrary::FindAbilityHandleByTag(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTag& AbilityTag,
	FGameplayAbilitySpecHandle& OutAbilityHandle)
{
	OutAbilityHandle = FGameplayAbilitySpecHandle();
	return ASC ? ASC->FindFirstAbilityHandleByTag(AbilityTag, OutAbilityHandle) : false;
}

bool UAscendAbilityQueryLibrary::FindAbilityHandleByTags(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	FGameplayAbilitySpecHandle& OutAbilityHandle)
{
	OutAbilityHandle = FGameplayAbilitySpecHandle();
	return ASC ? ASC->FindFirstAbilityHandleByTags(AbilityTags, bRequireAllTags, OutAbilityHandle) : false;
}

int32 UAscendAbilityQueryLibrary::FindAbilityHandlesByTags(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles)
{
	OutAbilityHandles.Reset();
	return ASC ? ASC->FindAbilityHandlesByTags(AbilityTags, bRequireAllTags, OutAbilityHandles) : 0;
}

bool UAscendAbilityQueryLibrary::FindAbilitySpecByTag(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTag& AbilityTag,
	FGameplayAbilitySpec& OutAbilitySpec)
{
	FGameplayAbilitySpecHandle AbilityHandle;
	return ASC
		&& ASC->FindFirstAbilityHandleByTag(AbilityTag, AbilityHandle)
		&& ASC->GetAbilitySpecCopy(AbilityHandle, OutAbilitySpec);
}

bool UAscendAbilityQueryLibrary::FindAbilitySpecByTags(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	FGameplayAbilitySpec& OutAbilitySpec)
{
	FGameplayAbilitySpecHandle AbilityHandle;
	return ASC
		&& ASC->FindFirstAbilityHandleByTags(AbilityTags, bRequireAllTags, AbilityHandle)
		&& ASC->GetAbilitySpecCopy(AbilityHandle, OutAbilitySpec);
}

int32 UAscendAbilityQueryLibrary::FindAbilitySpecsByTags(
	const UAscendAbilitySystemComponent* ASC,
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	TArray<FGameplayAbilitySpec>& OutAbilitySpecs)
{
	OutAbilitySpecs.Reset();
	if (!ASC)
	{
		return 0;
	}

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	ASC->FindAbilityHandlesByTags(AbilityTags, bRequireAllTags, MatchingHandles);

	for (const FGameplayAbilitySpecHandle& AbilityHandle : MatchingHandles)
	{
		FGameplayAbilitySpec AbilitySpec;
		if (ASC->GetAbilitySpecCopy(AbilityHandle, AbilitySpec))
		{
			OutAbilitySpecs.Add(MoveTemp(AbilitySpec));
		}
	}

	return OutAbilitySpecs.Num();
}
