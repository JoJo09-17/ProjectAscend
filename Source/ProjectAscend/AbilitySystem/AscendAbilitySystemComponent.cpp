#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AscendLogChannels.h"
#include "Abilities/AscendGameplayAbility.h"
#include "AbilitySystem/Data/AscendAbilityTagRelationshipMapping.h"
#include "AbilitySystem/Definition/AscendAbilityDefinition.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_InputBinding.h"
#include "Player/AscendAbilitySlotComponent.h"
#include "AscendGameplayTags.h"
#include "Settings/AscendAbilitySystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendAbilitySystemComponent)

UAscendAbilitySystemComponent::UAscendAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UAscendAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAscendAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAbilityInput();
	Super::EndPlay(EndPlayReason);
}

void UAscendAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (IsAbilityInputBlocked())
	{
		ClearAbilityInput();
		return;
	}

	TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reserve(InputPressedSpecHandles.Num() + InputHeldSpecHandles.Num());

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = true;

		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputPressed(*AbilitySpec);
			continue;
		}

		// Abilities marked as confirm/cancel short-circuit into the targeting system
		// instead of activating normally while a targeting confirmation is pending.
		if (HasPendingConfirm())
		{
			const UAscendAbilityDefinition* Def = GetDefinitionForHandle(SpecHandle);
			const FAscendAbilityFragment_InputBinding* InputFrag = Def ? Def->GetInputBindingFragment() : nullptr;

			if (InputFrag && InputFrag->bActAsConfirm)
			{
				HandleConfirmInput();
				continue;
			}

			if (InputFrag && InputFrag->bActAsCancel)
			{
				HandleCancelInput();
				continue;
			}
		}

		const EAscendAbilityActivationPolicy Policy = GetActivationPolicyForSpec(*AbilitySpec);
		if (Policy == EAscendAbilityActivationPolicy::OnInputTriggered ||
			Policy == EAscendAbilityActivationPolicy::WhileInputActive)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	// WhileInputActive abilities activate every frame the input is held.
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability || AbilitySpec->IsActive())
		{
			continue;
		}

		if (GetActivationPolicyForSpec(*AbilitySpec) == EAscendAbilityActivationPolicy::WhileInputActive)
		{
			AbilitiesToActivate.AddUnique(SpecHandle);
		}
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToActivate)
	{
		TryActivateAbility(Handle);
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle);
		if (!AbilitySpec || !AbilitySpec->Ability)
		{
			continue;
		}

		AbilitySpec->InputPressed = false;

		if (AbilitySpec->IsActive())
		{
			AbilitySpecInputReleased(*AbilitySpec);
		}
	}

	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UAscendAbilitySystemComponent::ClearAbilityInput()
{
	TArray<FGameplayAbilitySpecHandle> HandlesToReset;
	HandlesToReset.Append(InputPressedSpecHandles);
	HandlesToReset.Append(InputHeldSpecHandles);
	HandlesToReset.Append(InputReleasedSpecHandles);

	for (const FGameplayAbilitySpecHandle& Handle : HandlesToReset)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			Spec->InputPressed = false;
		}
	}

	InputPressedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UAscendAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || IsAbilityInputBlocked())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (AbilitySpecHasInputTag(AbilitySpec, InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UAscendAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || IsAbilityInputBlocked())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability)
		{
			continue;
		}

		if (AbilitySpecHasInputTag(AbilitySpec, InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UAscendAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		const FPredictionKey OriginalPredictionKey =
			Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey()
					 : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, OriginalPredictionKey);
	}
}

void UAscendAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		const UGameplayAbility* Instance = Spec.GetPrimaryInstance();
		const FPredictionKey OriginalPredictionKey =
			Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey()
					 : Spec.ActivationInfo.GetActivationPredictionKey();
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, OriginalPredictionKey);
	}
}

void UAscendAbilitySystemComponent::HandleConfirmInput()
{
	LocalInputConfirm();
	NotifyConfirmComplete();
}

void UAscendAbilitySystemComponent::HandleCancelInput()
{
	LocalInputCancel();
	NotifyConfirmComplete();
}

void UAscendAbilitySystemComponent::NotifyWaitingForConfirm()
{
	PendingConfirmCount++;
}

void UAscendAbilitySystemComponent::NotifyConfirmComplete()
{
	PendingConfirmCount = FMath::Max(0, PendingConfirmCount - 1);
}

bool UAscendAbilitySystemComponent::HasPendingConfirm() const
{
	return PendingConfirmCount > 0;
}

bool UAscendAbilitySystemComponent::IsAbilityInputBlocked() const
{
	FGameplayTagContainer TagsToCheck = InputBlockedTags;
	if (TagsToCheck.IsEmpty())
	{
		TagsToCheck.AddTag(AscendGameplayTags::State_InputBlocked_Ability);
	}

	return HasAnyMatchingGameplayTags(TagsToCheck);
}

