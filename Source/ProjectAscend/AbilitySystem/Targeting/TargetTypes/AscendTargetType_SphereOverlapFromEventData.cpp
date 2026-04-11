#include "AbilitySystem/Targeting/TargetTypes/AscendTargetType_SphereOverlapFromEventData.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Base/AscendCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendTargetType_SphereOverlapFromEventData)

namespace
{
	bool ResolveEventOrigin(const FGameplayEventData& EventData, const AActor* TargetingActor, FVector& OutOrigin)
	{
		if (const FHitResult* FoundHitResult = EventData.ContextHandle.GetHitResult())
		{
			OutOrigin = FoundHitResult->ImpactPoint;
			return true;
		}

		if (EventData.Target)
		{
			OutOrigin = EventData.Target.Get()->GetActorLocation();
			return true;
		}

		if (TargetingActor)
		{
			OutOrigin = TargetingActor->GetActorLocation();
			return true;
		}

		return false;
	}
}

void FAscendTargetType_SphereOverlapFromEventData::GetTargets(
	AAscendCharacterBase* TargetingCharacter,
	AActor* TargetingActor,
	const FGameplayEventData& EventData,
	TArray<FHitResult>& OutHitResults,
	TArray<AActor*>& OutActors) const
{
	UWorld* World = TargetingActor ? TargetingActor->GetWorld() : (TargetingCharacter ? TargetingCharacter->GetWorld() : nullptr);
	if (!World)
	{
		return;
	}

	FVector QueryOrigin = FVector::ZeroVector;
	if (!ResolveEventOrigin(EventData, TargetingActor, QueryOrigin) || QueryOrigin.IsNearlyZero())
	{
		return;
	}

	if (bDrawDebug)
	{
		DrawDebugSphere(World, QueryOrigin, Radius, 24, DebugColor.ToFColor(true), false, DebugDuration, 0, DebugThickness);
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(QueryChannel);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AscendTargetTypeSphereOverlap), false, bIgnoreSourceActor ? TargetingActor : nullptr);
	if (!World->OverlapMultiByObjectType(
		OverlapResults,
		QueryOrigin,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(Radius),
		QueryParams))
	{
		return;
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* ResolvedActor = OverlapResult.GetActor();
		if (!ResolvedActor || (bIgnoreSourceActor && ResolvedActor == TargetingActor))
		{
			continue;
		}

		OutActors.AddUnique(ResolvedActor);
	}
}
