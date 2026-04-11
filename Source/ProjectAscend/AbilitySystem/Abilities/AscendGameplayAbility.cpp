#include "AscendGameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Definition/AscendAbilityDefinition.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_AnimationProvider.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_EffectContainers.h"
#include "AbilitySystem/Fragments/AscendAbilityFragment_InputBinding.h"
#include "AbilitySystem/Providers/AscendAnimationProvider.h"
#include "AbilitySystem/Targeting/AscendTargetType.h"
#include "Character/Base/AscendCharacterBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendGameplayAbility)

UAscendGameplayAbility::UAscendGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

const UAscendAbilityDefinition* UAscendGameplayAbility::GetAbilityDefinition() const
{
	if (AbilityDefinition)
	{
		return AbilityDefinition;
	}

	if (!CurrentSpecHandle.IsValid() || !CurrentActorInfo || !CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		return nullptr;
	}

	const UAscendAbilitySystemComponent* ASC = Cast<UAscendAbilitySystemComponent>(
		CurrentActorInfo->AbilitySystemComponent.Get());
	if (ASC)
	{
		AbilityDefinition = ASC->GetDefinitionForHandle(CurrentSpecHandle);
	}

	return AbilityDefinition;
}

UObject* UAscendGameplayAbility::GetAbilitySourceObject() const
{
	if (!CurrentSpecHandle.IsValid() || !CurrentActorInfo || !CurrentActorInfo->AbilitySystemComponent.IsValid())
	{
		return nullptr;
	}

	const UAscendAbilitySystemComponent* ASC = Cast<UAscendAbilitySystemComponent>(
		CurrentActorInfo->AbilitySystemComponent.Get());
	return ASC ? ASC->GetSourceObjectForHandle(CurrentSpecHandle) : nullptr;
}

FGameplayTagContainer UAscendGameplayAbility::GetAbilityDefinitionTags() const
{
	const UAscendAbilityDefinition* Definition = GetAbilityDefinition();
	return Definition ? Definition->AbilityTags : FGameplayTagContainer();
}

FGameplayTagContainer UAscendGameplayAbility::GetOwnedTagsFromDefinition() const
{
	const UAscendAbilityDefinition* Definition = GetAbilityDefinition();
	return Definition ? Definition->OwnedTags : FGameplayTagContainer();
}

FGameplayCueParameters UAscendGameplayAbility::MakeGameplayCueParameters(FVector Location) const
{
	FGameplayCueParameters Parameters;
	Parameters.AbilityLevel = GetAbilityLevel();
	Parameters.EffectContext = MakeEffectContext(CurrentSpecHandle, GetCurrentActorInfo());
	Parameters.Instigator = GetAvatarActorFromActorInfo();
	Parameters.EffectCauser = GetAvatarActorFromActorInfo();
	Parameters.SourceObject = GetAbilitySourceObject();
	Parameters.AggregatedSourceTags = GetAbilityDefinitionTags();

	if (Location.IsNearlyZero())
	{
		if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			Location = AvatarActor->GetActorLocation();
		}
	}

	Parameters.Location = Location;
	return Parameters;
}

void UAscendGameplayAbility::ExecuteGameplayCueOnOwner(const FGameplayTag& GameplayCueTag, FGameplayCueParameters Parameters)
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->ExecuteGameplayCue(GameplayCueTag, Parameters);
	}
}

void UAscendGameplayAbility::AddGameplayCueOnOwner(const FGameplayTag& GameplayCueTag, FGameplayCueParameters Parameters)
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddGameplayCue(GameplayCueTag, Parameters);
	}
}

void UAscendGameplayAbility::RemoveGameplayCueFromOwner(const FGameplayTag& GameplayCueTag)
{
	if (!GameplayCueTag.IsValid())
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveGameplayCue(GameplayCueTag);
	}
}

const FAscendAbilityFragment_AnimationProvider* UAscendGameplayAbility::GetAnimationProviderFragment() const
{
	const UAscendAbilityDefinition* Definition = GetAbilityDefinition();
	return Definition ? Definition->GetAnimationProviderFragment() : nullptr;
}