bool UAscendAbilitySystemComponent::IsActivationGroupBlocked(EAscendAbilityActivationGroup Group) const
{
	switch (Group)
	{
	case EAscendAbilityActivationGroup::Independent:
		return false;

	case EAscendAbilityActivationGroup::ExclusiveReplaceable:
	case EAscendAbilityActivationGroup::ExclusiveBlocking:
		return ActivationGroupCounts[(uint8)EAscendAbilityActivationGroup::ExclusiveBlocking] > 0;

	default:
		return false;
	}
}

void UAscendAbilitySystemComponent::GetSupportedSlotTags(TArray<FGameplayTag>& OutSlotTags) const
{
	OutSlotTags.Reset();

	FGameplayTagContainer EffectiveSlotTags;
	GetEffectiveSupportedSlotTags(EffectiveSlotTags);
	EffectiveSlotTags.GetGameplayTagArray(OutSlotTags);
}

FGameplayAbilitySpecHandle UAscendAbilitySystemComponent::K2_GiveAscendAbility(
	const UAscendAbilityDefinition* Definition,
	UObject* SourceObject)
{
	if (!Definition || !Definition->Ability) 
	{
		return FGameplayAbilitySpecHandle();
	}
	FGameplayAbilitySpec Spec(Definition->Ability->GetDefaultObject<UGameplayAbility>(), Definition->DefaultLevel);
    
	return GiveAscendAbility(Spec, Definition, SourceObject);
}

FGameplayAbilitySpecHandle UAscendAbilitySystemComponent::GiveAscendAbility(
	FGameplayAbilitySpec& AbilitySpec,
	const UAscendAbilityDefinition* AbilityDefinition,
	UObject* SourceObject)
{
	if (!AbilityDefinition || !AbilitySpec.Ability)
	{
		UE_LOG(LogAscendAbilitySystem, Error, TEXT("GiveAscendAbility failed: invalid AbilityDefinition or AbilitySpec.Ability"));
		return FGameplayAbilitySpecHandle();
	}

	ApplyAbilityDefinitionToSpec(AbilitySpec, AbilityDefinition);
	AbilitySpec.SourceObject = SourceObject;

	const FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);
	if (!Handle.IsValid())
	{
		UE_LOG(LogAscendAbilitySystem, Error, TEXT("GiveAbility failed on ASC [%s]"), *GetNameSafe(this));
		return Handle;
	}

	AbilityDefinitionMap.Add(Handle, const_cast<UAscendAbilityDefinition*>(AbilityDefinition));
	AbilitySemanticTagMap.Add(Handle, AbilityDefinition->AbilityTags);
	if (SourceObject)
	{
		AbilitySourceObjectMap.Add(Handle, SourceObject);
	}

	if (const FGameplayAbilitySpec* GrantedSpec = FindAbilitySpecFromHandle(Handle))
	{
		if (GetActivationPolicyForSpec(*GrantedSpec) == EAscendAbilityActivationPolicy::OnSpawn)
		{
			TryActivateAbility(Handle);
		}
	}

	return Handle;
}

const UAscendAbilityDefinition* UAscendAbilitySystemComponent::GetDefinitionForHandle(FGameplayAbilitySpecHandle Handle) const
{
	const TObjectPtr<UAscendAbilityDefinition>* Found = AbilityDefinitionMap.Find(Handle);
	if (Found)
	{
		return Found->Get();
	}

	if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
	{
		if (const UAscendAbilityDefinition* Definition = Cast<UAscendAbilityDefinition>(Spec->SourceObject.Get()))
		{
			return Definition;
		}
	}

	return nullptr;
}

UObject* UAscendAbilitySystemComponent::GetSourceObjectForHandle(FGameplayAbilitySpecHandle Handle) const
{
	const TObjectPtr<UObject>* Found = AbilitySourceObjectMap.Find(Handle);
	if (Found)
	{
		return Found->Get();
	}

	if (const FGameplayAbilitySpec* Spec = GetAbilitySpecByHandle(Handle))
	{
		return Spec->SourceObject.Get();
	}

	return nullptr;
}

bool UAscendAbilitySystemComponent::HasAbilityHandle(FGameplayAbilitySpecHandle AbilityHandle) const
{
	return GetAbilitySpecByHandle(AbilityHandle) != nullptr;
}

bool UAscendAbilitySystemComponent::GetDynamicInputTagsForAbility(
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTagContainer& OutInputTags) const
{
	OutInputTags.Reset();

	const FGameplayAbilitySpec* Spec = GetAbilitySpecByHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	OutInputTags.AppendTags(Spec->GetDynamicSpecSourceTags());
	if (const FGameplayTag* AssignedSlot = AbilitySlotMap.Find(AbilityHandle))
	{
		OutInputTags.AddTag(*AssignedSlot);
	}

	return !OutInputTags.IsEmpty();
}

