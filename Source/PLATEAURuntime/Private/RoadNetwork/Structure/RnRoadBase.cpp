#include "RoadNetwork/Structure/RnRoadBase.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnModel.h"

URnRoadBase::URnRoadBase()
{
}

void URnRoadBase::AddSideWalk(const TRnRef_T<URnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    if (SideWalks.Contains(SideWalk)) return;

    if (SideWalk->GetParentRoad()) {
        SideWalk->GetParentRoad()->RemoveSideWalk(SideWalk);
    }
    SideWalk->SetParent(TRnRef_T<URnRoadBase>(this));
    SideWalks.Add(SideWalk);
}

void URnRoadBase::RemoveSideWalk(const TRnRef_T<URnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalk->SetParent(nullptr);
    SideWalks.Remove(SideWalk);
}

void URnRoadBase::AddTargetTran(UPLATEAUCityObjectGroup* TargetTran) {
    if (!TargetTrans.Contains(TargetTran)) {
        TargetTrans.Add(TargetTran);
    }
}

void URnRoadBase::AddTargetTrans(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans) {
    for (const auto& Tran : InTargetTrans) {
        AddTargetTran(Tran);
    }
}

TArray<TRnRef_T<URnWay>> URnRoadBase::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    for (const auto& SideWalk : SideWalks) {
        for (const auto& Way : SideWalk->GetAllWays()) {
            Ways.Add(Way);
        }
    }
    return Ways;
}

void URnRoadBase::DisConnect(bool RemoveFromModel) {
    if (RemoveFromModel) {
        for (const auto& SideWalk : SideWalks) {
            if (ParentModel) {
                ParentModel->RemoveSideWalk(SideWalk);
            }
        }
    }
}

void URnRoadBase::UnLinkEachOther(const TRnRef_T<URnRoadBase>& Other) {
    if (this) UnLink(Other);
    if (Other) Other->UnLink(TRnRef_T<URnRoadBase>(this));
}

FString URnRoadBase::GetTargetTransName() const {
    if (!this) 
        return TEXT("null");

    TArray<FString> Names;
    for (const auto& Trans : TargetTrans) {
        Names.Add(Trans ? Trans->GetName() : TEXT("null"));
    }
    return FString::Join(Names, TEXT(","));
}

TSet<TRnRef_T<URnLineString>> URnRoadBase::GetAllLineStringsDistinct() const {
    TSet<TRnRef_T<URnLineString>> LineStrings;
    if (!this) return LineStrings;

    for (const auto& Way : GetAllWays()) {
        if (Way && Way->LineString) {
            LineStrings.Add(Way->LineString);
        }
    }
    return LineStrings;
}
