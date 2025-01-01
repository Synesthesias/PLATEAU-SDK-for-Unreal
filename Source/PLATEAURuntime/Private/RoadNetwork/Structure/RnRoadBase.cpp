#include "RoadNetwork/Structure/RnRoadBase.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnModel.h"

RnRoadBase::RnRoadBase()
{
    SideWalks = MakeShared<TArray<RnRef_t<RnSideWalk>>>();
    TargetTrans = MakeShared<TArray<UPLATEAUCityObjectGroup*>>();
}

void RnRoadBase::AddSideWalk(const RnRef_t<RnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    if (SideWalks->Contains(SideWalk)) return;

    if (SideWalk->GetParentRoad()) {
        SideWalk->GetParentRoad()->RemoveSideWalk(SideWalk);
    }
    SideWalk->SetParent(RnRef_t<RnRoadBase>(this));
    SideWalks->Add(SideWalk);
}

void RnRoadBase::RemoveSideWalk(const RnRef_t<RnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalk->SetParent(nullptr);
    SideWalks->Remove(SideWalk);
}

void RnRoadBase::AddTargetTran(UPLATEAUCityObjectGroup* TargetTran) {
    if (!TargetTrans->Contains(TargetTran)) {
        TargetTrans->Add(TargetTran);
    }
}

void RnRoadBase::AddTargetTrans(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans) {
    for (const auto& Tran : InTargetTrans) {
        AddTargetTran(Tran);
    }
}

TArray<RnRef_t<RnWay>> RnRoadBase::GetAllWays() const {
    TArray<RnRef_t<RnWay>> Ways;
    for (const auto& SideWalk : *SideWalks) {
        for (const auto& Way : SideWalk->GetAllWays()) {
            Ways.Add(Way);
        }
    }
    return Ways;
}

void RnRoadBase::DisConnect(bool RemoveFromModel) {
    if (RemoveFromModel) {
        for (const auto& SideWalk : *SideWalks) {
            if (ParentModel) {
                ParentModel->RemoveSideWalk(SideWalk);
            }
        }
    }
}

void RnRoadBase::UnLinkEachOther(const RnRef_t<RnRoadBase>& Other) {
    if (this) UnLink(Other);
    if (Other) Other->UnLink(RnRef_t<RnRoadBase>(this));
}

FString RnRoadBase::GetTargetTransName() const {
    if (!this || !TargetTrans) return TEXT("null");

    TArray<FString> Names;
    for (const auto& Trans : *TargetTrans) {
        Names.Add(Trans ? Trans->GetName() : TEXT("null"));
    }
    return FString::Join(Names, TEXT(","));
}

TSet<RnRef_t<RnLineString>> RnRoadBase::GetAllLineStringsDistinct() const {
    TSet<RnRef_t<RnLineString>> LineStrings;
    if (!this) return LineStrings;

    for (const auto& Way : GetAllWays()) {
        if (Way && Way->LineString) {
            LineStrings.Add(Way->LineString);
        }
    }
    return LineStrings;
}