bool UAscendAbilitySystemComponent::HasInputTagForAbility(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& InputTag) const
{
	const FGameplayAbilitySpec* Spec = GetAbilitySpecByHandle(AbilityHandle);
	return InputTag.IsValid() && Spec && AbilitySpecHasInputTag(*Spec, InputTag);
}

bool UAscendAbilitySystemComponent::GetAbilityTagsForAbility(
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTagContainer& OutAbilityTags) const
{
	OutAbilityTags.Reset();

	const FGameplayAbilitySpec* Spec = GetAbilitySpecByHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	GetAbilityTagsForSpec(*Spec, OutAbilityTags);
	return !OutAbilityTags.IsEmpty();
}

bool UAscendAbilitySystemComponent::GetAbilitySpecCopy(
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayAbilitySpec& OutAbilitySpec) const
{
	const FGameplayAbilitySpec* Spec = GetAbilitySpecByHandle(AbilityHandle);
	if (!Spec)
	{
		return false;
	}

	OutAbilitySpec = *Spec;
	return true;
}

bool UAscendAbilitySystemComponent::FindFirstAbilityHandleByTag(
	const FGameplayTag& AbilityTag,
	FGameplayAbilitySpecHandle& OutAbilityHandle) const
{
	OutAbilityHandle = FGameplayAbilitySpecHandle();

	if (!AbilityTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer QueryTags;
	QueryTags.AddTag(AbilityTag);
	return FindFirstAbilityHandleByTags(QueryTags, false, OutAbilityHandle);
}

bool UAscendAbilitySystemComponent::FindFirstAbilityHandleByTags(
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	FGameplayAbilitySpecHandle& OutAbilityHandle) const
{
	OutAbilityHandle = FGameplayAbilitySpecHandle();

	TArray<FGameplayAbilitySpecHandle> MatchingHandles;
	if (FindAbilityHandlesByTags(AbilityTags, bRequireAllTags, MatchingHandles) <= 0)
	{
		return false;
	}

	OutAbilityHandle = MatchingHandles[0];
	return true;
}

int32 UAscendAbilitySystemComponent::FindAbilityHandlesByTags(
	const FGameplayTagContainer& AbilityTags,
	bool bRequireAllTags,
	TArray<FGameplayAbilitySpecHandle>& OutAbilityHandles) const
{
	OutAbilityHandles.Reset();

	if (AbilityTags.IsEmpty())
	{
		return 0;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.Ability || !Spec.Handle.IsValid())
		{
			continue;
		}

		FGameplayTagContainer SpecAbilityTags;
		GetAbilityTagsForSpec(Spec, SpecAbilityTags);

		const bool bMatches = bRequireAllTags
			? SpecAbilityTags.HasAll(AbilityTags)
			: SpecAbilityTags.HasAny(AbilityTags);
		if (bMatches)
		{
			OutAbilityHandles.Add(Spec.Handle);
		}
	}

	return OutAbilityHandles.Num();
}

bool UAscendAbilitySystemComponent::HasAbilityTag(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& AbilityTag) const
{
	if (!AbilityTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer AbilityTags;
	return GetAbilityTagsForAbility(AbilityHandle, AbilityTags) && AbilityTags.HasTagExact(AbilityTag);
}

bool UAscendAbilitySystemComponent::GetOwnedTagsForAbility(
	FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTagContainer& OutOwnedTags) const
{
	OutOwnedTags.Reset();

	const UAscendAbilityDefinition* Definition = GetDefinitionForHandle(AbilityHandle);
	if (!Definition)
	{
		return false;
	}

	OutOwnedTags.AppendTags(Definition->OwnedTags);
	return !OutOwnedTags.IsEmpty();
}

bool UAscendAbilitySystemComponent::GetRelationshipActivationTagRequirements(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutRequiredTags,
	FGameplayTagContainer& OutBlockedTags) const
{
	OutRequiredTags.Reset();
	OutBlockedTags.Reset();

	const UAscendAbilityTagRelationshipMapping* Mapping = GetAbilityTagRelationshipMapping();
	if (!Mapping)
	{
		return false;
	}

	FGameplayTagContainer OwnerTags;
	GetOwnedGameplayTags(OwnerTags);
	Mapping->GetActivationRequiredAndBlockedTagsForOwner(OwnerTags, AbilityTags, OutRequiredTags, OutBlockedTags);
	return true;
}

void UAscendAbilitySystemComponent::CancelAbilitiesByAbilityTags(
	const FGameplayTagContainer& WithTags,
	const FGameplayTagContainer& WithoutTags,
	UGameplayAbility* Ignore)
{
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.IsActive() || !Spec.Ability)
		{
			continue;
		}

		FGameplayTagContainer AbilityTags;
		GetAbilityTagsForSpec(Spec, AbilityTags);

		const bool bWithTagPass = WithTags.IsEmpty() || AbilityTags.HasAny(WithTags);
		const bool bWithoutTagPass = WithoutTags.IsEmpty() || !AbilityTags.HasAny(WithoutTags);
		if (bWithTagPass && bWithoutTagPass)
		{
			CancelAbilitySpec(Spec, Ignore);
		}
	}
}

