// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "../PLATEAUAutomationTestUtil.h"
#include <Reconstruct/PLATEAUModelLandscape.h>
#include <Reconstruct/PLATEAUMeshLoaderForLandscapeMesh.h>
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>
#include <ImageUtils.h>

/// <summary>
/// Heightmap to Landscape Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Heightmap_Landscape, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.Terrain.Heightmap.Landscape", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_Heightmap_Landscape::RunTest(const FString& Parameters) {
    InitializeTest("Terrain.Heightmap.Landscape");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    UTexture2D* Texture = PLATEAUAutomationTestUtil::Texture::LoadImage("HM_dem_test_505_505.png");
    TestNotNull("Texture Load", Texture);

    AddInfo("Pixel Format " + PLATEAUAutomationTestUtil::Texture::GetPixelFormatString(Texture));

    TArray<uint16> PixelDataArray = PLATEAUAutomationTestUtil::Texture::ConvertTexture2dToUint16Array(Texture);
    TestEqual("Pixel size ", PixelDataArray.Num(), 505 * 505 );

    FPLATEAUModelLandscape ModelLandscape;
    ALandscape* Land = ModelLandscape.CreateLandScape(GetWorld(), 2, 63, 126, 126, 505, 505, TVec3d(), TVec3d(1000,1000,100), TVec2f(), TVec2f(1, 1), "", PixelDataArray, "TestLandscape");

    //Assertions
    TestEqual("Landscape Name", Land->GetActorNameOrLabel(), "TestLandscape");

    const auto& Info = Land->GetLandscapeInfo();   
    TestEqual("SubsectionSizeQuads", Info->SubsectionSizeQuads, 63);
    TestEqual("NumSubsections", Info->ComponentNumSubsections, 2);

    return true;
}

/// <summary>
/// Heightmap to Landscape Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Heightmap_LandscapeMesh, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.Terrain.Heightmap.LandscapeMesh", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_Heightmap_LandscapeMesh::RunTest(const FString& Parameters) {
    InitializeTest("Terrain.Heightmap.LandscapeMesh");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    const auto& Actor = PLATEAUAutomationTestUtil::LandscapeFixtures::CreateActor(*GetWorld());
    //CityObjectGroup Item
    auto OriginalItem = Actor->FindComponentByTag<UPLATEAUCityObjectGroup>(PLATEAUAutomationTestUtil::Fixtures::TEST_OBJ_TAG);
    FPLATEAUCityObject CityObj;
    PLATEAUAutomationTestUtil::LandscapeFixtures::CreateCityObjectDem(CityObj);
    OriginalItem->SerializeCityObject(CityObj);
    auto StaticMesh = PLATEAUAutomationTestUtil::Fixtures::CreateStaticMesh(Actor, FName(TEXT("TestDemStaticMesh")));
    OriginalItem->SetStaticMesh(StaticMesh);
    PLATEAUAutomationTestUtil::Fixtures::SetMaterial(StaticMesh, FVector3f(0, 1, 0));

    // Load Heightmap Texture
    UTexture2D* Texture = PLATEAUAutomationTestUtil::Texture::LoadImage("HM_dem_test_505_505.png");
    TestNotNull("Texture Load", Texture);

    AddInfo("Pixel Format " + PLATEAUAutomationTestUtil::Texture::GetPixelFormatString(Texture));

    TArray<uint16> PixelDataArray = PLATEAUAutomationTestUtil::Texture::ConvertTexture2dToUint16Array(Texture);
    TestEqual("Pixel size ", PixelDataArray.Num(), 505 * 505);

    uint16_t* RawData = PixelDataArray.GetData();

    // Convert to Mesh
    FPLATEAUMeshLoaderForLandscapeMesh MeshLoader;
    MeshLoader.CreateMeshFromHeightMap(*Actor, 505, 505, TVec3d(), TVec3d(1000, 1000, 100), TVec2f(), TVec2f(1, 1), RawData, PLATEAUAutomationTestUtil::LandscapeFixtures::TEST_DEM_OBJ_NAME);

    // Assertions
    const auto& Parent = OriginalItem->GetAttachParent();
    TestEqual("New Component Created ", Parent->GetNumChildrenComponents(), 2);

    TArray<USceneComponent*> Children;
    Parent->GetChildrenComponents(false, Children);

    auto MeshComponentPtr = Children.FindByPredicate([OriginalItem](USceneComponent* Comp) {
        return Comp->GetName() == "Mesh_" + OriginalItem->GetName();
        });
    TestNotNull("Has Mesh Component ", MeshComponentPtr);

    //Created Terrain Mesh
    auto MeshComponent = (UPLATEAUCityObjectGroup*)*MeshComponentPtr; 

    TestEqual("Attr are the same ", MeshComponent->SerializedCityObjects, OriginalItem->SerializedCityObjects);

    // Static Mesh　生成まで待機
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, MeshComponent, OriginalItem] {
        if (!MeshComponent->GetStaticMesh())
            return false;

        TestEqual("Material are the same ", MeshComponent->GetStaticMesh()->GetMaterial(0), OriginalItem->GetStaticMesh()->GetMaterial(0));
        AddInfo("StaticMesh Test Finish");

        return true;
        }));


    return true;
}
