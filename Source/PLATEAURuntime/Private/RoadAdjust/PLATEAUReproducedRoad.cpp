// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/PLATEAUReproducedRoad.h"

#include "RoadAdjust/PLATEAURoadLineType.h"
#include "RoadAdjust/RoadMarking/LineGeneratorComponent.h"
#include "RoadAdjust/RoadMarking/PLATEAUCrosswalkComposer.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"

APLATEAUReproducedRoad::APLATEAUReproducedRoad() {
    CreateLineTypeMap();

    auto SceneRootComponent = CreateDefaultSubobject<UPLATEAUSceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());
    SceneRootComponent->SetMobility(EComponentMobility::Static);
    SceneRootComponent->RegisterComponent();
    this->SetRootComponent(SceneRootComponent);
    this->AddInstanceComponent(SceneRootComponent);
}

void APLATEAUReproducedRoad::CreateLineTypeMap() {
    const TCHAR* LineMeshPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Meshes/simple_line");
    const auto LineMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, LineMeshPath));
    const TCHAR* TileMeshPath = TEXT("/PLATEAU-SDK-for-Unreal/RoadNetwork/Meshes/simple_tile");
    const auto TileMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TileMeshPath));
    
    LineTypeMap.Add(EPLATEAURoadLineType::WhiteLine, PLATEAURoadLineTypeExtension::ToRoadLineParam(EPLATEAURoadLineType::WhiteLine, LineMesh, TileMesh));
    LineTypeMap.Add(EPLATEAURoadLineType::YellowLine, PLATEAURoadLineTypeExtension::ToRoadLineParam(EPLATEAURoadLineType::YellowLine, LineMesh, TileMesh));
    LineTypeMap.Add(EPLATEAURoadLineType::DashedWhilteLine, PLATEAURoadLineTypeExtension::ToRoadLineParam(EPLATEAURoadLineType::DashedWhilteLine, LineMesh, TileMesh));
    LineTypeMap.Add(EPLATEAURoadLineType::StopLine, PLATEAURoadLineTypeExtension::ToRoadLineParam(EPLATEAURoadLineType::StopLine, LineMesh, TileMesh));
    LineTypeMap.Add(EPLATEAURoadLineType::Crossing, PLATEAURoadLineTypeExtension::ToRoadLineParam(EPLATEAURoadLineType::Crossing, LineMesh, TileMesh));
}

void APLATEAUReproducedRoad::CreateRoadMarks(APLATEAURnStructureModel* Model) {

    auto RnModel = Model->Model;

    // ターゲットとなる道路ネットワークの定義
    auto TargetModel = NewObject<UPLATEAURrTargetModel>(GetTransientPackage(), UPLATEAURrTargetModel::StaticClass());
    TargetModel->Initialize(RnModel);

    // 白線生成の対象と種類を定義
    auto WayComposer = NewObject<UPLATEAUMarkedWayListComposerMain>(GetTransientPackage(), UPLATEAUMarkedWayListComposerMain::StaticClass());
    auto MarkedWays = WayComposer->ComposeFrom(TargetModel).GetMarkedWays();

    // 横断歩道
    auto CrossRoads = NewObject<UPLATEAUCrosswalkComposer>()->Compose(*TargetModel, EPLATEAUCrosswalkFrequency::BigRoad);
    MarkedWays.Append(CrossRoads.GetMarkedWays());

    // 白線を生成
    for(const auto& MarkedWay : MarkedWays)
    {
        const auto Points = MarkedWay.GetLine().GetPoints();
        const auto Type = MarkedWay.GetRoadLineType();
        CreateLineComponentByType(Type, Points, FVector2d(0.0f, 0.0f));
    }
    
    // const auto& Roads = RnModel->GetRoads();
    //
    // for (const auto& Road : Roads) {
    //
    //     const auto& Lanes = Road->GetAllLanes();
    //     for (const auto& Lane : Lanes) {
    //
    //         //Leftway
    //         const auto& Way = Lane->GetLeftWay();
    //         if (Way != nullptr)                 
    //         {
    //             TArray<FVector> LinePoints;
    //             const auto& itr = Way->GetVertices();
    //             for (auto it = itr.begin(); it != itr.end(); ++it) {
    //                 LinePoints.Add(*it);
    //             }
    //             CreateLineComponentByType(EPLATEAURoadLineType::DashedWhilteLine, LinePoints, FVector2D(-50.0f, 0.f));
    //         }
    //
    //         //Rightway
    //         const auto& RightWay = Lane->GetRightWay();
    //         if (RightWay != nullptr) {
    //             TArray<FVector> LinePoints;
    //             const auto& itr = RightWay->GetVertices();
    //             for (auto it = itr.begin(); it != itr.end(); ++it) {
    //                 LinePoints.Add(*it);
    //             }
    //             CreateLineComponentByType(EPLATEAURoadLineType::WhiteLine, LinePoints, FVector2D(50.0f, 0.f));
    //         }
    //
    //         //Centerway
    //         const auto& CenterWay = Lane->GetCenterWay();
    //         if (CenterWay != nullptr) {
    //             TArray<FVector> LinePoints;
    //             const auto& itr = CenterWay->GetVertices();
    //             for (auto it = itr.begin(); it != itr.end(); ++it) {
    //                 LinePoints.Add(*it);
    //             }
    //             CreateLineComponentByType(EPLATEAURoadLineType::YellowLine, LinePoints);
    //         }
    //     }
    //
    //     //Borders
    //     const auto& Borders =  Road->GetBorders();
    //     for (const auto& Border : Borders) {
    //
    //         if (Border != nullptr) {
    //
    //             TArray<FVector> LinePoints;
    //             const auto& itr = Border->GetVertices();
    //             for (auto it = itr.begin(); it != itr.end(); ++it) {
    //                 LinePoints.Add(*it);
    //             }
    //
    //             if(LinePoints.Num() <= 2)
    //                 CreateLineComponentByType(EPLATEAURoadLineType::StopLine, LinePoints);
    //             else
    //                 CreateLineComponentByType(EPLATEAURoadLineType::Crossing, LinePoints);
    //         }
    //     }
    // }
}

void APLATEAUReproducedRoad::CreateLineComponentByType(EPLATEAURoadLineType Type, const TArray<FVector>& LinePoints, FVector2D Offset) {
    const auto& Param = LineTypeMap[Type];
    const auto& Component = NewObject<ULineGeneratorComponent>(this, FName(TEXT("LineGeneratorComponent_") + StaticEnum<EPLATEAURoadLineType>()->GetDisplayValueAsText(Type).ToString() + TEXT("_") + FString::FromInt(NumComponents)));
    Component->RegisterComponent();
    this->AddInstanceComponent(Component);
    Component->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);  
    Component->SplinePointType = Param.SplinePointType;
    Component->CreateSplineFromVectorArray(LinePoints);
    Component->SplineMeshType = ESplineMeshType::LengthBased; //Segment単位に変更可能
    Component->FillEnd = Param.FillEnd;
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
