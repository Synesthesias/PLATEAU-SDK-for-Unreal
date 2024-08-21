// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "../PLATEAUAutomationTestUtil.h"
#include "Reconstruct/PLATEAUMeshLoaderForHeightmap.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>

namespace FPLATEAUTest_MeshLoader_Heightmap_Local{

    const FString TEST_DEM_OP_NAME = "000000_dem_0000_op";
    const FString TEST_DEM_OBJ_NAME = "dem_00000000-0000-0000-0000-000000000000";
    const FString TEST_DEM_CITYOBJ_TYPE = "TINRelief";

    /// <summary>
    /// DemのCityObjectIndexのList生成
    /// </summary>
    void CreateCityObjectList(plateau::polygonMesh::CityObjectList& CityObj) {
        CityObj.add(plateau::polygonMesh::CityObjectIndex(0, -1), TCHAR_TO_UTF8(*TEST_DEM_OBJ_NAME));
    }

    /// <summary>
    /// Dem Model / 各Node 生成
    /// </summary>
    std::shared_ptr<plateau::polygonMesh::Model> CreateModel(plateau::polygonMesh::Mesh& Mesh) {
        std::shared_ptr<plateau::polygonMesh::Model> Model = plateau::polygonMesh::Model::createModel();
        auto& NodeOP = Model->addEmptyNode(TCHAR_TO_UTF8(*TEST_DEM_OP_NAME));
        auto& NodeLod = NodeOP.addEmptyChildNode(TCHAR_TO_UTF8(*PLATEAUAutomationTestUtil::TEST_LOD_NAME));
        auto& NodeObj = NodeLod.addEmptyChildNode(TCHAR_TO_UTF8(*TEST_DEM_OBJ_NAME));
        auto MeshPtr = std::make_unique<plateau::polygonMesh::Mesh>(Mesh);
        NodeObj.setMesh(std::move(MeshPtr));
        Model->assignNodeHierarchy();
        Model->optimizeMeshes();
        return Model;
    }

    /// <summary>
    /// Actor と DEM Compoenent生成
    /// </summary>
    APLATEAUInstancedCityModel* CreateActor(UWorld& World) {
        APLATEAUInstancedCityModel* Actor = World.SpawnActor<APLATEAUInstancedCityModel>();
        const auto& SceneRoot = NewObject<UPLATEAUSceneComponent>(Actor,
            USceneComponent::GetDefaultSceneRootVariableName());
        const auto& CompRoot = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(TEST_DEM_OP_NAME));
        const auto& CompLod = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(PLATEAUAutomationTestUtil::TEST_LOD_NAME + "__1"));
        const auto& CompObj = NewObject<UPLATEAUCityObjectGroup>(Actor,
            FName(TEST_DEM_OBJ_NAME + "__1"));

        Actor->SetActorLabel(PLATEAUAutomationTestUtil::TEST_ACTOR_NAME);
        Actor->AddInstanceComponent(SceneRoot);
        Actor->SetRootComponent(SceneRoot);
        SceneRoot->RegisterComponent();

        Actor->AddInstanceComponent(CompRoot);
        CompRoot->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepWorldTransform);
        CompRoot->RegisterComponent();
        CompRoot->SetMobility(EComponentMobility::Static);

        CompLod->AttachToComponent(CompRoot, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompLod);
        CompLod->RegisterComponent();
        CompLod->SetMobility(EComponentMobility::Static);

        CompObj->AttachToComponent(CompLod, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompObj);
        CompObj->RegisterComponent();
        CompObj->SetMobility(EComponentMobility::Static);
        CompObj->ComponentTags.Add(PLATEAUAutomationTestUtil::TEST_OBJ_TAG);

        GEngine->BroadcastLevelActorListChanged();
        return Actor;
    }

    /// <summary>
    /// Dem CityObject
    /// </summary>
    void CreateCityObjectDem(FPLATEAUCityObject& InCityObj) {
        InCityObj.SetGmlID(TEST_DEM_OBJ_NAME);
        InCityObj.SetCityObjectsType(TEST_DEM_CITYOBJ_TYPE);
        InCityObj.SetCityObjectIndex(plateau::polygonMesh::CityObjectIndex(0, -1));
    }

}

/// <summary>
/// Landscape/Heightmap 用 MeshLoader (PLATEAUMeshLoaderForHeightmap) Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_MeshLoader_Heightmap, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.MeshLoader.Heightmap", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_MeshLoader_Heightmap::RunTest(const FString& Parameters) {
    InitializeTest("MeshLoader.Heightmap");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    ///Dynamic 生成処理 ====================================================

    plateau::polygonMesh::Mesh Mesh;
    plateau::polygonMesh::CityObjectList CityObjectList;
    FPLATEAUTest_MeshLoader_Heightmap_Local::CreateCityObjectList(CityObjectList);
    PLATEAUAutomationTestUtil::CreateMesh(Mesh, CityObjectList);
    std::shared_ptr<plateau::polygonMesh::Model> Model = FPLATEAUTest_MeshLoader_Heightmap_Local::CreateModel(Mesh);

    const auto& Actor = FPLATEAUTest_MeshLoader_Heightmap_Local::CreateActor(*GetWorld());
    const auto LoadData = PLATEAUAutomationTestUtil::CreateLandscapeParam(); //505 x 505

    //CityObjectGroup Item
    auto OriginalItem = Actor->FindComponentByTag<UPLATEAUCityObjectGroup>(PLATEAUAutomationTestUtil::TEST_OBJ_TAG);
    FPLATEAUCityObject CityObj;
    FPLATEAUTest_MeshLoader_Heightmap_Local::CreateCityObjectDem(CityObj);
    OriginalItem->SerializeCityObject(PLATEAUAutomationTestUtil::GetObjNode(Model), CityObj, ConvertGranularity::PerPrimaryFeatureObject);
    OriginalItem->SetStaticMesh(PLATEAUAutomationTestUtil::CreateStaticMesh(Actor, FName(TEXT("TestDemStaticMesh"))));
    PLATEAUAutomationTestUtil::SetMaterial(OriginalItem, FVector3f(1,0,0));

    GEngine->BroadcastLevelActorListChanged();

    /// Height Map 生成 ====================================================

    FPLATEAUMeshLoaderForHeightmap MeshLoader;
    auto Results = MeshLoader.CreateHeightMap(Actor, Model, LoadData);

    AddInfo(FString(Model->debugString().c_str()));

    /// Assertions ====================================================

    TestEqual("Landscape Results Num", Results.Num(), 1);

    const HeightmapCreationResult& Result = Results[0];  
    TestEqual("Heightmap size", Result.Data.Get()->size() , 505 * 505 );
    TestEqual("Result NodeName", Result.NodeName, FPLATEAUTest_MeshLoader_Heightmap_Local::TEST_DEM_OBJ_NAME);
    
    return true;
}
