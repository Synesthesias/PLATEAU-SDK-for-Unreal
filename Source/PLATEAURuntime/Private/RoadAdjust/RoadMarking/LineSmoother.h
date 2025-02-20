// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnWay.h"

namespace PLATEAU::RoadAdjust::RoadMarking {

    /**
     * @brief 線を滑らかにする際の戦略を定義する抽象クラスです。
     * どこの線を滑らかにし、どこを滑らかにしないかが道路生成の用途によって異なるため、サブクラスで設定します。
     */
    class PLATEAURUNTIME_API ISmoothingStrategy {
    public:
        virtual ~ISmoothingStrategy() = default;
        virtual bool ShouldSmoothSidewalkOutside() const = 0;
        virtual bool ShouldSmoothRoadSidewalkInside(const TWeakObjectPtr<UPLATEAUCityObjectGroup> Src) const = 0;
        virtual bool ShouldSmoothIntersectionSidewalkInside() const = 0;
        virtual bool ShouldSubdivide() const = 0;
    };

    /**
     * @brief 3D都市モデルの道路から道路ネットワークを生成する場合に使用する設定です。
     * 道路線を滑らかにするにあたって、外形線など3D都市モデルに直接由来する線は、元の形状を尊重したいというクライアント要望のために滑らかにしません。
     * 一方で、類推された線はは3D都市モデルに直接由来しないので滑らかにします。例えばLOD1の道路で類推された歩道の内側の線は滑らかにします。
     */
    class PLATEAURUNTIME_API FSmoothingStrategyRespectOriginal : public ISmoothingStrategy {
    public:
        virtual bool ShouldSmoothSidewalkOutside() const override { return false; }
        virtual bool ShouldSmoothRoadSidewalkInside(const TWeakObjectPtr<UPLATEAUCityObjectGroup> Src) const override;
        virtual bool ShouldSmoothIntersectionSidewalkInside() const override { return true; }
        virtual bool ShouldSubdivide() const override { return true; }
    };

    /**
     * @brief ユーザーが新しく道路を作成する場合に使用する設定です。
     * ユーザーが指定した形状が数少ない点からなる線であっても綺麗になるよう、すべて滑らかにします。
     */
    class PLATEAURUNTIME_API FSmoothingStrategySmoothAll : public ISmoothingStrategy {
    public:
        virtual bool ShouldSmoothSidewalkOutside() const override { return true; }
        virtual bool ShouldSmoothRoadSidewalkInside(const TWeakObjectPtr<UPLATEAUCityObjectGroup> Src) const override { return true; }
        virtual bool ShouldSmoothIntersectionSidewalkInside() const override { return true; }
        virtual bool ShouldSubdivide() const override { return false; }
    };

    /**
     * @brief 線を滑らかにするクラスです。
     */
    class PLATEAURUNTIME_API FLineSmoother {
    public:
        explicit FLineSmoother(bool bInDoSubdivide);

        TArray<FVector> Smooth(const TArray<FVector>& Line);
        void Smooth(TRnRef_T<URnWay> Way);

    private:
        static constexpr float SubDivideDistance = 300.0f;        // cm
        static constexpr float SmoothResolutionDistance = 50.0f;  // cm
        static constexpr float OptimizeAngleThreshold = 2.0f;     // 度数法

        bool bDoSubdivide;

        TArray<FVector> SubDivide(const TArray<FVector>& Line) const;
        TArray<FVector> SmoothBySpline(const TArray<FVector>& Line) const;
        TArray<FVector> Optimize(const TArray<FVector>& Line) const;
    };

    /**
     * @brief 道路ネットワーク中にある線を滑らかにするクラスです。
     */
    class PLATEAURUNTIME_API FRoadNetworkLineSmoother {
    public:
        void Smooth(URnModel* Target, const ISmoothingStrategy& SmoothingStrategy);
    };
}
