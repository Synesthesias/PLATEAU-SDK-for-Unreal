// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "../PLATEAUAutomationTestUtil.h"
#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>

/// <summary>
/// 分割・結合 用 MeshLoader (FPLATEAUMeshLoaderForReconstruct) Test
/// 主に階層生成テスト
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_MeshLoader_Reconstruct_Model, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.MeshLoader.ReconstructModel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_MeshLoader_Reconstruct_Model::RunTest(const FString& Parameters) {
    InitializeTest("ReconstructModel");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");


    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([&,this] {

        plateau::polygonMesh::Mesh Mesh;
        plateau::polygonMesh::CityObjectList CityObjectList;
        PLATEAUAutomationTestUtil::CreateCityObjectList(CityObjectList);
        PLATEAUAutomationTestUtil::CreateMesh(Mesh, CityObjectList);
        std::shared_ptr<plateau::polygonMesh::Model> Model = PLATEAUAutomationTestUtil::CreateModel(Mesh);

        const auto& Actor = PLATEAUAutomationTestUtil::CreateActor(*GetWorld());
        const auto LoadData = PLATEAUAutomationTestUtil::CreateLoadInputData(plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject);
        const ConvertGranularity ConvGranularity = ConvertGranularity::PerPrimaryFeatureObject;
        TMap<FString, FPLATEAUCityObject> CityObj = PLATEAUAutomationTestUtil::CreateCityObjectMap();

        auto FoundItem = Actor->FindComponentByTag<UPLATEAUCityObjectGroup>(PLATEAUAutomationTestUtil::TEST_OBJ_TAG);
        FoundItem->SerializeCityObject(PLATEAUAutomationTestUtil::GetObjNode(Model), CityObj[PLATEAUAutomationTestUtil::TEST_OBJ_NAME]);

        TAtomic<bool> bCanceled;
        bCanceled.Store(false);
        FPLATEAUMeshLoaderForReconstruct MeshLoader;

        MeshLoader.ReloadComponentFromModel(Model, ConvGranularity, CityObj, *Actor);

        AddInfo(FString(Model->debugString().c_str()));

        GEngine->BroadcastLevelActorListChanged();

        //Assertions
        const auto DefaultSceneRootComp = Actor->GetRootComponent();
        const auto RootComp = DefaultSceneRootComp->GetAttachChildren()[0];
        const auto LodComp = RootComp->GetAttachChildren()[0];
        const auto ObjComps = LodComp->GetAttachChildren();

        TestEqual("Obj should be 2", ObjComps.Num() , 2 );
        for (auto Obj : ObjComps) {
            TestEqual("Original Compoenent Name = Node Name", FPLATEAUComponentUtil::GetOriginalComponentName(Obj), PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

            UPLATEAUCityObjectGroup* CityObjGrp = StaticCast<UPLATEAUCityObjectGroup*>(Obj);
            TestEqual("GmlID = Node Name", CityObjGrp->GetAllRootCityObjects()[0].GmlID, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

            //分割・結合で新規に生成されたComponent
            if (CityObjGrp->GetName() == PLATEAUAutomationTestUtil::TEST_OBJ_NAME + "__2") {
                const int32 NumIndices = (int32)Mesh.getIndices().size();
                CityObjGrp->OnStaticMeshChanged().AddLambda([&, NumIndices](UStaticMeshComponent* Comp) {

                    TestNotNull("Component StaticMesh is not null ", Comp->GetStaticMesh().Get());
                    TestEqual("Vertex size are the same", Comp->GetStaticMesh()->GetNumVertices(0), NumIndices);
                });

            }
        }
        return true;
    }));
    return true;
}