bool UAscendAbilitySystemComponent::AssignAbilityToSlot(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& SlotInputTag,
	bool bReplaceExisting)
{
	if (!SlotInputTag.IsValid() || !IsSupportedSlotInputTag(SlotInputTag))
	{
		return false;
	}

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		UE_LOG(LogAscendAbilitySystem, Error, TEXT("AssignAbilityToSlot failed: invalid spec for handle [%s]"), *AbilityHandle.ToString());
		return false;
	}

	const FAscendAbilityFragment_InputBinding* InputFragment = GetSlotBindingFragmentForAbility(AbilityHandle);
	if (!InputFragment)
	{
		UE_LOG(
			LogAscendAbilitySystem,
			Warning,
			TEXT("AssignAbilityToSlot failed: ability [%s] does not provide an InputBinding fragment with runtime slot binding enabled."),
			*AbilityHandle.ToString());
		return false;
	}

	FGameplayAbilitySpecHandle ExistingHandle;
	if (GetAbilityHandleForSlot(SlotInputTag, ExistingHandle))
	{
		if (ExistingHandle == AbilityHandle)
		{
			return true;
		}

		if (!bReplaceExisting)
		{
			return false;
		}

		ClearAbilitySlot(SlotInputTag);
	}

	AbilitySlotMap.Add(AbilityHandle, SlotInputTag);
	ResetInputStateForAbility(AbilityHandle);
	BroadcastSlotChanged(SlotInputTag);
	return true;
}

bool UAscendAbilitySystemComponent::ClearAbilitySlot(const FGameplayTag& SlotInputTag)
{
	if (!SlotInputTag.IsValid())
	{
		return false;
	}

	bool bRemovedAny = false;

	TArray<FGameplayAbilitySpecHandle> HandlesToClear;
	for (const TPair<FGameplayAbilitySpecHandle, FGameplayTag>& Pair : AbilitySlotMap)
	{
		if (Pair.Value.MatchesTagExact(SlotInputTag))
		{
			HandlesToClear.Add(Pair.Key);
		}
	}

	for (const FGameplayAbilitySpecHandle Handle : HandlesToClear)
	{
		AbilitySlotMap.Remove(Handle);
		ResetInputStateForAbility(Handle);
		BroadcastSlotChanged(SlotInputTag);
		bRemovedAny = true;
	}

	return bRemovedAny;
}


bool UAscendAbilitySystemComponent::AddDynamicInputTagToAbility(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	Spec->GetDynamicSpecSourceTags().AddTag(InputTag);
	MarkAbilitySpecDirty(*Spec);
	return true;
}

bool UAscendAbilitySystemComponent::RemoveDynamicInputTagFromAbility(FGameplayAbilitySpecHandle AbilityHandle, const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	Spec->GetDynamicSpecSourceTags().RemoveTag(InputTag);
	MarkAbilitySpecDirty(*Spec);
	ResetInputStateForAbility(AbilityHandle);
	return true;
}

bool UAscendAbilitySystemComponent::IsSupportedSlotInputTag(const FGameplayTag& SlotInputTag) const
{
	if (!SlotInputTag.IsValid())
	{
		return false;
	}

	FGameplayTagContainer EffectiveSlotTags;
	GetEffectiveSupportedSlotTags(EffectiveSlotTags);
	return EffectiveSlotTags.HasTagExact(SlotInputTag);
}

bool UAscendAbilitySystemComponent::GetAbilityHandleForSlot(const FGameplayTag& SlotInputTag,
	FGameplayAbilitySpecHandle& OutHandle) const
{
	OutHandle = FGameplayAbilitySpecHandle();

	if (!SlotInputTag.IsValid())
	{
		return false;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (const FGameplayTag* AssignedSlot = AbilitySlotMap.Find(Spec.Handle);
			AssignedSlot && AssignedSlot->MatchesTagExact(SlotInputTag))
		{
			OutHandle = Spec.Handle;
			return true;
		}
	}

	return false;
}

bool UAscendAbilitySystemComponent::GetSlotForAbility(FGameplayAbilitySpecHandle AbilityHandle,
	FGameplayTag& OutSlotInputTag) const
{
	OutSlotInputTag = FGameplayTag();

	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityHandle);
	if (!Spec || !Spec->Ability)
	{
		return false;
	}

	const FGameplayTag SlotTag = GetSlotInputTagFromSpec(*Spec);
	if (!SlotTag.IsValid())
	{
		return false;
	}

	OutSlotInputTag = SlotTag;
	return true;
}

