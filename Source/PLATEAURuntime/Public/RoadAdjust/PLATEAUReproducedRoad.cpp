// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/PLATEAUReproducedRoad.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnLane.h"

using namespace UE::Tasks;

APLATEAUReproducedRoad::APLATEAUReproducedRoad() {

    const TCHAR* LineMeshPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Meshes/simple_line");
    const auto LineMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, LineMeshPath));
    const TCHAR* TileMeshPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Meshes/simple_tile");
    const auto TileMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TileMeshPath));

    const TCHAR* WhiteMaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Materials/WhiteLineMaterial");
    const auto WhiteMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, WhiteMaterialPath));
    const TCHAR* YellowMaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Materials/YellowLineMaterial");
    const auto YellowMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, YellowMaterialPath));


    FPLATEAURoadLineParam WhiteLine{ EPLATEAURoadLineType::WhiteLine, LineMesh, WhiteMaterial, 0.0f , 0.3f, 0.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::WhiteLine, WhiteLine);

    FPLATEAURoadLineParam YellowLine{ EPLATEAURoadLineType::YellowLine, LineMesh, YellowMaterial, 0.0f , 0.3f, 0.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::YellowLine, YellowLine);

    FPLATEAURoadLineParam DashedWhilteLine{ EPLATEAURoadLineType::DashedWhilteLine, LineMesh, WhiteMaterial, 50.0f , 0.3f, 90.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::DashedWhilteLine, DashedWhilteLine);

    FPLATEAURoadLineParam StopLine{ EPLATEAURoadLineType::StopLine, LineMesh, WhiteMaterial, 0.0f , 1.2f, 0.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::StopLine, StopLine);

    FPLATEAURoadLineParam Crossing{ EPLATEAURoadLineType::Crossing, TileMesh, WhiteMaterial, 80.0f , 1.5f, 50.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::Crossing, Crossing);
}

void APLATEAUReproducedRoad::CreateRoadFromAttr(APLATEAUInstancedCityModel* Model, const TArray<USceneComponent*> TargetComponents, const FString AttributeKey, TMap<FString, UMaterialInterface*> Materials)
{

    const auto& SceneRoot = NewObject<USceneComponent>(this,
        USceneComponent::GetDefaultSceneRootVariableName());
    this->AddInstanceComponent(SceneRoot);
    this->SetRootComponent(SceneRoot);
    SceneRoot->RegisterComponent();
    SceneRoot->SetMobility(EComponentMobility::Static);


    FTask CreateRoadTask = Launch(TEXT("CreateRoadTask"), [this, SceneRoot, Model, TargetComponents, AttributeKey, Materials] {
        auto InnerTask = Model->ClassifyModel(TargetComponents, AttributeKey, Materials, EPLATEAUMeshGranularity::DoNotChange, false);
        InnerTask.Wait();


        for (auto& TargetComponent : TargetComponents) {
            TargetComponent->SetVisibility(true);
        }

        TArray<USceneComponent*> CreatedComponents = InnerTask.GetResult();



        //Static Mesh生成まで待機したいが動作しない？？

        int32 NumStaticMeshes = 0;
        int32 NumStaticMeshBuilt = 0;

        while (NumStaticMeshes > 0 && NumStaticMeshes != NumStaticMeshBuilt) 
        {
            for (auto& CreatedComponent : CreatedComponents) {
                if (CreatedComponent->IsA(UStaticMeshComponent::StaticClass())) {

                    NumStaticMeshes++;

                    if (StaticCast<UStaticMeshComponent*>(CreatedComponent)->GetStaticMesh() == nullptr) {
                        UE_LOG(LogTemp, Log, TEXT("StaticMesh is nullptr"));
                        NumStaticMeshBuilt++;
                    }

                }
            }
        }


        FFunctionGraphTask::CreateAndDispatchWhenReady([SceneRoot, CreatedComponents]() {

            for (auto& CreatedComponent : CreatedComponents) {

                UE_LOG(LogTemp, Log, TEXT("Created Comp: %s"), *CreatedComponent->GetName());

                CreatedComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
                //this->AddInstanceComponent(CreatedComponent);
                bool attached = CreatedComponent->AttachToComponent(SceneRoot, FAttachmentTransformRules::KeepWorldTransform);

                CreatedComponent->RegisterComponent();

                UE_LOG(LogTemp, Log, TEXT("Attach Success: %s , Parent : %s"), attached? TEXT("True") : TEXT("False"), *CreatedComponent->GetAttachmentRootActor()->GetName());

                //なぜかAttachされない
            }

        }, TStatId(), NULL, ENamedThreads::GameThread)->Wait();


    });
}

