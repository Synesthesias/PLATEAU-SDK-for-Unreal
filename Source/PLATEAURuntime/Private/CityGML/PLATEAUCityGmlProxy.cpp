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
            if (CityModelCache.Contains(GmlInfo.GmlName)) {
                Completed.Broadcast(CityModelCache[GmlInfo.GmlName]);
                return;
            }

            citygml::ParserParams params;
            params.tesselate = false;

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
                Failed.Broadcast();
                return;
            }

            CityModelCache.Add(GmlInfo.GmlName, FPLATEAUCityModel(CityModelData));

            Completed.Broadcast(CityModelCache[GmlInfo.GmlName]);
        }

        }, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
}

UPLATEAUCityGmlProxy* UPLATEAUCityGmlProxy::LoadAsync(UObject* WorldContextObject, const FPLATEAUCityObjectInfo& GmlInfo) {
    const auto Node = NewObject<UPLATEAUCityGmlProxy>();
    Node->WorldContextObject = WorldContextObject;
    Node->GmlInfo = GmlInfo;
    return Node;
}