bool UAscendAbilitySystemComponent::IsAbilityAssignedToSlot(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& SlotInputTag) const
{
	if (!SlotInputTag.IsValid())
	{
		return false;
	}

	FGameplayTag AssignedSlot;
	return GetSlotForAbility(AbilityHandle, AssignedSlot) && AssignedSlot.MatchesTagExact(SlotInputTag);
}

const UAscendAbilityDefinition* UAscendAbilitySystemComponent::GetDefinitionForSlot(
	const FGameplayTag& SlotInputTag) const
{
	FGameplayAbilitySpecHandle Handle;
	if (!GetAbilityHandleForSlot(SlotInputTag, Handle))
	{
		return nullptr;
	}

	return GetDefinitionForHandle(Handle);
}

UObject* UAscendAbilitySystemComponent::GetSourceObjectForSlot(const FGameplayTag& SlotInputTag) const
{
	FGameplayAbilitySpecHandle Handle;
	if (!GetAbilityHandleForSlot(SlotInputTag, Handle))
	{
		return nullptr;
	}

	return GetSourceObjectForHandle(Handle);
}

bool UAscendAbilitySystemComponent::IsSlotOccupied(const FGameplayTag& SlotInputTag) const
{
	FGameplayAbilitySpecHandle Handle;
	return GetAbilityHandleForSlot(SlotInputTag, Handle);
}

bool UAscendAbilitySystemComponent::MoveAbilityToSlot(
	FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayTag& NewSlotInputTag,
	bool bSwapIfOccupied)
{
	if (!IsSupportedSlotInputTag(NewSlotInputTag))
	{
		return false;
	}

	FGameplayTag CurrentSlot;
	const bool bHasCurrentSlot = GetSlotForAbility(AbilityHandle, CurrentSlot);
	if (bHasCurrentSlot && CurrentSlot.MatchesTagExact(NewSlotInputTag))
	{
		return true;
	}

	FGameplayAbilitySpecHandle ExistingHandle;
	const bool bTargetOccupied = GetAbilityHandleForSlot(NewSlotInputTag, ExistingHandle);
	if (!bTargetOccupied)
	{
		return AssignAbilityToSlot(AbilityHandle, NewSlotInputTag, true);
	}

	if (!bSwapIfOccupied || !bHasCurrentSlot)
	{
		return AssignAbilityToSlot(AbilityHandle, NewSlotInputTag, false);
	}

	if (!AssignAbilityToSlot(AbilityHandle, NewSlotInputTag, true))
	{
		return false;
	}

	return AssignAbilityToSlot(ExistingHandle, CurrentSlot, true);
}

bool UAscendAbilitySystemComponent::SwapAbilitySlots(
	const FGameplayTag& FirstSlotInputTag,
	const FGameplayTag& SecondSlotInputTag)
{
	if (!FirstSlotInputTag.IsValid() || !SecondSlotInputTag.IsValid())
	{
		return false;
	}

	if (FirstSlotInputTag.MatchesTagExact(SecondSlotInputTag))
	{
		return true;
	}

	FGameplayAbilitySpecHandle FirstHandle;
	FGameplayAbilitySpecHandle SecondHandle;
	const bool bHasFirst = GetAbilityHandleForSlot(FirstSlotInputTag, FirstHandle);
	const bool bHasSecond = GetAbilityHandleForSlot(SecondSlotInputTag, SecondHandle);

	if (!bHasFirst && !bHasSecond)
	{
		return false;
	}

	if (bHasFirst && !bHasSecond)
	{
		return MoveAbilityToSlot(FirstHandle, SecondSlotInputTag, false);
	}

	if (!bHasFirst && bHasSecond)
	{
		return MoveAbilityToSlot(SecondHandle, FirstSlotInputTag, false);
	}

	// Clear both slots first so the second clear doesn't interfere with the first assignment.
	if (!ClearAbilitySlot(FirstSlotInputTag))
	{
		return false;
	}

	if (!ClearAbilitySlot(SecondSlotInputTag))
	{
		AssignAbilityToSlot(FirstHandle, FirstSlotInputTag, true);
		return false;
	}

	const bool bAssignedFirst = AssignAbilityToSlot(FirstHandle, SecondSlotInputTag, true);
	const bool bAssignedSecond = AssignAbilityToSlot(SecondHandle, FirstSlotInputTag, true);

	// Rollback on partial failure to avoid leaving slots in an inconsistent state.
	if (!bAssignedFirst || !bAssignedSecond)
	{
		AssignAbilityToSlot(FirstHandle, FirstSlotInputTag, true);
		AssignAbilityToSlot(SecondHandle, SecondSlotInputTag, true);
		return false;
	}

	return true;
}

