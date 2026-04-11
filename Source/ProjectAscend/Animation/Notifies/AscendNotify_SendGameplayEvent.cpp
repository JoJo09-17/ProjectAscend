#include "Animation/Notifies/AscendNotify_SendGameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AscendAbilitySystemComponent.h"
#include "Animation/AnimMontage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendNotify_SendGameplayEvent)

UAscendNotify_SendGameplayEvent::UAscendNotify_SendGameplayEvent()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif

	bIsNativeBranchingPoint = true;
}

FString UAscendNotify_SendGameplayEvent::GetNotifyName_Implementation() const
{
#if WITH_EDITOR
	if (!CustomDescription.IsNone())
	{
		return FString::Printf(TEXT("Ascend Event: %s"), *CustomDescription.ToString());
	}
#endif

	return EventTag.IsValid()
		? FString::Printf(TEXT("Ascend Event: %s"), *EventTag.ToString())
		: TEXT("Ascend Event");
}

void UAscendNotify_SendGameplayEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !EventTag.IsValid())
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UAscendAbilitySystemComponent* AbilitySystemComponent = nullptr;
	if (!UAscendAbilitySystemComponent::TryGetAscendAbilitySystem(OwnerActor, AbilitySystemComponent))
	{
		return;
	}

	FGameplayEventData EventData;
	BuildEventPayload(MeshComp, Animation, AbilitySystemComponent, EventData);

	// The notify is only responsible for producing a stable gameplay event payload.
	// Ability-side routing stays in GAS where montage tasks already listen for these tags.
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, EventData);
}

#if WITH_EDITOR
bool UAscendNotify_SendGameplayEvent::CanBePlaced(UAnimSequenceBase* Animation) const
{
	return Animation && Animation->IsA(UAnimMontage::StaticClass());
}
#endif

void UAscendNotify_SendGameplayEvent::BuildEventPayload(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	UAscendAbilitySystemComponent* AbilitySystemComponent,
	FGameplayEventData& OutEventData) const
{
	OutEventData = FGameplayEventData();
	OutEventData.EventTag = EventTag;
	OutEventData.EventMagnitude = EventMagnitude;

	if (AbilitySystemComponent && AbilitySystemComponent->AbilityActorInfo.IsValid())
	{
		const FGameplayAbilityActorInfo* ActorInfo = AbilitySystemComponent->AbilityActorInfo.Get();
		OutEventData.Instigator = ActorInfo->OwnerActor.Get();
		OutEventData.Target = ActorInfo->AvatarActor.Get();
		OutEventData.ContextHandle = AbilitySystemComponent->MakeEffectContext();
	}
	else if (MeshComp)
	{
		OutEventData.Instigator = MeshComp->GetOwner();
		OutEventData.Target = MeshComp->GetOwner();
	}

	if (bAddAnimationAsSourceObject && AbilitySystemComponent && Animation)
	{
		OutEventData.ContextHandle.AddSourceObject(Animation);
	}

	if (bStoreAnimationInOptionalObject)
	{
		OutEventData.OptionalObject = Animation;
	}

	if (bStoreMeshInOptionalObject2)
	{
		OutEventData.OptionalObject2 = MeshComp;
	}
}
