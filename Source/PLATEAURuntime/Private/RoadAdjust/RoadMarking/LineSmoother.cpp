// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/LineSmoother.h"

#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "Engine/Engine.h"

namespace PLATEAU::RoadAdjust::RoadMarking {

    bool FSmoothingStrategyRespectOriginal::ShouldSmoothRoadSidewalkInside(const TWeakObjectPtr<UPLATEAUCityObjectGroup> Src) const  {
        if (Src == nullptr) return true;
        // if (Src->GetLod() < 2) return true;
        // return false;
        return true;
    }

    FLineSmoother::FLineSmoother(bool bInDoSubdivide) : bDoSubdivide(bInDoSubdivide) {}

    TArray<FVector> FLineSmoother::Smooth(const TArray<FVector>& Line) {
        if (Line.Num() == 0) return Line;

        // スプライン補間だと点が離れている場合に元の線からのズレが大きくなりがちなので、
        // 点を細かくしてからスプライン補間を行います。
        const TArray<FVector> LineA = bDoSubdivide ? SubDivide(Line) : Line;
        const TArray<FVector> LineB = SmoothBySpline(LineA);
        const TArray<FVector> LineC = Optimize(LineB);
        return LineC;
    }

    void FLineSmoother::Smooth(TRnRef_T<URnWay> Way)
    {
        if (Way == nullptr) return;
        if (!Way->IsValid()) return;
        
        TArray<FVector> Points;
        for (const auto& Point : Way->GetPoints()) {
            Points.Add(Point->GetVertex());
        }
        
        const TArray<FVector> Smoothed = Smooth(Points);
        TArray<TRnRef_T<URnPoint>> NewPoints;
        for (const auto& Point : Smoothed) {
            auto P = NewObject<URnPoint>();
            P->Init(Point);
            NewPoints.Add(P);
        }
        Way->SetPoints(NewPoints);
    }

    TArray<FVector> FLineSmoother::SubDivide(const TArray<FVector>& Line) const {
        if (Line.Num() <= 1) return Line;

        TArray<FVector> NextPoints;
        for (int32 i = 0; i < Line.Num() - 1; i++) {
            const FVector& P1 = Line[i];
            const FVector& P2 = Line[i + 1];
            const FVector Dir = P2 - P1;
            const float Len = Dir.Size();

            if (Len <= SubDivideDistance) {
                NextPoints.Add(P1);
                continue;
            }

            const int32 Num = FMath::FloorToInt(Len / SubDivideDistance);
            for (int32 j = 0; j < Num; j++) {
                const float T = static_cast<float>(j) / static_cast<float>(Num);
                const FVector P = P1 + Dir * T;
                NextPoints.Add(P);
            }
        }

        NextPoints.Add(Line.Last());
        return NextPoints;
    }

    TArray<FVector> FLineSmoother::SmoothBySpline(const TArray<FVector>& Line) const {
        if (Line.Num() <= 1) return Line;

        float SumDistance = 0.0f;
        for (int32 i = 0; i < Line.Num() - 1; i++) {
            SumDistance += FVector::Distance(Line[i], Line[i + 1]);
        }

        if (SumDistance <= 0.0f) return Line;

        // 一時的なアクターを作成してSplineComponentを使用
        UWorld* World = nullptr;
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            World = GEngine->GetWorldContexts()[0].World();
        }
        if(World == nullptr) return Line;

        AActor* TempActor = World->SpawnActor<AActor>();
        USplineComponent* SplineComp = NewObject<USplineComponent>(TempActor);
        SplineComp->RegisterComponent();
        SplineComp->ClearSplinePoints();

        // スプラインポイントを追加
        for (int32 i = 0; i < Line.Num(); i++) {
            SplineComp->AddSplinePoint(Line[i], ESplineCoordinateSpace::World);
        }

        // スプラインの補間方式を設定
        for (int32 i = 0; i < SplineComp->GetNumberOfSplinePoints(); i++) {
            SplineComp->SetSplinePointType(i, ESplinePointType::Curve);
        }
        
        // 端点のタンジェントを隣の点を向くように設定
        if (Line.Num() >= 2) {
            // 最初のポイントのタンジェント設定
            FVector FirstTangent = (Line[1] - Line[0]).GetSafeNormal() * 10.f; // 長さは経験則から
            SplineComp->SetTangentAtSplinePoint(0, FirstTangent, ESplineCoordinateSpace::World);
            
            // 最後のポイントのタンジェント設定
            int32 LastIndex = Line.Num() - 1;
            int32 PrevIndex = LastIndex - 1;
            FVector LastTangent = (Line[LastIndex] - Line[PrevIndex]).GetSafeNormal() * 10.f; // 長さは経験則から
            SplineComp->SetTangentAtSplinePoint(LastIndex, LastTangent, ESplineCoordinateSpace::World);
        }