int32 UAscendAbilitySystemComponent::AutoAssignDefaultSlotsForGrantedAbilities(bool bReplaceExisting)
{
	int32 AssignedCount = 0;

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.Ability || !Spec.Handle.IsValid())
		{
			continue;
		}

		if (GetSlotInputTagFromSpec(Spec).IsValid())
		{
			continue;
		}

		if (!GetSlotBindingFragmentForAbility(Spec.Handle))
		{
			continue;
		}

		FGameplayTagContainer EffectiveSlotTags;
		GetEffectiveSupportedSlotTags(EffectiveSlotTags);

		for (const FGameplayTag& SupportedSlotTag : EffectiveSlotTags)
		{
			if (AssignAbilityToSlot(Spec.Handle, SupportedSlotTag, bReplaceExisting))
			{
				AssignedCount++;
				break;
			}
		}
	}

	return AssignedCount;
}

bool UAscendAbilitySystemComponent::TryGetAscendAbilitySystem(const UObject* Object, UAscendAbilitySystemComponent*& AbilitySystem)
{
	AbilitySystem = nullptr;
	if (!Object)
	{
		return false;
	}
	const IAbilitySystemInterface* Accessor = Cast<IAbilitySystemInterface>(Object);
	if (Accessor != nullptr)
	{
		AbilitySystem = Cast<UAscendAbilitySystemComponent>(Accessor->GetAbilitySystemComponent());
		if (IsValid(AbilitySystem))
		{
			return true;
		}
	}
	const AActor* Actor = Cast<AActor>(Object);
	if (IsValid(Actor))
	{
		AbilitySystem = Actor->FindComponentByClass<UAscendAbilitySystemComponent>();
		return IsValid(AbilitySystem);
	}

	return false;
}

UAttributeSet* UAscendAbilitySystemComponent::FindSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass) const
{
	if (!AttributeSetClass)
	{
		return nullptr;
	}

	return const_cast<UAttributeSet*>(GetAttributeSubobject(AttributeSetClass));
}

UAttributeSet* UAscendAbilitySystemComponent::GetOrCreateSpawnedAttributeSet(TSubclassOf<UAttributeSet> AttributeSetClass)
{
	if (!AttributeSetClass)
	{
		return nullptr;
	}

	return const_cast<UAttributeSet*>(GetOrCreateAttributeSubobject(AttributeSetClass));
}

void UAscendAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	const FGameplayTag RemovedSlotTag = GetSlotInputTagFromSpec(AbilitySpec);
	AbilityDefinitionMap.Remove(AbilitySpec.Handle);
	AbilitySourceObjectMap.Remove(AbilitySpec.Handle);
	AbilitySlotMap.Remove(AbilitySpec.Handle);
	AbilitySemanticTagMap.Remove(AbilitySpec.Handle);
	if (RemovedSlotTag.IsValid())
	{
		BroadcastSlotChanged(RemovedSlotTag);
	}
	Super::OnRemoveAbility(AbilitySpec);
}

void UAscendAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	AddAbilityToActivationGroup(GetActivationGroupForAbility(Ability), Ability);
}

void UAscendAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	RemoveAbilityFromActivationGroup(GetActivationGroupForAbility(Ability), Ability);
}

void UAscendAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(
	const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility,
	bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags,
	bool bExecuteCancelTags,
	const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer EffectiveAbilityTags = AbilityTags;

	// Relationship rules are authored against semantic definition tags, so merge them in before
	// delegating to the base GAS block/cancel flow.
	if (const UAscendGameplayAbility* AscendAbility = Cast<UAscendGameplayAbility>(RequestingAbility))
	{
		FGameplayTagContainer SemanticAbilityTags;
		if (GetAbilityTagsForAbility(AscendAbility->GetCurrentAbilitySpecHandle(), SemanticAbilityTags))
		{
			EffectiveAbilityTags.AppendTags(SemanticAbilityTags);
		}
	}

	FGameplayTagContainer EffectiveBlockTags = BlockTags;
	FGameplayTagContainer EffectiveCancelTags = CancelTags;

	if (const UAscendAbilityTagRelationshipMapping* Mapping = GetAbilityTagRelationshipMapping())
	{
		FGameplayTagContainer OwnerTags;
		GetOwnedGameplayTags(OwnerTags);
		Mapping->GetAbilityTagsToBlockAndCancelForOwner(OwnerTags, EffectiveAbilityTags, EffectiveBlockTags, EffectiveCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(
		EffectiveAbilityTags,
		RequestingAbility,
		bEnableBlockTags,
		EffectiveBlockTags,
		bExecuteCancelTags,
		EffectiveCancelTags);
}

EAscendAbilityActivationPolicy UAscendAbilitySystemComponent::GetActivationPolicyForSpec(const FGameplayAbilitySpec& Spec) const
{
	const UAscendAbilityDefinition* Definition = GetDefinitionForHandle(Spec.Handle);
	if (Definition)
	{
		if (const FAscendAbilityFragment_InputBinding* InputFragment = Definition->GetInputBindingFragment())
		{
			return InputFragment->ActivationPolicy;
		}
	}

	return EAscendAbilityActivationPolicy::OnInputTriggered;
}

void UAscendAbilitySystemComponent::ApplyAbilityDefinitionToSpec(FGameplayAbilitySpec& AbilitySpec, const UAscendAbilityDefinition* AbilityDefinition) const
{
	if (!AbilityDefinition)
	{
		return;
	}

	if (const FAscendAbilityFragment_InputBinding* InputFragment = AbilityDefinition->GetInputBindingFragment())
	{
		FAscendAbilityFragment_InputBinding NormalizedInputFragment = *InputFragment;
		NormalizedInputFragment.Normalize();

		FGameplayTagContainer GrantedStaticInputTags;
		BuildGrantedStaticInputTags(AbilityDefinition, NormalizedInputFragment, GrantedStaticInputTags);
		AbilitySpec.GetDynamicSpecSourceTags().AppendTags(GrantedStaticInputTags);
		if (NormalizedInputFragment.HasFixedInputBinding())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(NormalizedInputFragment.DefaultSlotInputTag);
		}
	}

}

void UAscendAbilitySystemComponent::BuildGrantedStaticInputTags(
	const UAscendAbilityDefinition* AbilityDefinition,
	const FAscendAbilityFragment_InputBinding& InputFragment,
	FGameplayTagContainer& OutStaticInputTags) const
{
	OutStaticInputTags = InputFragment.AbilityInputTags;

	if (!InputFragment.bSupportsSlotBinding)
	{
		return;
	}

	bool bRemovedSlotInputTag = false;
	FGameplayTagContainer EffectiveSlotTags;
	GetEffectiveSupportedSlotTags(EffectiveSlotTags);

	for (const FGameplayTag& SupportedSlotTag : EffectiveSlotTags)
	{
		if (OutStaticInputTags.HasTagExact(SupportedSlotTag))
		{
			OutStaticInputTags.RemoveTag(SupportedSlotTag);
			bRemovedSlotInputTag = true;
		}
	}

	if (bRemovedSlotInputTag)
	{
		UE_LOG(
			LogAscendAbilitySystem,
			Warning,
			TEXT("Ability definition [%s] supports runtime slot binding. Slot input tags found in Static Input Tags were ignored."),
			*GetNameSafe(AbilityDefinition));
	}
}

void UAscendAbilitySystemComponent::GetEffectiveSupportedSlotTags(FGameplayTagContainer& OutSlotTags) const
{
	OutSlotTags.Reset();

	const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (!ActorInfo || !ActorInfo->PlayerController.IsValid())
	{
		// AI has no slot concept — return empty intentionally
		return;
	}
	
	if (ActorInfo && ActorInfo->PlayerController.IsValid())
	{
		if (const UAscendAbilitySlotComponent* SlotComponent =
			ActorInfo->PlayerController->FindComponentByClass<UAscendAbilitySlotComponent>())
		{
			TArray<FGameplayTag> SlotTags;
			SlotComponent->GetSupportedSlotTags(SlotTags);
			for (const FGameplayTag& SlotTag : SlotTags)
			{
				OutSlotTags.AddTag(SlotTag);
			}
		}
	}

	if (OutSlotTags.IsEmpty())
	{
		OutSlotTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot1);
		OutSlotTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot2);
		OutSlotTags.AddTag(AscendGameplayTags::InputTag_Ability_Slot3);
	}
}

bool UAscendAbilitySystemComponent::AbilitySpecHasInputTag(const FGameplayAbilitySpec& Spec, const FGameplayTag& InputTag) const
{
	if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
	{
		return true;
	}

	const FGameplayTag* AssignedSlot = AbilitySlotMap.Find(Spec.Handle);
	return AssignedSlot && AssignedSlot->MatchesTagExact(InputTag);
}

void UAscendAbilitySystemComponent::GetAbilityTagsForSpec(
	const FGameplayAbilitySpec& Spec,
	FGameplayTagContainer& OutAbilityTags) const
{
	OutAbilityTags.AppendTags(Spec.Ability->GetAssetTags());

	if (const FGameplayTagContainer* SemanticTags = AbilitySemanticTagMap.Find(Spec.Handle))
	{
		OutAbilityTags.AppendTags(*SemanticTags);
	}

	if (const UAscendAbilityDefinition* Definition = GetDefinitionForHandle(Spec.Handle))
	{
		// Fall back to definition tags when the semantic map wasn't populated (e.g. external grant).
		if (!AbilitySemanticTagMap.Contains(Spec.Handle))
		{
			OutAbilityTags.AppendTags(Definition->AbilityTags);
		}
	}
}

const FAscendAbilityFragment_InputBinding* UAscendAbilitySystemComponent::GetSlotBindingFragmentForAbility(
	FGameplayAbilitySpecHandle AbilityHandle) const
{
	const UAscendAbilityDefinition* Definition = GetDefinitionForHandle(AbilityHandle);
	if (!Definition)
	{
		return nullptr;
	}

	const FAscendAbilityFragment_InputBinding* InputFragment = Definition->GetInputBindingFragment();
	if (!InputFragment || !InputFragment->bSupportsSlotBinding)
	{
		return nullptr;
	}

	return InputFragment;
}

