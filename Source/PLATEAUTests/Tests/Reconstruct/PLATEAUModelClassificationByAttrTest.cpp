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
#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <PLATEAUExportSettings.h>
#include <PLATEAUMeshExporter.h>


namespace FPLATEAUTest_Reconstruct_ModelClassificationByAttr_Local {

    TMap<FString, UMaterialInterface*> CreateMaterialMap() {
        TMap<FString, UMaterialInterface*> Materials;
        FString SourcePath = TEXT("/PLATEAU-SDK-for-Unreal/Materials/Fallback/PlateauDefaultDisasterMaterialInstance");
        UMaterialInstance* Materail = Cast<UMaterialInstance>(
            StaticLoadObject(UMaterialInstance::StaticClass(), nullptr, *SourcePath));
        Materials.Add("TestAttrValue", Materail);
        return Materials;
    }

    /// <summary>
    /// FPLATEAUCityObject & GMLID : FPLATEAUCityObject のMap 生成
    /// </summary>
    void CreateCityObjectWithAttr(FPLATEAUCityObject& CityObj) {   
        CityObj.SetGmlID(PLATEAUAutomationTestUtil::TEST_OBJ_NAME);
        CityObj.SetCityObjectsType(PLATEAUAutomationTestUtil::TEST_CITYOBJ_TYPE);
        CityObj.SetCityObjectIndex(plateau::polygonMesh::CityObjectIndex(0, -1));
        TMap<FString, FPLATEAUAttributeValue> AttrMap;
        FPLATEAUAttributeValue Value;
        Value.SetType("String");
        Value.SetValue(EPLATEAUAttributeType::String, "TestAttrValue");
        AttrMap.Add("TestAttr", Value);
        CityObj.SetAttribute(AttrMap);
    }

    void CreateCityObjectWall(FPLATEAUCityObject& CityObj) {
        CityObj.SetGmlID(PLATEAUAutomationTestUtil::TEST_CITYOBJ_WALL_NAME);
        CityObj.SetCityObjectsType(PLATEAUAutomationTestUtil::TEST_CITYOBJ_WALL_TYPE);
        CityObj.SetCityObjectIndex(plateau::polygonMesh::CityObjectIndex(0, 1));
    }

    void CreateCityObjectRoof(FPLATEAUCityObject& CityObj) {
        CityObj.SetGmlID(PLATEAUAutomationTestUtil::TEST_CITYOBJ_ROOF_NAME);
        CityObj.SetCityObjectsType(PLATEAUAutomationTestUtil::TEST_CITYOBJ_ROOF_TYPE);
        CityObj.SetCityObjectIndex(plateau::polygonMesh::CityObjectIndex(0, 2));
    }
}

/// <summary>
/// 属性によるマテリアル分け Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Reconstruct_ModelClassificationByAttr, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.ClassificationByAttr", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)


bool FPLATEAUTest_Reconstruct_ModelClassificationByAttr::RunTest(const FString& Parameters) {
    InitializeTest("ClassificationByAttr");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    ///AtomicのComponentでテスト
    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([&,this] {

        //Required Paramters
        FString AttributeKey = "TestAttr";
        TMap<FString, UMaterialInterface*> Materials = FPLATEAUTest_Reconstruct_ModelClassificationByAttr_Local::CreateMaterialMap();
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
                FPLATEAUTest_Reconstruct_ModelClassificationByAttr_Local::CreateCityObjectWall(CityObj);
                ChildCompConv->SerializeCityObject(CityObj, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

                auto StaticMesh1 = PLATEAUAutomationTestUtil::CreateStaticMesh(ModelActor, FName(TEXT("TestMesh1")), FVector3f(-100, -100, 0));
                PLATEAUAutomationTestUtil::SetMaterial(StaticMesh1, FVector3f(1, 0, 0));
                ChildCompConv->SetStaticMesh(StaticMesh1);
                WallCreated = true;
            }
            else {
                //Roof
                FPLATEAUCityObject CityObj;
                FPLATEAUTest_Reconstruct_ModelClassificationByAttr_Local::CreateCityObjectRoof(CityObj);
                ChildCompConv->SerializeCityObject(CityObj, PLATEAUAutomationTestUtil::TEST_OBJ_NAME);

                auto StaticMesh2 = PLATEAUAutomationTestUtil::CreateStaticMesh(ModelActor, FName(TEXT("TestMesh2")), FVector3f(100, 100, 0));
                PLATEAUAutomationTestUtil::SetMaterial(StaticMesh2, FVector3f(0, 1, 0));
                ChildCompConv->SetStaticMesh(StaticMesh2);
            }  
        }
    
        //Serialize CityObject
        FPLATEAUCityObject CityObj;
        FPLATEAUTest_Reconstruct_ModelClassificationByAttr_Local::CreateCityObjectWithAttr(CityObj);
        TargetComponent->SerializeCityObject(CityObj, "", PLATEAUAutomationTestUtil::CreateCityObjectBuildingOutsideChildren());
        TargetComponent->SetConvertGranularity(ConvertGranularity::PerPrimaryFeatureObject);

        //Atomicなので子をターゲットに加える
        TArray<UPLATEAUCityObjectGroup*> TargetComponents = { (UPLATEAUCityObjectGroup*)TargetComponent->GetChildComponent(0), (UPLATEAUCityObjectGroup*)TargetComponent->GetChildComponent(1) };

        //Model Creation Test
        for(const auto& Target : TargetComponents )
            PLATEAUAutomationTestUtil::TestConvertModel(this, ModelActor, Target, ConvertGranularity::PerPrimaryFeatureObject);

        //Classification
        FPLATEAUModelClassificationByAttribute ModelClassification(ModelActor, AttributeKey, Materials);
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