void APLATEAUReproducedRoad::GetVectors(TArray<FVector>& Vectors, URnWay* Way) const
{
    const auto& VertsItr = Way->GetVertices();
    for (auto it = VertsItr.begin(); it != VertsItr.end(); ++it) {
        //FVector Vec = *it;
        //Vectors.Add(Vec);
        Vectors.Add(*it);
    }
}


void APLATEAUReproducedRoad::CreateRoadMarks(URnModel* Model) {

    //const TCHAR* StaticMeshPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/simple_line");
    //const auto StaticMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, StaticMeshPath));

    ////const TCHAR* MaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/WhiteLineMaterial");
    //const TCHAR* MaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/YellowLineMaterial");
    //const auto MaterialInterface = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), NULL, MaterialPath));

    //float Gap = 100.f;

    int index = 0;

    const auto& Roads = Model->GetRoads();

    for (const auto& Road : Roads) {

        const auto& Lanes = Road->GetAllLanes();

        for (const auto& Lane : Lanes) {

            const auto& Way = Lane->GetLeftWay();
            //const auto& Way = Lane->GetRightWay();
            if (Way != nullptr)                 
            {
                TArray<FVector> LinePoints;
                //GetVectors(LinePoints, Way);
                const auto& VertsItr = Way->GetVertices();
                for (auto it = VertsItr.begin(); it != VertsItr.end(); ++it) {
                    FVector Vec = *it;
                    LinePoints.Add(Vec);
                }

                //const auto& Component = NewObject<ULineGeneratorComponent>(this, FName(TEXT("LineGeneratorComponent") + FString::FromInt(index)));
                ////SplineMeshRoot->SetMobility(EComponentMobility::Static);
                //this->AddInstanceComponent(Component);
                //Component->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
                //Component->RegisterComponent();

                //Component->CreateSplineFromVectorArray(LinePoints);
                //Component->CreateSplineMeshFromAssets(this, StaticMesh, MaterialInterface, Gap);

                CreateLineComponentByType(EPLATEAURoadLineType::DashedWhilteLine, LinePoints, index);

                index++;
            }

            const auto& RightWay = Lane->GetRightWay();
            if (RightWay != nullptr) {
                TArray<FVector> LinePoints;
                //GetVectors(LinePoints, Way);
                const auto& VertsItr = RightWay->GetVertices();
                for (auto it = VertsItr.begin(); it != VertsItr.end(); ++it) {
                    FVector Vec = *it;
                    LinePoints.Add(Vec);
                }
                CreateLineComponentByType(EPLATEAURoadLineType::YellowLine, LinePoints, index);

                index++;
            }

        }

        const auto& Borders =  Road->GetBorders();

        UE_LOG(LogTemp, Log, TEXT("Borders: %d"), Borders.Num());

        for (const auto& Border : Borders) {

            if (Border != nullptr) {

                TArray<FVector> LinePoints;
                //GetVectors(LinePoints, Border);
                const auto& VertsItr = Border->GetVertices();
                for (auto it = VertsItr.begin(); it != VertsItr.end(); ++it) {
                    LinePoints.Add(*it);
                }

                if(LinePoints.Num() <= 2)
                    CreateLineComponentByType(EPLATEAURoadLineType::StopLine, LinePoints, index);
                else
                    CreateLineComponentByType(EPLATEAURoadLineType::Crossing, LinePoints, index);

                index++;
            }

        }

    }
}

void APLATEAUReproducedRoad::CreateLineComponentByType(EPLATEAURoadLineType type, TArray<FVector> LinePoints, int32 index) {

    const auto& Param = LineTypeMap[type];

    const auto& Component = NewObject<ULineGeneratorComponent>(this, FName(TEXT("LineGeneratorComponent") + FString::FromInt(index)));
    //SplineMeshRoot->SetMobility(EComponentMobility::Static);
    this->AddInstanceComponent(Component);
    Component->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    Component->RegisterComponent();

    Component->CreateSplineFromVectorArray(LinePoints);
    Component->CreateSplineMeshFromAssets(this, Param.LineMesh, Param.LineMaterial, Param.LineGap, Param.LineXScale, Param.LineLength);

}


void APLATEAUReproducedRoad::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUReproducedRoad::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}