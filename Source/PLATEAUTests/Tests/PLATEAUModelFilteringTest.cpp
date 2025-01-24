// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUModelFiltering.h"
#include "Component/PLATEAUSceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>

namespace FPLATEAUTest_Filter_ModelFiltering_Local {

    AActor* CreateComponentHierarchy(UWorld& World) {
        //Component
        FActorSpawnParameters SpawnParam;
        const auto& Actor = World.SpawnActor<AActor>(SpawnParam);

        const auto& SceneRoot = NewObject<UPLATEAUSceneComponent>(Actor,
            USceneComponent::GetDefaultSceneRootVariableName());
        const auto& CompRoot = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(TEXT("0000_op")));
        const auto& CompLod = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(TEXT("Lod0")));
        const auto& CompObj = NewObject<UPLATEAUCityObjectGroup>(Actor,
            FName(TEXT("obj_00000")));
        const auto& CompLod1 = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(TEXT("Lod1")));
        const auto& CompObj1 = NewObject<UPLATEAUCityObjectGroup>(Actor,
            FName(TEXT("obj_00000")));
        const auto& CompLod2 = NewObject<UPLATEAUSceneComponent>(Actor,
            FName(TEXT("Lod2")));
        const auto& CompObj2 = NewObject<UPLATEAUCityObjectGroup>(Actor,
            FName(TEXT("obj_00000")));


        Actor->AddInstanceComponent(SceneRoot);
        Actor->SetRootComponent(SceneRoot);
        SceneRoot->RegisterComponent();

        Actor->AddInstanceComponent(CompRoot);
        CompRoot->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepWorldTransform);
        CompRoot->RegisterComponent();

        CompLod->AttachToComponent(CompRoot, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompLod);
        CompLod->RegisterComponent();

        CompObj->AttachToComponent(CompLod, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompObj);
        CompObj->RegisterComponent();

        CompLod1->AttachToComponent(CompRoot, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompLod1);
        CompLod1->RegisterComponent();

        CompObj1->AttachToComponent(CompLod1, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompObj1);
        CompObj1->RegisterComponent();

        CompLod2->AttachToComponent(CompRoot, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompLod2);
        CompLod2->RegisterComponent();

        CompObj2->AttachToComponent(CompLod2, FAttachmentTransformRules::KeepWorldTransform);
        Actor->AddInstanceComponent(CompObj2);
        CompObj2->RegisterComponent();

        GEngine->BroadcastLevelActorListChanged();

        return Actor;
    }
}

/// <summary>
/// PLATEAUModelFiltering Test
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_Filter_ModelFiltering, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.ModelAdjustmentFilter.ModelFiltering", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_Filter_ModelFiltering::RunTest(const FString& Parameters) {
    InitializeTest("ModelFiltering");
    
    FPLATEAUModelFiltering Filter;

    //FilterLowLods
    AActor* Actor = FPLATEAUTest_Filter_ModelFiltering_Local::CreateComponentHierarchy(*GetWorld());
    const auto& Root = Actor->GetRootComponent()->GetAttachChildren().GetData();      
    Filter.FilterLowLods(Root->Get(), 2, 2);
    TArray<USceneComponent*> Children;
    Root->Get()->GetChildrenComponents(true, Children);

    for (const auto& Child : Children) {
        if (Child->GetName() == "Lod0") {
            const auto& Objs = Child->GetAttachChildren();
            for(const auto& Obj : Objs)
                TestFalse("Lod0 should be hidden", Obj->IsVisible());
        }     
        else if(Child->GetName() == "Lod1") {
            const auto& Objs = Child->GetAttachChildren();
            for (const auto& Obj : Objs)
                TestFalse("Lod1 should be hidden", Obj->IsVisible());
        }      
        else if (Child->GetName() == "Lod2") {
            const auto& Objs = Child->GetAttachChildren();
            for (const auto& Obj : Objs)
                TestTrue("Lod2 should be visible", Obj->IsVisible());
        }
    }

    return true;
}
