#include "RoadNetwork/Util/PLATEAURnEx.h"

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Structure/RnLineString.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnModel.h"

int32 FPLATEAURnEx::Vector3Comparer::operator()(const FVector& A, const FVector& B) const
{
    auto X = Compare(A.X, B.X);
    if (X != 0)
        return X;
    auto Y = Compare(A.Y, B.Y);
    if (Y != 0)
        return Y;
    return X = Compare(A.Z, B.Z);
}

void FPLATEAURnEx::ReplaceLane(TArray<TRnRef_T<URnLane>>& Self, TRnRef_T<URnLane> Before, TRnRef_T<URnLane> After) {
    Replace(Self, Before, After);
}


TRnRef_T<URnLineString> FPLATEAURnEx::CreateInnerLerpLineString(
    const TArray<FVector>& LeftVertices,
    const TArray<FVector>& RightVertices,
    TRnRef_T<URnPoint> Start,
    TRnRef_T<URnPoint> End,
    TRnRef_T<URnWay> StartBorder,
    TRnRef_T<URnWay> EndBorder,
    float T,
    float PointSkipDistance) {
    // 左右がどちらも直線もしくは点以下の場合 -> start/endを直接つなぐ
    if (LeftVertices.Num() <= 2 && RightVertices.Num() <= 2) {
        auto Points = TArray<TRnRef_T<URnPoint>>();
        Points.Add(Start);
        Points.Add(End);
        return RnNew<URnLineString>(Points);
    }

    auto Line = RnNew<URnLineString>();

    auto AddPoint = [&](TRnRef_T<URnPoint> P) {
        if (!P) return;
        Line->AddPointOrSkip(P, PointSkipDistance);
        };

    AddPoint(Start);
    auto Segments = FGeoGraphEx::GetInnerLerpSegments(LeftVertices, RightVertices, FPLATEAURnDef::Plane, T);

    // 1つ目の点はボーダーと重複するのでスキップ
    for (int32 i = 1; i < Segments.Num(); ++i) {
        AddPoint(RnNew<URnPoint>(Segments[i]));
    }

    AddPoint(End);

    // 自己交差があれば削除する
    auto Plane = URnModel::Plane;
    FGeoGraph2D::RemoveSelfCrossing<TRnRef_T<URnPoint>>(
        Line->GetPoints(),
        [Plane](TRnRef_T<URnPoint> T) { return FAxisPlaneEx::GetTangent(T->Vertex, Plane); },
        [](TRnRef_T<URnPoint> P1, TRnRef_T<URnPoint> P2, TRnRef_T<URnPoint> P3, TRnRef_T<URnPoint> P4,
            const FVector2D& Inter, float F1, float F2) {
                return RnNew<URnPoint>(FMath::Lerp(P1->Vertex, P2->Vertex, F1));
        });

    return Line;
}

FPLATEAURnEx::FLineCrossPointResult FPLATEAURnEx::GetLineIntersections(
    const FLineSegment3D& LineSegment,
    const TArray<TRnRef_T<URnWay>>& Ways)
{
    FLineCrossPointResult Result;
    Result.LineSegment = LineSegment;

    // 全てのwayのLineStringを取得
    TSet<TRnRef_T<URnLineString>> TargetLines;
    for (const auto& Way : Ways) {
        TargetLines.Add(Way->LineString);
    }

    for (const auto& Way : TargetLines) {
        FLineCrossPointResult::FTargetLineInfo Elem;
        Elem.LineString = Way;

        auto Intersections = Way->GetIntersectionBy2D(LineSegment, URnModel::Plane);
        for (const auto& R : Intersections) {
            Elem.Intersections.Add(MakeTuple(R.Key, R.Value));
        }

        Result.TargetLines.Add(Elem);
    }

    return Result;
}

void FPLATEAURnEx::AddChildInstanceComponent(AActor* Actor, USceneComponent* Parent, USceneComponent* Child,
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
