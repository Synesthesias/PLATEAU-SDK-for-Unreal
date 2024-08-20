// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "../PLATEAUAutomationTestUtil.h"
#include "Reconstruct/PLATEAUMeshLoaderCloneComponent.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>
#include "MeshDescription.h"

/// <summary>
/// コンポーネントパラメータコピー利用 MeshLoader (FPLATEAUMeshLoaderCloneComponent) Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_MeshLoader_CloneComponent, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.MeshLoader.CloneComponent", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


void FPLATEAUTest_MeshLoader_CloneComponent_TestStaticMesh(FPLATEAUAutomationTestBase* Test, UPLATEAUCityObjectGroup* Comp, UPLATEAUCityObjectGroup* OriginalItem, int32 NumIndices) {

    Test->TestNotNull("Component StaticMesh is not null ", Comp->GetStaticMesh().Get());
    Test->TestEqual("Vertex sizes are the same as Models", Comp->GetStaticMesh()->GetNumVertices(0), NumIndices);
    Test->TestEqual("Material is same as original", Comp->GetMaterial(0), OriginalItem->GetMaterial(0));

    UPLATEAUCityObjectGroup* CompAsCityObj = StaticCast<UPLATEAUCityObjectGroup*>(Comp);
    Test->TestEqual("Json is same as original", CompAsCityObj->SerializedCityObjects, OriginalItem->SerializedCityObjects);
    Test->TestEqual("Granularity is same as LoadInputData", CompAsCityObj->GetConvertGranularity(), ConvertGranularity::PerPrimaryFeatureObject);
    Test->AddInfo(FString::Format(TEXT("MeshGranularity: {0}"), { CompAsCityObj->MeshGranularityIntValue }));
    Test->AddInfo("StaticMesh Test Finished.");
}

/// <summary>
/// FPLATEAUMeshLoaderCloneComponentの単体テスト
/// 主に階層生成テスト
/// </summary>
bool FPLATEAUTest_MeshLoader_CloneComponent::RunTest(const FString& Parameters) {
    InitializeTest("CloneComponent");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    plateau::polygonMesh::Mesh Mesh;
    plateau::polygonMesh::CityObjectList CityObjectList;
    PLATEAUAutomationTestUtil::CreateCityObjectList(CityObjectList);
    PLATEAUAutomationTestUtil::CreateMesh(Mesh, CityObjectList);
    std::shared_ptr<plateau::polygonMesh::Model> Model = PLATEAUAutomationTestUtil::CreateModel(Mesh);

    const auto& Actor = PLATEAUAutomationTestUtil::CreateActor(*GetWorld());
    const auto LoadData = PLATEAUAutomationTestUtil::CreateLoadInputData(plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject);
    const ConvertGranularity ConvGranularity = ConvertGranularity::PerPrimaryFeatureObject;
    TMap<FString, FPLATEAUCityObject> CityObj = PLATEAUAutomationTestUtil::CreateCityObjectMap();
        
    //CityObjectGroup Item
    auto OriginalItem = Actor->FindComponentByTag<UPLATEAUCityObjectGroup>(PLATEAUAutomationTestUtil::TEST_OBJ_TAG);
    OriginalItem->SerializeCityObject(PLATEAUAutomationTestUtil::GetObjNode(Model), CityObj[PLATEAUAutomationTestUtil::TEST_OBJ_NAME], ConvertGranularity::PerPrimaryFeatureObject);
    TMap<FString, UPLATEAUCityObjectGroup*> CompMap = FPLATEAUComponentUtil::CreateComponentsMapWithNodePath(TArray<UPLATEAUCityObjectGroup*>{OriginalItem});
    OriginalItem->SetStaticMesh(PLATEAUAutomationTestUtil::CreateStaticMesh(Actor, FName(TEXT("TestStaticMesh"))));
    PLATEAUAutomationTestUtil::SetMaterial(OriginalItem);

    TAtomic<bool> bCanceled;
    bCanceled.Store(false);
    FPLATEAUMeshLoaderCloneComponent MeshLoader;

    MeshLoader.ReloadComponentFromModel(Model, CompMap, *Actor);

    AddInfo(FString(Model->debugString().c_str()));

    GEngine->BroadcastLevelActorListChanged();

    //Assertions
    const auto DefaultSceneRootComp = Actor->GetRootComponent();
    const auto RootComp = DefaultSceneRootComp->GetAttachChildren()[0];
    const auto LodComp = RootComp->GetAttachChildren()[0];
    const auto ObjComps = LodComp->GetAttachChildren();

    ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

    TestEqual("Obj should be 2", ObjComps.Num() , 2 );
    for (auto Obj : ObjComps) {
        TestEqual("Original Compoenent Name = Node Name", FPLATEAUComponentUtil::GetOriginalComponentName(Obj), PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

        UPLATEAUCityObjectGroup* CityObjGrp = StaticCast<UPLATEAUCityObjectGroup*>(Obj);
        TestEqual("GmlID = Node Name", CityObjGrp->GetAllRootCityObjects()[0].GmlID, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

        //新規に生成されたComponent
        if (CityObjGrp->GetName() == PLATEAUAutomationTestUtil::TEST_OBJ_NAME + "__2") {
            const int32 NumIndices = (int32)Mesh.getIndices().size();

            if (!CityObjGrp->GetStaticMesh()) {
                    //StaticMesh生成を待機
                bool OnGetStaticMeshFinish = false;
                CityObjGrp->OnStaticMeshChanged().AddLambda([&, this, NumIndices, OriginalItem](UStaticMeshComponent* Comp) {
                    UPLATEAUCityObjectGroup* CompAsCityObj = StaticCast<UPLATEAUCityObjectGroup*>(Comp);
                    FPLATEAUTest_MeshLoader_CloneComponent_TestStaticMesh(this, CompAsCityObj, OriginalItem, NumIndices);
                    OnGetStaticMeshFinish = true;
                    });

                //Test終了まで待機
                ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([&] {
                    return OnGetStaticMeshFinish;
                    }));
            }
            else {
                FPLATEAUTest_MeshLoader_CloneComponent_TestStaticMesh(this, CityObjGrp, OriginalItem, NumIndices);
            }
        }
    }

    return true;
}
