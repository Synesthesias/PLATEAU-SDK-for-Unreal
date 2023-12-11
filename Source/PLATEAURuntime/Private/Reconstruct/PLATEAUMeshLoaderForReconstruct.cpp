// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"
#include "PLATEAUCityModelLoader.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "citygml/citygml.h"
#include "PLATEAUCityObjectGroup.h"
#include "Engine/StaticMesh.h"

void FPLATEAUMeshLoaderForReconstruct::ReloadComponentFromNode(
    USceneComponent* InParentComponent,
    const plateau::polygonMesh::Node& InNode,
    plateau::polygonMesh::MeshGranularity Granularity,
    TMap<FString, FPLATEAUCityObject> cityObjMap,
    AActor& InActor) {

    CityObjMap = cityObjMap;
    LastCreatedComponents.Empty();

    ReloadNodeRecursive(InParentComponent, InNode, Granularity, InActor);

    // メッシュをワールド内にビルド
    const auto CopiedStaticMeshes = StaticMeshes;
    FFunctionGraphTask::CreateAndDispatchWhenReady(
        [CopiedStaticMeshes]() {
            UStaticMesh::BatchBuild(CopiedStaticMeshes, true, [](UStaticMesh* mesh) {return false; });
        }, TStatId(), nullptr, ENamedThreads::GameThread)->Wait();
        StaticMeshes.Reset();
}

void FPLATEAUMeshLoaderForReconstruct::ReloadNodeRecursive(
    USceneComponent* InParentComponent,
    const plateau::polygonMesh::Node& InNode,
    plateau::polygonMesh::MeshGranularity Granularity,
    AActor& InActor) {
    const auto Component = ReloadNode(InParentComponent, InNode, Granularity, InActor);
    const size_t ChildNodeCount = InNode.getChildCount();
    for (int i = 0; i < ChildNodeCount; i++) {
        const auto& TargetNode = InNode.getChildAt(i);
        ReloadNodeRecursive(Component, TargetNode, Granularity, InActor);
    }
}

USceneComponent* FPLATEAUMeshLoaderForReconstruct::ReloadNode(USceneComponent* ParentComponent,
    const plateau::polygonMesh::Node& Node,
    plateau::polygonMesh::MeshGranularity Granularity,
    AActor& Actor) {
    if (Node.getMesh() == nullptr || Node.getMesh()->getVertices().size() == 0) {
        USceneComponent* Comp = nullptr;
        UClass* StaticClass;
        const FString DesiredName = FString(UTF8_TO_TCHAR(Node.getName().c_str()));
        const FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([&, DesiredName] {

            // すでに同名、同階層のComponentが存在する場合は再利用
            if (const auto ExistComponent = FindChildComponentWithOriginalName(ParentComponent, DesiredName)) {
                Comp = ExistComponent;
                return;
            }

            StaticClass = UPLATEAUCityObjectGroup::StaticClass();
            const auto& PLATEAUCityObjectGroup = NewObject<UPLATEAUCityObjectGroup>(&Actor, NAME_None);
            Comp = PLATEAUCityObjectGroup;

            auto cityObjRef = CityObjMap.Find(DesiredName);
            if (cityObjRef != nullptr) {
                const FPLATEAUCityObject cityObj = *cityObjRef;
                PLATEAUCityObjectGroup->SerializeCityObject(Node, cityObj);
            }

            const FString NewUniqueName = MakeUniqueGmlObjectName(&Actor, StaticClass, DesiredName);
            Comp->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);

            check(Comp != nullptr);
            if (bAutomationTest) {
                Comp->Mobility = EComponentMobility::Movable;
            }
            else {
                Comp->Mobility = EComponentMobility::Static;
            }

            Actor.AddInstanceComponent(Comp);
            Comp->RegisterComponent();
            Comp->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
            }, TStatId(), nullptr, ENamedThreads::GameThread);
        Task->Wait();
        return Comp;
    }

    plateau::polygonMesh::MeshExtractOptions MeshExtractOptions{};
    MeshExtractOptions.mesh_granularity = Granularity;
    FLoadInputData LoadInputData
    {
        MeshExtractOptions,
        std::vector<plateau::geometry::Extent>{},
        FString(),
        false,
        nullptr
    };
    return CreateStaticMeshComponent(Actor, *ParentComponent, *Node.getMesh(), LoadInputData, nullptr,
        Node.getName(), true);
}


