#include "Animation/NotifyStates/AscendTimeDilation.h"

#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AscendTimeDilation)

FString UAscendTimeDilation::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Ascend Time Dilation: %.2f"), TimeDilation);
}

void UAscendTimeDilation::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (bAffectGlobalTimeDilation)
	{
		ApplyGlobalTimeDilation(MeshComp->GetWorld());
		return;
	}

	ApplyActorTimeDilation(MeshComp->GetOwner());
}

void UAscendTimeDilation::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!bRestoreOnEnd || !MeshComp)
	{
		return;
	}

	if (bAffectGlobalTimeDilation)
	{
		RestoreGlobalTimeDilation(MeshComp->GetWorld());
		return;
	}

	RestoreActorTimeDilation(MeshComp->GetOwner());
}

void UAscendTimeDilation::ApplyActorTimeDilation(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return;
	}

	if (!PreviousActorTimeDilations.Contains(OwnerActor))
	{
		PreviousActorTimeDilations.Add(OwnerActor, OwnerActor->CustomTimeDilation);
	}

	OwnerActor->CustomTimeDilation = TimeDilation;
}

void UAscendTimeDilation::RestoreActorTimeDilation(AActor* OwnerActor)
{
	if (!OwnerActor)
	{
		return;
	}

	if (const float* PreviousValue = PreviousActorTimeDilations.Find(OwnerActor))
	{
		OwnerActor->CustomTimeDilation = *PreviousValue;
		PreviousActorTimeDilations.Remove(OwnerActor);
		return;
	}

	OwnerActor->CustomTimeDilation = 1.0f;
}

void UAscendTimeDilation::ApplyGlobalTimeDilation(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (!PreviousGlobalTimeDilations.Contains(World))
	{
		PreviousGlobalTimeDilations.Add(World, UGameplayStatics::GetGlobalTimeDilation(World));
	}

	UGameplayStatics::SetGlobalTimeDilation(World, TimeDilation);
}

void UAscendTimeDilation::RestoreGlobalTimeDilation(UWorld* World)
{
	if (!World)
	{
		return;
	}

	if (const float* PreviousValue = PreviousGlobalTimeDilations.Find(World))
	{
		UGameplayStatics::SetGlobalTimeDilation(World, *PreviousValue);
		PreviousGlobalTimeDilations.Remove(World);
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
}
