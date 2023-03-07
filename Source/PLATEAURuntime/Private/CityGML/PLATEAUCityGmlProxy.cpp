#include "CityGML/PLATEAUCityGmlProxy.h"
#include "CityGML/PLATEAUCityModel.h"

#include <citygml/citygml.h>

#include "Async/Async.h"

TMap<FString, FPLATEAUCityModel> UPLATEAUCityGmlProxy::CityModelCache;

void UPLATEAUCityGmlProxy::Activate() {
    static FCriticalSection CriticalSection;

    FFunctionGraphTask::CreateAndDispatchWhenReady([this]() {
        {
            FScopeLock Lock(&CriticalSection);

            const auto CityModelData = Load(GmlInfo);

            if (CityModelData == nullptr)
                Failed.Broadcast();

            Completed.Broadcast(FPLATEAUCityModel(CityModelData));
        }

        }, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
}

UPLATEAUCityGmlProxy* UPLATEAUCityGmlProxy::LoadAsync(UObject* WorldContextObject, const FPLATEAUCityObjectInfo& GmlInfo) {
    const auto Node = NewObject<UPLATEAUCityGmlProxy>();
    Node->WorldContextObject = WorldContextObject;
    Node->GmlInfo = GmlInfo;
    return Node;
}

std::shared_ptr<const citygml::CityModel> UPLATEAUCityGmlProxy::Load(const FPLATEAUCityObjectInfo& GmlInfo) {
    if (CityModelCache.Find(GmlInfo.GmlName))
        return CityModelCache[GmlInfo.GmlName].GetData();

    citygml::ParserParams params;
    params.tesselate = false;
    params.ignoreGeometries = true;

    FString SubFolderName;
    int Index = 0;
    if (GmlInfo.GmlName.FindChar('_', Index))
        SubFolderName = GmlInfo.GmlName.RightChop(Index + 1);
    if (SubFolderName.FindChar('_', Index))
        SubFolderName = SubFolderName.LeftChop(SubFolderName.Len() - Index);

    const auto FullGmlPath =
        FPaths::ProjectContentDir() +
        "PLATEAU/Datasets/" +
        GmlInfo.DatasetName +
        "/udx/" +
        SubFolderName + "/" +
        GmlInfo.GmlName;
    std::shared_ptr<const citygml::CityModel> CityModelData;
    try {
        CityModelData = citygml::load(TCHAR_TO_UTF8(*FullGmlPath), params);
    }
    catch (...) {
    }

    if (CityModelData == nullptr) {
        return nullptr;
    }

    CityModelCache.Add(GmlInfo.GmlName, FPLATEAUCityModel(CityModelData));
    return CityModelCache[GmlInfo.GmlName].GetData();
}