void UAscendAbilitySystemComponent::RemoveAllSlotTagsFromSpec(FGameplayAbilitySpec& Spec)
{
	AbilitySlotMap.Remove(Spec.Handle);
}

void UAscendAbilitySystemComponent::ResetInputStateForAbility(FGameplayAbilitySpecHandle AbilityHandle)
{
	if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityHandle))
	{
		Spec->InputPressed = false;
	}
}

FGameplayTag UAscendAbilitySystemComponent::GetSlotInputTagFromSpec(const FGameplayAbilitySpec& Spec) const
{
	const FGameplayTag* AssignedSlot = AbilitySlotMap.Find(Spec.Handle);
	return AssignedSlot ? *AssignedSlot : FGameplayTag();
}

void UAscendAbilitySystemComponent::BroadcastSlotChanged(const FGameplayTag& SlotInputTag)
{
	if (!SlotInputTag.IsValid())
	{
		return;
	}

	FGameplayAbilitySpecHandle AssignedHandle;
	const bool bOccupied = GetAbilityHandleForSlot(SlotInputTag, AssignedHandle);
	OnAbilitySlotChanged.Broadcast(SlotInputTag, AssignedHandle, bOccupied);
}

const FGameplayAbilitySpec* UAscendAbilitySystemComponent::GetAbilitySpecByHandle(
	FGameplayAbilitySpecHandle AbilityHandle) const
{
	return FindAbilitySpecFromHandle(AbilityHandle);
}

FGameplayAbilitySpec* UAscendAbilitySystemComponent::GetAbilitySpecByHandle(FGameplayAbilitySpecHandle AbilityHandle)
{
	return FindAbilitySpecFromHandle(AbilityHandle);
}

const UAscendAbilityTagRelationshipMapping* UAscendAbilitySystemComponent::GetAbilityTagRelationshipMapping() const
{
	if (AbilityTagRelationshipMappingOverride)
	{
		return AbilityTagRelationshipMappingOverride;
	}

	return GetDefault<UAscendAbilitySystemSettings>()->GetDefaultAbilityTagRelationshipMapping();
}

EAscendAbilityActivationGroup UAscendAbilitySystemComponent::GetActivationGroupForAbility(const UGameplayAbility* Ability) const
{
	if (const UAscendGameplayAbility* AscendAbility = Cast<UAscendGameplayAbility>(Ability))
	{
		return AscendAbility->GetActivationGroup();
	}

	return EAscendAbilityActivationGroup::Independent;
}

void UAscendAbilitySystemComponent::AddAbilityToActivationGroup(EAscendAbilityActivationGroup Group, UGameplayAbility* Ability)
{
	if (!Ability || Group == EAscendAbilityActivationGroup::MAX)
	{
		return;
	}

	const uint8 GroupIndex = (uint8)Group;
	check(ActivationGroupCounts[GroupIndex] < MAX_int32);
	ActivationGroupCounts[GroupIndex]++;

	if (Group == EAscendAbilityActivationGroup::ExclusiveReplaceable ||
		Group == EAscendAbilityActivationGroup::ExclusiveBlocking)
	{
		// Exclusive abilities own the action lane and replace any existing replaceable action.
		CancelActivationGroupAbilities(EAscendAbilityActivationGroup::ExclusiveReplaceable, Ability, false);
	}
}

void UAscendAbilitySystemComponent::RemoveAbilityFromActivationGroup(EAscendAbilityActivationGroup Group, UGameplayAbility* Ability)
{
	if (!Ability || Group == EAscendAbilityActivationGroup::MAX)
	{
		return;
	}

	const uint8 GroupIndex = (uint8)Group;
	if (ensure(ActivationGroupCounts[GroupIndex] > 0))
	{
		ActivationGroupCounts[GroupIndex]--;
	}
}

void UAscendAbilitySystemComponent::CancelActivationGroupAbilities(
	EAscendAbilityActivationGroup Group,
	UGameplayAbility* IgnoreAbility,
	bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.Ability || !AbilitySpec.IsActive())
		{
			continue;
		}

		TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : AbilityInstances)
		{
			if (!AbilityInstance || AbilityInstance == IgnoreAbility)
			{
				continue;
			}

			if (GetActivationGroupForAbility(AbilityInstance) != Group)
			{
				continue;
			}

			if (AbilityInstance->CanBeCanceled())
			{
				AbilityInstance->CancelAbility(
					AbilitySpec.Handle,
					AbilityActorInfo.Get(),
					AbilityInstance->GetCurrentActivationInfoRef(),
					bReplicateCancelAbility);
			}
		}
	}
}
