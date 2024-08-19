// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "../PLATEAUAutomationTestUtil.h"
#include "Reconstruct/PLATEAUModelClassificationByType.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>

namespace FPLATEAUTest_Reconstruct_ModelClassificationByType_Local {

    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> CreateMaterialMap() {
        TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials;
        FString SourcePath = TEXT("/PLATEAU-SDK-for-Unreal/Materials/Fallback/PlateauDefaultDisasterMaterialInstance");
        UMaterialInstance* Material = Cast<UMaterialInstance>(
            StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *SourcePath));
        Materials.Add(EPLATEAUCityObjectsType::COT_Building, Material);
        return Materials;
    }

}

/// <summary>
/// タイプによるマテリアル分け Test
/// FPLATEAUModelClassificationByType　単体テスト
/// 各Component生成によるTest
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Reconstruct_ModelClassificationByType, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.Classification.Model.ClassificationByType", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_Reconstruct_ModelClassificationByType::RunTest(const FString& Parameters) {
    InitializeTest("Model.ClassificationByType");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");


    ///AtomicのComponentでテスト
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([&, this] {

        //Required Paramters
        TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials = FPLATEAUTest_Reconstruct_ModelClassificationByType_Local::CreateMaterialMap();
        APLATEAUInstancedCityModel* ModelActor = PLATEAUAutomationTestUtil::CreateActorAtomic(*GetWorld());

        //Target Component
        auto TargetComponent = ModelActor->FindComponentByTag<UPLATEAUCityObjectGroup>(PLATEAUAutomationTestUtil::TEST_OBJ_TAG);
        TArray<USceneComponent*> ChildrenComps;
        TargetComponent->GetChildrenComponents(true, ChildrenComps);
        bool WallCreated = false;
        for (const auto& ChildComp : ChildrenComps) {
            UPLATEAUCityObjectGroup* ChildCompConv = (UPLATEAUCityObjectGroup*)ChildComp;

            if (!WallCreated) {
                //Wall
                FPLATEAUCityObject CityObj;
                PLATEAUAutomationTestUtil::CreateCityObjectWall(CityObj);
                ChildCompConv->SerializeCityObject(CityObj, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

                auto StaticMesh1 = PLATEAUAutomationTestUtil::CreateStaticMesh(ModelActor, FName(TEXT("TestMesh1")), FVector3f(-100, -100, 0));
                PLATEAUAutomationTestUtil::SetMaterial(StaticMesh1, FVector3f(1, 0, 0));
                ChildCompConv->SetStaticMesh(StaticMesh1);
                WallCreated = true;
            }
            else {
                //Roof
                FPLATEAUCityObject CityObj;
                PLATEAUAutomationTestUtil::CreateCityObjectRoof(CityObj);
                ChildCompConv->SerializeCityObject(CityObj, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

                auto StaticMesh2 = PLATEAUAutomationTestUtil::CreateStaticMesh(ModelActor, FName(TEXT("TestMesh2")), FVector3f(100, 100, 0));
                PLATEAUAutomationTestUtil::SetMaterial(StaticMesh2, FVector3f(0, 1, 0));
                ChildCompConv->SetStaticMesh(StaticMesh2);
            }
        }

        //Serialize CityObject
        FPLATEAUCityObject CityObj;
        TargetComponent->SerializeCityObject(CityObj, "", PLATEAUAutomationTestUtil::CreateCityObjectBuildingOutsideChildren());
        TargetComponent->SetConvertGranularity(ConvertGranularity::PerPrimaryFeatureObject);

        //Atomicなので子をターゲットに加える
        TArray<UPLATEAUCityObjectGroup*> TargetComponents = { (UPLATEAUCityObjectGroup*)TargetComponent->GetChildComponent(0), (UPLATEAUCityObjectGroup*)TargetComponent->GetChildComponent(1) };

        //Model Creation Test
        for (const auto& Target : TargetComponents)
            PLATEAUAutomationTestUtil::TestConvertModel(this, ModelActor, Target, ConvertGranularity::PerPrimaryFeatureObject);

        //Classification
        FPLATEAUModelClassificationByType ModelClassification(ModelActor, Materials);
        ModelClassification.SetConvertGranularity(ConvertGranularity::PerPrimaryFeatureObject);
        const auto& Converted = ModelClassification.ConvertModelForReconstruct(TargetComponents);
        const auto ResultComponents = ModelClassification.ReconstructFromConvertedModel(Converted);

        AddInfo("Converted Model: " + FString(Converted->debugString().c_str()));

        //auto SubMeshes = Converted->getRootNodeAt(0).getChildAt(0).getChildAt(0).getMesh()->getSubMeshes();
        //AddInfo("Material ID : " + FString::FromInt(SubMeshes[0].getGameMaterialID()));

        //Atomic => PrimaryなのでTargetと同階層に新規に１つ生成され2つ
        TestEqual("TargetComponent and Newly Created Component", TargetComponent->GetAttachParent()->GetNumChildrenComponents(), 2);

        for (auto Comp : ResultComponents) {
            AddInfo("Created Component " + Comp->GetName());
            TestEqual("Created Components Name matches input Target Components", FPLATEAUComponentUtil::GetOriginalComponentName(Comp), FPLATEAUComponentUtil::GetOriginalComponentName(TargetComponent));
        }
        return true;
        }));
    return true;
}