UAscendAnimationProvider* UAscendGameplayAbility::GetAnimationProvider() const
{
	const FAscendAbilityFragment_AnimationProvider* Fragment = GetAnimationProviderFragment();
	return Fragment ? Fragment->Provider : nullptr;
}

const FAscendAbilityFragment_EffectContainers* UAscendGameplayAbility::GetEffectContainersFragment() const
{
	const UAscendAbilityDefinition* Definition = GetAbilityDefinition();
	return Definition ? Definition->GetEffectContainersFragment() : nullptr;
}

bool UAscendGameplayAbility::GetEffectContainer(FGameplayTag ContainerTag, FAscendGameplayEffectContainer& OutContainer) const
{
	if (!ContainerTag.IsValid())
	{
		return false;
	}

	const FAscendAbilityFragment_EffectContainers* Fragment = GetEffectContainersFragment();
	if (!Fragment)
	{
		return false;
	}

	if (const FAscendGameplayEffectContainer* FoundContainer = Fragment->EffectContainerMap.Find(ContainerTag))
	{
		OutContainer = *FoundContainer;
		return true;
	}

	return false;
}

FAscendGameplayEffectContainerSpec UAscendGameplayAbility::MakeEffectContainerSpecFromContainer(
	const FAscendGameplayEffectContainer& Container,
	const FGameplayEventData& EventData,
	int32 OverrideGameplayLevel)
{
	FAscendGameplayEffectContainerSpec ContainerSpec;
	const UAscendAbilitySystemComponent* ASC = Cast<UAscendAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!ASC)
	{
		return ContainerSpec;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	AActor* OwningActor = GetOwningActorFromActorInfo();
	AActor* TargetingActor = AvatarActor ? AvatarActor : OwningActor;
	AAscendCharacterBase* TargetingCharacter = Cast<AAscendCharacterBase>(TargetingActor);

	if (Container.TargetType.IsValid())
	{
		TArray<FHitResult> HitResults;
		TArray<AActor*> TargetActors;

		if (const FAscendTargetType* TargetType = Container.TargetType.GetPtr<FAscendTargetType>())
		{
			TargetType->GetTargets(TargetingCharacter, TargetingActor, EventData, HitResults, TargetActors);
			ContainerSpec.AddTargets(HitResults, TargetActors);
		}
	}

	// Merge externally-provided target data (e.g. from targeting tasks) with resolved targets.
	if (EventData.TargetData.Num() > 0)
	{
		ContainerSpec.AddTargetData(EventData.TargetData);
	}

	if (OverrideGameplayLevel == INDEX_NONE)
	{
		OverrideGameplayLevel = GetAbilityLevel();
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
	{
		if (!EffectClass)
		{
			continue;
		}

		ContainerSpec.TargetGameplayEffectSpecs.Add(MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel));
	}

	return ContainerSpec;
}

FAscendGameplayEffectContainerSpec UAscendGameplayAbility::MakeEffectContainerSpec(
	FGameplayTag ContainerTag,
	const FGameplayEventData& EventData,
	int32 OverrideGameplayLevel)
{
	FAscendGameplayEffectContainer Container;
	return GetEffectContainer(ContainerTag, Container)
		? MakeEffectContainerSpecFromContainer(Container, EventData, OverrideGameplayLevel)
		: FAscendGameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UAscendGameplayAbility::ApplyEffectContainerSpec(
	const FAscendGameplayEffectContainerSpec& ContainerSpec)
{
	TArray<FActiveGameplayEffectHandle> AppliedEffects;

	for (const FGameplayEffectSpecHandle& EffectSpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		AppliedEffects.Append(K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, ContainerSpec.TargetData));
	}

	return AppliedEffects;
}

void UAscendGameplayAbility::SetSetByCallerMagnitudeOnContainerSpec(
	FAscendGameplayEffectContainerSpec& ContainerSpec,
	FGameplayTag DataTag,
	float Magnitude) const
{
	if (!DataTag.IsValid())
	{
		return;
	}

	for (FGameplayEffectSpecHandle& EffectSpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		SetSetByCallerMagnitudeOnEffectSpec(EffectSpecHandle, DataTag, Magnitude);
	}
}

