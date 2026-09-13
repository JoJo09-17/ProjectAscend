#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Materials/AscendMaterialController.h"
#include "Materials/AscendMaterialControllerComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAscendMaterialControllerTest, "Ascend.Materials.ControllerLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAscendMaterialControllerTest::RunTest(const FString& Parameters)
{
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!TestNotNull(TEXT("Test mesh"), Sphere)) { return false; }
	UMaterialInterface* Original = UMaterial::GetDefaultMaterial(MD_Surface);
	const FName WorldName = MakeUniqueObjectName(nullptr, UWorld::StaticClass(), TEXT("MaterialControllerWorld"), EUniqueObjectNameOptions::GloballyUnique);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, WorldName, GetTransientPackage());
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AActor* OwnerA = World->SpawnActor<AActor>();
	AActor* OwnerB = World->SpawnActor<AActor>();
	UStaticMeshComponent* MeshA = NewObject<UStaticMeshComponent>();
	UStaticMeshComponent* MeshB = NewObject<UStaticMeshComponent>();
	MeshA->SetStaticMesh(Sphere);
	MeshB->SetStaticMesh(Sphere);
	MeshA->SetMaterial(0, Original);
	MeshB->SetMaterial(0, Original);
	UAscendMaterialControllerComponent* A = NewObject<UAscendMaterialControllerComponent>(OwnerA);
	UAscendMaterialControllerComponent* B = NewObject<UAscendMaterialControllerComponent>(OwnerB);
	A->RegisterComponent();
	B->RegisterComponent();
	TestTrue(TEXT("Initialize A"), A->InitializeMaterials(MeshA));
	TestTrue(TEXT("Initialize B"), B->InitializeMaterials(MeshB));
	TestTrue(TEXT("Actors have independent MIDs"), A->GetDynamicMaterial(0) != B->GetDynamicMaterial(0));
	UAscendMaterialController* Timed = NewObject<UAscendMaterialController>();
	Timed->Duration = 0.2f;
	UAscendMaterialController* Persistent = NewObject<UAscendMaterialController>();
	Persistent->Duration = 0.0f;
	const int32 TimedHandle = A->ApplyController(Timed);
	const int32 PersistentHandle = A->ApplyController(Persistent);
	TestTrue(TEXT("Unique valid handles"), TimedHandle > 0 && PersistentHandle > 0 && TimedHandle != PersistentHandle);
	A->TickComponent(0.3f, LEVELTICK_All, nullptr);
	TestEqual(TEXT("Only persistent effect remains"), A->GetActiveControllerCount(), 1);
	TestEqual(TEXT("Other actor is unaffected"), B->GetActiveControllerCount(), 0);
	TestFalse(TEXT("Expired handle cannot be removed"), A->RemoveController(TimedHandle));
	TestTrue(TEXT("Persistent effect can be removed"), A->RemoveController(PersistentHandle));
	TestEqual(TEXT("DA configuration is unchanged"), Timed->Duration, 0.2f);
	A->RestoreOriginalMaterials();
	TestTrue(TEXT("Original material is restored"), MeshA->GetMaterial(0) == Original);
	UMaterialInstanceDynamic* Existing = UMaterialInstanceDynamic::Create(Original, MeshA);
	MeshA->SetMaterial(0, Existing);
	TestTrue(TEXT("Can wrap an existing MID"), A->InitializeMaterials(MeshA));
	TestTrue(TEXT("Existing MID is not used as parent"), A->GetDynamicMaterial(0)->Parent == Original);
	A->RestoreOriginalMaterials();
	TestTrue(TEXT("Existing MID is restored"), MeshA->GetMaterial(0) == Existing);
	B->RestoreOriginalMaterials();
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}
#endif
