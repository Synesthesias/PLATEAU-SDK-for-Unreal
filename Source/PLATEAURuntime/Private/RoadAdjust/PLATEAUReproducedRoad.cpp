// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/PLATEAUReproducedRoad.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnLane.h"

APLATEAUReproducedRoad::APLATEAUReproducedRoad() {
    CreateLineTypeMap();

    auto SceneRootComponent = CreateDefaultSubobject<UPLATEAUSceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
    SceneRootComponent->SetMobility(EComponentMobility::Static);
    SceneRootComponent->RegisterComponent();
    this->SetRootComponent(SceneRootComponent);
    this->AddInstanceComponent(SceneRootComponent);
}

/**
* @brief LineTypeMap生成
*
* 各LineTypeに応じたAssetパラメータ設定
*/
void APLATEAUReproducedRoad::CreateLineTypeMap() {
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

    FPLATEAURoadLineParam DashedWhilteLine{ EPLATEAURoadLineType::DashedWhilteLine, LineMesh, WhiteMaterial, 100.0f , 0.3f, 90.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::DashedWhilteLine, DashedWhilteLine);

    FPLATEAURoadLineParam StopLine{ EPLATEAURoadLineType::StopLine, LineMesh, WhiteMaterial, 0.0f , 1.2f, 0.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::StopLine, StopLine);

    FPLATEAURoadLineParam Crossing{ EPLATEAURoadLineType::Crossing, TileMesh, WhiteMaterial, 80.0f , 1.5f, 50.0f };
    LineTypeMap.Add(EPLATEAURoadLineType::Crossing, Crossing);
}

void APLATEAUReproducedRoad::CreateRoadMarks(APLATEAURnStructureModel* Model) {

    const auto& RnModel = Model->Model;
    const auto& Roads = RnModel->GetRoads();

    for (const auto& Road : Roads) {

        const auto& Lanes = Road->GetAllLanes();
        for (const auto& Lane : Lanes) {

            //Leftway
            const auto& Way = Lane->GetLeftWay();
            if (Way != nullptr)                 
            {
                TArray<FVector> LinePoints;
                const auto& itr = Way->GetVertices();
                for (auto it = itr.begin(); it != itr.end(); ++it) {
                    LinePoints.Add(*it);
                }
                CreateLineComponentByType(EPLATEAURoadLineType::DashedWhilteLine, LinePoints, FVector2D(-50.0f, 0.f));
            }

            //Rightway
            const auto& RightWay = Lane->GetRightWay();
            if (RightWay != nullptr) {
                TArray<FVector> LinePoints;
                const auto& itr = RightWay->GetVertices();
                for (auto it = itr.begin(); it != itr.end(); ++it) {
                    LinePoints.Add(*it);
                }
                CreateLineComponentByType(EPLATEAURoadLineType::WhiteLine, LinePoints, FVector2D(50.0f, 0.f));
            }

            //Centerway
            const auto& CenterWay = Lane->GetCenterWay();
            if (CenterWay != nullptr) {
                TArray<FVector> LinePoints;
                const auto& itr = CenterWay->GetVertices();
                for (auto it = itr.begin(); it != itr.end(); ++it) {
                    LinePoints.Add(*it);
                }
                CreateLineComponentByType(EPLATEAURoadLineType::YellowLine, LinePoints);
            }
        }

        //Borders
        const auto& Borders =  Road->GetBorders();
        for (const auto& Border : Borders) {

            if (Border != nullptr) {

                TArray<FVector> LinePoints;
                const auto& itr = Border->GetVertices();
                for (auto it = itr.begin(); it != itr.end(); ++it) {
                    LinePoints.Add(*it);
                }

                if(LinePoints.Num() <= 2)
                    CreateLineComponentByType(EPLATEAURoadLineType::StopLine, LinePoints);
                else
                    CreateLineComponentByType(EPLATEAURoadLineType::Crossing, LinePoints);
            }
        }
    }
}

void APLATEAUReproducedRoad::CreateLineComponentByType(EPLATEAURoadLineType Type, TArray<FVector> LinePoints, FVector2D Offset) {
    const auto& Param = LineTypeMap[Type];
    const auto& Component = NewObject<ULineGeneratorComponent>(this, FName(TEXT("LineGeneratorComponent_") + StaticEnum<EPLATEAURoadLineType>()->GetDisplayValueAsText(Type).ToString() + TEXT("_") + FString::FromInt(NumComponents)));
    Component->RegisterComponent();
    this->AddInstanceComponent(Component);
    Component->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);  
    Component->CreateSplineFromVectorArray(LinePoints);
    Component->Offset = Offset;
    Component->CreateSplineMeshFromAssets(this, Param.LineMesh, Param.LineMaterial, Param.LineGap, Param.LineXScale, Param.LineLength);
    NumComponents++;
}

void APLATEAUReproducedRoad::BeginPlay() {
    Super::BeginPlay();
}

void APLATEAUReproducedRoad::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}