// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"
#include "PLATEAUInstancedCityModel.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Util/PLATEAUComponentUtil.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>
#include "Tasks/Task.h"
#include "PLATEAUModelLandscapeTestEventListener.h"

using namespace UE::Tasks;


/// <summary>
/// 地形平滑化テスト
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Reconstruct_ModelClassification, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.Reconstruct.ModelClassification", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_Reconstruct_ModelClassification::RunTest(const FString& Parameters) {
    InitializeTest("ModelClassification");
    if (!OpenMap("SampleBldg"))
        AddError("Failed to OpenMap");



    return true;
}
