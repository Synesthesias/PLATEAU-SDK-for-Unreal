// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/PLATEAUMarkedWayListComposerMain.h"

#include "RoadAdjust/RoadMarking/PLATEAUMCCenterLine.h"
#include "RoadAdjust/RoadMarking/PLATEAUMCIntersection.h"
#include "RoadAdjust/RoadMarking/PLATEAUMCLaneLine.h"
#include "RoadAdjust/RoadMarking/PLATEAUMCShoulderLine.h"
#include "RoadAdjust/RoadMarking/UPLATEAUStopLineComposer.h"


FPLATEAUMarkedWayList UPLATEAUMarkedWayListComposerMain::ComposeFrom(const IPLATEAURrTarget* Target)
{
    // 結果を格納するリスト
    FPLATEAUMarkedWayList Result;
    

    // 生成したい線の種類を列挙
    auto Composers = TArray<TScriptInterface<IPLATEAUMarkedWayListComposer>>();
    Composers.Add(NewObject<UPLATEAUMCLaneLine>()); // 車線の間の線のうち、センターラインでないもの
    Composers.Add(NewObject<UPLATEAUMCShoulderLine>());  // 路側帯線、すなわち歩道と車道の間の線
    Composers.Add(NewObject<UPLATEAUMCCenterLine>());    // センターライン
    Composers.Add(NewObject<UPLATEAUMCIntersection>());  // 交差点の線
    Composers.Add(NewObject<UPLATEAUStopLineComposer>()); // 停止線
    

    // 各コンポーザーの結果を収集し、高さオフセットを適用
    for (const auto Composer : Composers)
    {
        if (Composer != nullptr)
        {
            FPLATEAUMarkedWayList MarkedWayList = Composer->ComposeFrom(Target);
            MarkedWayList.Translate(FVector::UpVector * UPLATEAUMarkedWayListComposerMain::HeightOffset);
            Result.AddRange(MarkedWayList);
        }
    }

    return Result;
}