        // 中間点のタンジェントも計算して設定
        for (int32 i = 1; i < Line.Num() - 1; i++) {
            // 前後の点からタンジェントを計算
            FVector PrevTangent = (Line[i] - Line[i-1]).GetSafeNormal();
            FVector NextTangent = (Line[i+1] - Line[i]).GetSafeNormal();
            // 前後のタンジェントの平均を取る
            FVector AvgTangent = (PrevTangent + NextTangent).GetSafeNormal();

            // タンジェントの長さを調整
            float TangentLength = FMath::Min(
                FVector::Distance(Line[i], Line[i-1]), 
                FVector::Distance(Line[i], Line[i+1])
            ) * 0.4f; // この係数は0.3～0.7くらいの範囲で調整の余地がある。小さいほど急カーブになる。経験則でこのくらいのほうが綺麗に見える。
    
            SplineComp->SetTangentAtSplinePoint(i, AvgTangent * TangentLength, ESplineCoordinateSpace::World);
        }
        
        SplineComp->UpdateSpline();

        // 補間された点を生成
        TArray<FVector> NextPoints;

        for (float Dist = 0; Dist < SplineComp->GetSplineLength(); Dist += SmoothResolutionDistance)
        {
            const FVector P = SplineComp->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
            NextPoints.Add(P);
        }
        NextPoints.Add(SplineComp->GetLocationAtSplinePoint(SplineComp->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World));

        // 一時的なアクターを削除
        TempActor->Destroy();

        return NextPoints;
    }

    TArray<FVector> FLineSmoother::Optimize(const TArray<FVector>& Line) const {
        if (Line.Num() <= 2) return Line;

        TArray<bool> ShouldRemove;
        ShouldRemove.Init(false, Line.Num());
        ShouldRemove[0] = false;
        ShouldRemove.Last() = false;

        float AngleDiffSum = 0.0f;
        for (int32 i = 0; i < Line.Num() - 2; i++) {
            const FVector& V1 = Line[i];
            const FVector& V2 = Line[i + 1];
            const FVector& V3 = Line[i + 2];

            const FVector Dir1 = (V2 - V1).GetSafeNormal();
            const FVector Dir2 = (V3 - V2).GetSafeNormal();
            float Dot = FVector::DotProduct(Dir1.GetSafeNormal(), Dir2.GetSafeNormal());
            Dot = FMath::Clamp(Dot, -1.0f, 1.0f);
            AngleDiffSum += FMath::RadiansToDegrees(FMath::Acos(Dot));

            if (AngleDiffSum >= OptimizeAngleThreshold) {
                ShouldRemove[i + 1] = false;
                AngleDiffSum = 0.0f;
                continue;
            }

            ShouldRemove[i + 1] = true;
        }

        TArray<FVector> NextLine;
        for (int32 i = 0; i < Line.Num(); i++) {
            if (!ShouldRemove[i]) {
                NextLine.Add(Line[i]);
            }
        }
        return NextLine;
    }

    void FRoadNetworkLineSmoother::Smooth(URnModel* Target, const ISmoothingStrategy& SmoothingStrategy) {
        if (Target == nullptr) return;

        auto Smoother = MakeShared<FLineSmoother>(SmoothingStrategy.ShouldSubdivide());

        for (const auto& Road : Target->GetRoads()) {
            const auto RoadSrc = Road->GetTargetTrans().Num() > 0 ? Road->GetTargetTrans()[0] : nullptr;

            for (const auto& SideWalk : Road->GetSideWalks()) {
                const auto Inside = SideWalk->GetInsideWay();
                if (Inside != nullptr && Inside->IsValid() && SmoothingStrategy.ShouldSmoothRoadSidewalkInside(RoadSrc)) {
                    Smoother->Smooth(Inside);
                }

                const auto Outside = SideWalk->GetOutsideWay();
                if (Outside != nullptr && Outside->IsValid() && SmoothingStrategy.ShouldSmoothSidewalkOutside()) {
                    Smoother->Smooth(Outside);
                }
            }

            for (const auto& Lane : Road->GetAllLanesWithMedian()) {
                for (const auto& Way : Lane->GetAllWays()) {
                    Smoother->Smooth(Way);
                }
            }
        }

        for (const auto& Intersection : Target->GetIntersections()) {
            for (const auto& SideWalk : Intersection->GetSideWalks()) {
                const auto Inside = SideWalk->GetInsideWay();
                if (Inside != nullptr && Inside->IsValid() && SmoothingStrategy.ShouldSmoothIntersectionSidewalkInside()) {
                    Smoother->Smooth(Inside);
                }

                const auto Outside = SideWalk->GetOutsideWay();
                if (Outside != nullptr && Outside->IsValid() && SmoothingStrategy.ShouldSmoothSidewalkOutside()) {
                    Smoother->Smooth(Outside);
                }
            }

            for (const auto& Edge : Intersection->GetEdges()) {
                if (Edge != nullptr && !Edge->IsBorder()) {
                    Smoother->Smooth(Edge->GetBorder());
                }
            }
        }
    }
}
