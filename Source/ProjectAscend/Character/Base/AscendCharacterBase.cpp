#include "AscendCharacterBase.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AscendGameplayAbility.h"
#include "AbilitySystem/Attributes/AscendAttributeTypes.h"
#include "AbilitySystem/Data/AscendAbilitySet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/AscendPlayerState.h"
#include "GameplayEffect.h"
#include "AscendGameplayTags.h"

AAscendCharacterBase::AAscendCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* AAscendCharacterBase::GetAbilitySystemComponent() const
{
	if (AAscendPlayerState* PS = GetPlayerState<AAscendPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}

	return AbilitySystemComponent;
}

void AAscendCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeGAS();
}

void AAscendCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeGAS();
}

void AAscendCharacterBase::InitializeGAS()
{
	UAscendAbilitySystemComponent* ResolvedASC = nullptr;

	if (AAscendPlayerState* PS = GetPlayerState<AAscendPlayerState>())
	{
		ResolvedASC = Cast<UAscendAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (ResolvedASC)
		{
			ResolvedASC->InitAbilityActorInfo(PS, this);
		}
	}
	else
	{
		ResolvedASC = AbilitySystemComponent;
		if (ResolvedASC)
		{
			ResolvedASC->InitAbilityActorInfo(this, this);
		}
	}

	AbilitySystemComponent = ResolvedASC;

	if (!AbilitySystemComponent)
	{
		return;
	}

	if (AbilitySet && !bStartupAbilitiesGranted)
	{
		AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr, this);
		bStartupAbilitiesGranted = true;
	}

	const bool bUseLegacyDefaultAttributes =
		DefaultAttributes &&
		!bDefaultAttributesApplied &&
		(!AbilitySet || !AbilitySet->HasAttributeInitializers());

	if (bUseLegacyDefaultAttributes)
	{
		const UGameplayEffect* DefaultAttributesGE = DefaultAttributes->GetDefaultObject<UGameplayEffect>();
		if (DefaultAttributesGE)
		{
			AbilitySystemComponent->ApplyGameplayEffectToSelf(
				DefaultAttributesGE,
				1.0f,
				AbilitySystemComponent->MakeEffectContext());
			bDefaultAttributesApplied = true;
		}
	}
}

void AAscendCharacterBase::Die()
{
	FAscendAttributeSetExecutionData ExecutionData;
	HandleOutOfHealth(ExecutionData);
}

void AAscendCharacterBase::HandleOutOfHealth(const FAscendAttributeSetExecutionData& ExecutionData)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	HandleDeathStarted();

	UAscendAbilitySystemComponent* AscendASC = Cast<UAscendAbilitySystemComponent>(GetAbilitySystemComponent());
	if (AscendASC && !bDeathFinished)
	{
		const FGameplayTag EffectiveDeathTag = DeathAbilityTag.IsValid()
			? DeathAbilityTag
			: AscendGameplayTags::Ability_Type_Death;

		FGameplayAbilitySpecHandle DeathAbilityHandle;
		if (EffectiveDeathTag.IsValid() &&
			AscendASC->FindFirstAbilityHandleByTag(EffectiveDeathTag, DeathAbilityHandle) &&
			DeathAbilityHandle.IsValid())
		{
			FGameplayEventData EventData;
			EventData.Instigator = ExecutionData.SourceActor.Get();
			EventData.Target = this;
			EventData.ContextHandle = ExecutionData.Context;
			EventData.EventMagnitude = FMath::Max(-ExecutionData.DeltaValue, 0.0f);
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AscendGameplayTags::Event_Death, EventData);
			return;
		}
	}

	FinishDeath();
}

void AAscendCharacterBase::FinishDeath()
{
	if (bDeathFinished)
	{
		return;
	}

	bDeathFinished = true;
	HandleDeathFinished();
	OnCharacterDied.Broadcast(this);
}

void AAscendCharacterBase::HandleDeathStarted()
{
}

void AAscendCharacterBase::HandleDeathFinished()
{
}
