// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "UPLATEAUStopLineComposer.generated.h"

UCLASS()
class PLATEAURUNTIME_API UPLATEAUStopLineComposer : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()
public:
    ~UPLATEAUStopLineComposer() = default;

    /**
     * @brief 道路ネットワークから停止線を生成します。
     * @param Target 道路ネットワークのターゲット
     * @return 生成された停止線のリスト
     */
    FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;

private:
    static constexpr float CONST_HeightOffset = 7.0f; // 7cm。経験的にこのくらいの高さなら道路にめりこまないという値

    /**
     * @brief 停止線を追加します。
     * @param WayList 停止線を追加するリスト
     * @param Border 境界線
     */
    void AddStopLine(FPLATEAUMarkedWayList& WayList, const FPLATEAUMWLine& Border);
};
