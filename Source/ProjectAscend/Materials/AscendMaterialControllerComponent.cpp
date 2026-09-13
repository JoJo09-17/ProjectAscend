#include "Materials/AscendMaterialControllerComponent.h"
#include "Materials/AscendMaterialController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UAscendMaterialControllerComponent::UAscendMaterialControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UAscendMaterialControllerComponent::InitializeMaterials(UMeshComponent* Mesh)
{
	RestoreOriginalMaterials();
	if (!IsValid(Mesh)) { return false; }
	TargetMesh = Mesh;
	for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
	{
		UMaterialInterface* Original = Mesh->GetMaterial(Slot);
		OriginalMaterials.Add(Original);
		// UE does not support dynamic instances as parents. Keep the original
		// instance for restoration, but use its non-dynamic parent for our MID.
		UMaterialInterface* Parent = Original;
		while (UMaterialInstanceDynamic* Existing = Cast<UMaterialInstanceDynamic>(Parent)) { Parent = Existing->Parent; }
		UMaterialInstanceDynamic* Dynamic = Parent ? UMaterialInstanceDynamic::Create(Parent, this) : nullptr;
		DynamicMaterials.Add(Dynamic);
		if (Dynamic) { Mesh->SetMaterial(Slot, Dynamic); }
	}
	RebuildParameters();
	return DynamicMaterials.ContainsByPredicate([](const auto& Material) { return Material != nullptr; });
}

int32 UAscendMaterialControllerComponent::ApplyController(UAscendMaterialController* Controller)
{
	if (!IsValid(Controller)) { return 0; }
	if (!IsValid(TargetMesh) || !DynamicMaterials.ContainsByPredicate([](const auto& Material) { return Material != nullptr; }))
	{
		AActor* Owner = GetOwner();
		UMeshComponent* Mesh = nullptr;
		if (ACharacter* Avatar = Cast<ACharacter>(Owner)) { Mesh = Avatar->GetMesh(); }
		else if (Owner) { Mesh = Owner->FindComponentByClass<UMeshComponent>(); }
		if (!InitializeMaterials(Mesh)) { return 0; }
	}
	FAscendActiveMaterialController& Active = ActiveControllers.AddDefaulted_GetRef();
	Active.Controller = Controller;
	Active.Handle = NextHandle++;
	Active.RemainingTime = FMath::Max(Controller->Duration, 0.0f);
	Active.bTimed = Active.RemainingTime > 0.0f;
	const int32 Handle = Active.Handle;
	RebuildParameters();
	UpdateTickEnabled();
	return Handle;
}

bool UAscendMaterialControllerComponent::RemoveController(int32 Handle)
{
	const int32 Removed = ActiveControllers.RemoveAll([Handle](const auto& Active) { return Active.Handle == Handle; });
	if (Removed) { RebuildParameters(); UpdateTickEnabled(); }
	return Removed > 0;
}

void UAscendMaterialControllerComponent::RemoveControllersByAsset(UAscendMaterialController* Controller)
{
	ActiveControllers.RemoveAll([Controller](const auto& Active) { return Active.Controller == Controller; });
	RebuildParameters();
	UpdateTickEnabled();
}

void UAscendMaterialControllerComponent::ClearControllers()
{
	ActiveControllers.Reset();
	RebuildParameters();
	UpdateTickEnabled();
}

void UAscendMaterialControllerComponent::RebuildParameters()
{
	// Restore the original baseline before recomposing active effects.
	for (int32 Slot = 0; Slot < DynamicMaterials.Num(); ++Slot)
	{
		if (UMaterialInstanceDynamic* Dynamic = DynamicMaterials[Slot])
		{
			Dynamic->ClearParameterValues();
			if (Cast<UMaterialInstanceDynamic>(OriginalMaterials[Slot]))
			{
				Dynamic->CopyMaterialUniformParameters(OriginalMaterials[Slot]);
			}
		}
	}
	ActiveControllers.Sort([](const auto& A, const auto& B)
	{
		const int32 APriority = A.Controller ? A.Controller->Priority : 0;
		const int32 BPriority = B.Controller ? B.Controller->Priority : 0;
		return APriority == BPriority ? A.Handle < B.Handle : APriority < BPriority;
	});
	for (const auto& Active : ActiveControllers)
	{
		const UAscendMaterialController* Controller = Active.Controller;
		if (!Controller) { continue; }
		for (int32 Slot = 0; Slot < DynamicMaterials.Num(); ++Slot)
		{
			UMaterialInstanceDynamic* Dynamic = DynamicMaterials[Slot];
			if (!Dynamic || (!Controller->MaterialSlots.IsEmpty() && !Controller->MaterialSlots.Contains(Slot))) { continue; }
			for (const auto& Parameter : Controller->ScalarParameters) { Dynamic->SetScalarParameterValue(Parameter.Key, Parameter.Value); }
			for (const auto& Parameter : Controller->VectorParameters) { Dynamic->SetVectorParameterValue(Parameter.Key, Parameter.Value); }
			for (const auto& Parameter : Controller->TextureParameters) { Dynamic->SetTextureParameterValue(Parameter.Key, Parameter.Value); }
		}
	}
}

void UAscendMaterialControllerComponent::UpdateTickEnabled()
{
	SetComponentTickEnabled(ActiveControllers.ContainsByPredicate([](const auto& Active) { return Active.bTimed; }));
}

void UAscendMaterialControllerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	for (auto& Active : ActiveControllers) { if (Active.bTimed) { Active.RemainingTime -= DeltaTime; } }
	if (ActiveControllers.RemoveAll([](const auto& Active) { return Active.bTimed && Active.RemainingTime <= 0.0f; }))
	{
		RebuildParameters();
		UpdateTickEnabled();
	}
}

void UAscendMaterialControllerComponent::RestoreOriginalMaterials()
{
	if (IsValid(TargetMesh))
	{
		for (int32 Slot = 0; Slot < OriginalMaterials.Num(); ++Slot)
		{
			// Don't overwrite materials replaced by another system after initialization.
			if (TargetMesh->GetMaterial(Slot) == DynamicMaterials[Slot]) { TargetMesh->SetMaterial(Slot, OriginalMaterials[Slot]); }
		}
	}
	ActiveControllers.Reset();
	DynamicMaterials.Reset();
	OriginalMaterials.Reset();
	TargetMesh = nullptr;
	SetComponentTickEnabled(false);
}

UMaterialInstanceDynamic* UAscendMaterialControllerComponent::GetDynamicMaterial(int32 Slot) const
{
	return DynamicMaterials.IsValidIndex(Slot) ? DynamicMaterials[Slot].Get() : nullptr;
}

void UAscendMaterialControllerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreOriginalMaterials();
	Super::EndPlay(EndPlayReason);
}
