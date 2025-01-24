// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"
#include "Misc/AutomationTest.h"
#include "PLATEAUModelLandscapeTestEventListener.generated.h"

/// <summary>
/// FPLATEAUTest_Reconstruct_ModelLandscape, FPLATEAUTest_Reconstruct_ModelLandscapeMesh,FPLATEAUTest_Reconstruct_ModelLandscapeAlign での 生成後のテスト
/// </summary>
UCLASS()
class ULandscapeLoadEventListener : public UObject {
    GENERATED_BODY()
public:
    FAutomationTestBase* TestBase;
    bool OnCalled = false;
    UFUNCTION(CallInEditor)
    void OnLandscapeLoaded(EPLATEAULandscapeCreationResult Result) {

        TestBase->TestEqual("LandscapeCreationResult Success", Result, EPLATEAULandscapeCreationResult::Success);

        TestBase->AddInfo("ULandscapeLoadEventListener::OnLandscapeLoaded");
        OnCalled = true;
        RemoveFromRoot();
    }
};

