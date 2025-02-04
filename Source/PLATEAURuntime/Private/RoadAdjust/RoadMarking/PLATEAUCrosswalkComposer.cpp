// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/PLATEAUCrosswalkComposer.h"
#include "RoadAdjust/RoadMarking/LineGeneratorComponent.h"

UPLATEAUCrosswalkComposer::UPLATEAUCrosswalkComposer() {
}

UPLATEAUCrosswalkComposer::~UPLATEAUCrosswalkComposer() {
}

FPLATEAUMarkedWayList UPLATEAUCrosswalkComposer::Compose(
    const IPLATEAURrTarget& Target,
    const EPLATEAUCrosswalkFrequency& CrosswalkFrequency) {
    
    FPLATEAUMarkedWayList Result;
    const auto PlacementRule = FPLATEAUCrosswalkFrequencyExtensions::ToPlacementRule(CrosswalkFrequency);

    for (const auto& Road : Target.GetRoads()) {
        if (!PlacementRule->ShouldPlace(Road)) continue;

        auto Next = Road->GetNext();
        if(Next != nullptr)
        {
            if (const auto NextIntersection = Next->CastToIntersection()) {
                const auto NextBorder = FPLATEAUMWLine(Road->GetMergedBorder(EPLATEAURnLaneBorderType::Next, TOptional<EPLATEAURnDir>()));
                const auto Crosswalk = GenerateCrosswalk(NextBorder, NextIntersection, Road, EPLATEAUReproducedRoadDirection::Next);
                Result.AddRange(Crosswalk);
            }
        }
        
        auto Prev = Road->GetPrev();
        if(Prev != nullptr)
        {
            if (const auto PrevIntersection = Prev->CastToIntersection()) {
                const auto PrevBorder = FPLATEAUMWLine(Road->GetMergedBorder(EPLATEAURnLaneBorderType::Prev, TOptional<EPLATEAURnDir>()));
                const auto Crosswalk = GenerateCrosswalk(PrevBorder, PrevIntersection, Road, EPLATEAUReproducedRoadDirection::Prev);
                Result.AddRange(Crosswalk);
            }
        }
        
    }

    return Result;
}

FPLATEAUMarkedWayList UPLATEAUCrosswalkComposer::GenerateCrosswalk(
    const FPLATEAUMWLine& Border,
    const TRnRef_T<URnIntersection>& Intersection,
    const TRnRef_T<URnRoadBase>& SrcRoad,
    EPLATEAUReproducedRoadDirection Direction) {

    auto Result = FPLATEAUMarkedWayList(); 

    // 横断歩道を結ぶべき場所として、歩道と歩道を結んだ線を求めます。
    const auto LinePositions = ShiftStopLine(Border, Intersection, PositionOffset);
    if (LinePositions.Num() == 0) return Result;

    auto LineString = NewObject<URnLineString>();
    for(int i=0; i<LinePositions.Num(); i++)
    {
        auto Pos = LinePositions[i];
        auto Point = NewObject<URnPoint>();
        Point->Init(Pos);
        LineString->AddPointOrSkip(TRnRef_T<URnPoint>(Point));
    }
    
    const auto LineWay = NewObject<URnWay>();
    LineWay->Init(TRnRef_T<URnLineString>(LineString));

    // 横断歩道を車道の中心に配置するための計算です。
    const float LineLen = FVector::Distance(LinePositions[0], LinePositions.Last());
    const float LineLenMinusOffset = LineLen - LineOffset * 2.0f;
    int32 DashCount = FMath::FloorToInt(LineLenMinusOffset / CrosslineDashInterval);
    if (DashCount % 2 == 0) DashCount--; // 破線が空白部で終わる場合、末尾の空白を除く
    if (DashCount <= 0) return Result;

    const float CrosswalkLen = CrosslineDashInterval * DashCount + 1.0f;
    const float CrosswalkOffset = (LineLen - CrosswalkLen) / 2.0f;

    TArray<FVector> CrosswalkPositions;
    CrosswalkPositions.Add(LineWay->PositionAtDistance(CrosswalkOffset, false));
    CrosswalkPositions.Add(LineWay->PositionAtDistance(CrosswalkOffset, true));

    // 高さオフセットを適用
    for (auto& Position : CrosswalkPositions) {
        Position.Z += HeightOffset;
    }

    // 横断歩道を1本の破線として生成します。
    auto CrosswalkLine = FPLATEAUMWLine(CrosswalkPositions);
    auto MarkedWay = FPLATEAUMarkedWay(CrosswalkLine, EPLATEAUMarkedWayType::ShoulderLine, false);
    // auto Generator = FDashedLineMeshGenerator(ERoadMarkingMaterial::White, true, CrosslineWidth, CrosslineDashInterval);
    // const auto Crosswalk = Generator.GenerateMeshInstance(CrosswalkPositions);
    Result.Add(MarkedWay);

    return Result;
}

TArray<FVector> UPLATEAUCrosswalkComposer::ShiftStopLine(
    const FPLATEAUMWLine& Border,
    const TRnRef_T<URnIntersection>& Intersection,
    float PositionOffsetArg) {

    if (Border.Num() <= 1) return TArray<FVector>();

    constexpr float Threshold = 10.0f; // 0.1m * 100 (Unreal units)
    const auto Stop1 = Border[0];
    const auto Stop2 = Border[Border.Num() - 1];

    // 交差点で道路と接していない外形のうち、該当道路と接する線がどこかを探します
    FVector Shift1 = Stop1;
    FVector Shift2 = Stop2;
    bool bShift1Found = false;
    bool bShift2Found = false;

    for (const auto& Edge : Intersection->GetEdges()) {
        if (Edge->GetRoad() == nullptr) continue;

        const auto& EWay = Edge->GetBorder();
        const auto& E = EWay->GetVertices().ToArray();
        const auto& E1 = E[0];
        const auto& E2 = E[E.Num() - 1];

        if (FVector::Distance(E1, Stop1) < Threshold) {
            Shift1 = EWay->PositionAtDistance(PositionOffsetArg, false);
            bShift1Found = true;
        }
        else if (FVector::Distance(E1, Stop2) < Threshold) {
            Shift2 = EWay->PositionAtDistance(PositionOffsetArg, false);
            bShift2Found = true;
        }
        else if (FVector::Distance(E2, Stop1) < Threshold) {
            Shift1 = EWay->PositionAtDistance(PositionOffsetArg, true);
            bShift1Found = true;
        }
        else if (FVector::Distance(E2, Stop2) < Threshold) {
            Shift2 = EWay->PositionAtDistance(PositionOffsetArg, true);
            bShift2Found = true;
        }
    }

    if (!bShift1Found || !bShift2Found) {
        return TArray<FVector>();
    }

    return TArray<FVector>{Shift1, Shift2};
}