TArray<FActiveGameplayEffectHandle> UAscendGameplayAbility::ApplyEffectContainer(
	FGameplayTag ContainerTag,
	const FGameplayEventData& EventData,
	int32 OverrideGameplayLevel)
{
	return ApplyEffectContainerSpec(MakeEffectContainerSpec(ContainerTag, EventData, OverrideGameplayLevel));
}

void UAscendGameplayAbility::SetSetByCallerMagnitudeOnEffectSpec(
	FGameplayEffectSpecHandle& EffectSpecHandle,
	FGameplayTag DataTag,
	float Magnitude) const
{
	if (!DataTag.IsValid() || !EffectSpecHandle.IsValid())
	{
		return;
	}

	EffectSpecHandle.Data->SetSetByCallerMagnitude(DataTag, Magnitude);
}

bool UAscendGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAscendAbilitySystemComponent* ASC = ActorInfo
		? Cast<UAscendAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get())
		: nullptr;
	if (!ASC)
	{
		return true;
	}

	if (ASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(UAbilitySystemGlobals::Get().ActivateFailTagsBlockedTag);
		}

		return false;
	}

	FGameplayTagContainer EffectiveAbilityTags;
	if (!ASC->GetAbilityTagsForAbility(Handle, EffectiveAbilityTags))
	{
		EffectiveAbilityTags.AppendTags(GetAssetTags());
	}

	// Semantic relationship rules extend the default GAS tag checks with definition-authored tags.
	if (ASC->AreAbilityTagsBlocked(EffectiveAbilityTags))
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(UAbilitySystemGlobals::Get().ActivateFailTagsBlockedTag);
		}

		return false;
	}

	FGameplayTagContainer RequiredTags;
	FGameplayTagContainer BlockedTags;
	ASC->GetRelationshipActivationTagRequirements(EffectiveAbilityTags, RequiredTags, BlockedTags);

	if (RequiredTags.IsEmpty() && BlockedTags.IsEmpty())
	{
		return true;
	}

	FGameplayTagContainer OwnedTags;
	ASC->GetOwnedGameplayTags(OwnedTags);

	const bool bMissingRequiredTags = !RequiredTags.IsEmpty() && !OwnedTags.HasAll(RequiredTags);
	const bool bHasBlockedTags = !BlockedTags.IsEmpty() && OwnedTags.HasAny(BlockedTags);
	if (!bMissingRequiredTags && !bHasBlockedTags)
	{
		return true;
	}

	if (OptionalRelevantTags)
	{
		if (bMissingRequiredTags)
		{
			OptionalRelevantTags->AddTag(UAbilitySystemGlobals::Get().ActivateFailTagsMissingTag);
			OptionalRelevantTags->AppendTags(RequiredTags);
		}

		if (bHasBlockedTags)
		{
			OptionalRelevantTags->AddTag(UAbilitySystemGlobals::Get().ActivateFailTagsBlockedTag);
			OptionalRelevantTags->AppendTags(BlockedTags);
		}
	}

	return false;
}

void UAscendGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
	// Seed activation tags from the definition so they're available throughout the ability lifecycle.
	ActivationOwnedTags.Reset();

	if (const UAscendAbilityDefinition* Definition = GetAbilityDefinition())
	{
		ActivationOwnedTags.AppendTags(Definition->OwnedTags);
	}

	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

EAscendAbilityActivationPolicy UAscendGameplayAbility::GetActivationPolicy() const
{
	const UAscendAbilityDefinition* Definition = GetAbilityDefinition();
	if (!Definition)
	{
		return EAscendAbilityActivationPolicy::OnInputTriggered;
	}

	if (const FAscendAbilityFragment_InputBinding* InputFragment = Definition->GetInputBindingFragment())
	{
		return InputFragment->ActivationPolicy;
	}

	return EAscendAbilityActivationPolicy::OnInputTriggered;
}
