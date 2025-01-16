#include "RoadNetwork/Util/RnEx.h"

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Structure/RnLineString.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnModel.h"

void FRnEx::ReplaceLane(TArray<RnRef_t<RnLane>>& Self, RnRef_t<RnLane> Before, RnRef_t<RnLane> After) {
    Replace(Self, Before, After);
}


RnRef_t<RnLineString> FRnEx::CreateInnerLerpLineString(
    const TArray<FVector>& LeftVertices,
    const TArray<FVector>& RightVertices,
    RnRef_t<RnPoint> Start,
    RnRef_t<RnPoint> End,
    RnRef_t<RnWay> StartBorder,
    RnRef_t<RnWay> EndBorder,
    float T,
    float PointSkipDistance) {
    // 左右がどちらも直線もしくは点以下の場合 -> start/endを直接つなぐ
    if (LeftVertices.Num() <= 2 && RightVertices.Num() <= 2) {
        auto Points = MakeShared<TArray<RnRef_t<RnPoint>>>();
        Points->Add(Start);
        Points->Add(End);
        return RnNew<RnLineString>(Points);
    }

    auto Line = RnNew<RnLineString>();

    auto AddPoint = [&](RnRef_t<RnPoint> P) {
        if (!P) return;
        Line->AddPointOrSkip(P, PointSkipDistance);
        };

    AddPoint(Start);
    auto Segments = FGeoGraphEx::GetInnerLerpSegments(LeftVertices, RightVertices, RnModel::Plane, T);

    // 1つ目の点はボーダーと重複するのでスキップ
    for (int32 i = 1; i < Segments.Num(); ++i) {
        AddPoint(RnNew<RnPoint>(Segments[i]));
    }

    AddPoint(End);

    // 自己交差があれば削除する
    auto Plane = RnModel::Plane;
    FGeoGraph2D::RemoveSelfCrossing<RnRef_t<RnPoint>>(
        *Line->Points,
        [Plane](RnRef_t<RnPoint> T) { return FAxisPlaneEx::GetTangent(T->Vertex, Plane); },
        [](RnRef_t<RnPoint> P1, RnRef_t<RnPoint> P2, RnRef_t<RnPoint> P3, RnRef_t<RnPoint> P4,
            const FVector2D& Inter, float F1, float F2) {
                return RnNew<RnPoint>(FMath::Lerp(P1->Vertex, P2->Vertex, F1));
        });

    return Line;
}

RnRef_t<FLineCrossPointResult> FRnEx::GetLineIntersections(
    const FLineSegment3D& LineSegment,
    const TArray<RnRef_t<RnWay>>& Ways) {
    auto Result = RnNew<FLineCrossPointResult>();
    Result->LineSegment = LineSegment;

    // 全てのwayのLineStringを取得
    TSet<RnRef_t<RnLineString>> TargetLines;
    for (const auto& Way : Ways) {
        TargetLines.Add(Way->LineString);
    }

    for (const auto& Way : TargetLines) {
        auto Elem = RnNew<FLineCrossPointResult::FTargetLineInfo>();
        Elem->LineString = Way;

        auto Intersections = Way->GetIntersectionBy2D(LineSegment, RnModel::Plane);
        for (const auto& R : Intersections) {
            Elem->Intersections.Add(MakeTuple(R.Key, R.Value));
        }

        Result->TargetLines.Add(Elem);
    }

    return Result;
}

void FRnEx::AddChildInstanceComponent(AActor* Actor, USceneComponent* Parent, USceneComponent* Child,
    FAttachmentTransformRules TransformRule)
{
    if (!Parent || !Child || !Actor)
        return;
    Actor->AddInstanceComponent(Child);
    Child->SetupAttachment(Parent);
    Child->RegisterComponent();
    Child->AttachToComponent(Parent, TransformRule);
    Actor->RerunConstructionScripts();
}
