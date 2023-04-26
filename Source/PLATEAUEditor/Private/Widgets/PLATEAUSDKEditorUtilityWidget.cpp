// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditor/Public/Widgets/PLATEAUSDKEditorUtilityWidget.h"
#include "Async/Async.h"


/**
 * @brief 範囲選択完了通知
 * @param ReferencePoint リファレンス位置 
 * @param PackageMask パッケージマスク
 */
void UPLATEAUSDKEditorUtilityWidget::AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const {
    AreaSelectSuccessDelegate.Broadcast(ReferencePoint, PackageMask);
}

/**
 * @brief サーバから非同期でデータセット情報を取得
 * @param InServerURL サーバURL
 * @param InToken トークン
 */
void UPLATEAUSDKEditorUtilityWidget::GetDatasetMetadataAsync(const FString& InServerURL, const FString& InToken) {
    if (bGettingNativeDatasetMetadata)
        return;

    bGettingNativeDatasetMetadata = true;
    ServerDatasetMetadataMapArray.Reset();
    
    Async(EAsyncExecution::Thread, [this, InServerURL, InToken] {
        ClientPtr = std::make_shared<plateau::network::Client>(TCHAR_TO_UTF8(*InServerURL), TCHAR_TO_UTF8(*InToken));
        std::vector<plateau::network::DatasetMetadataGroup> NativeDatasetMetadataGroups;
        ClientPtr->getMetadata(NativeDatasetMetadataGroups);
        
        for (const auto& DatasetGroup : NativeDatasetMetadataGroups) {
            FServerDatasetMetadataMap ServerDatasetMetadataMap;
            ServerDatasetMetadataMap.GroupTitle = UTF8_TO_TCHAR(DatasetGroup.title.c_str());
            for (const auto& Dataset : DatasetGroup.datasets) {
                FServerDatasetMetadata ServerDatasetMetadata;
                ServerDatasetMetadata.Title = UTF8_TO_TCHAR(Dataset.title.c_str());
                ServerDatasetMetadata.Description = UTF8_TO_TCHAR(Dataset.description.c_str());
                ServerDatasetMetadata.ID = UTF8_TO_TCHAR(Dataset.id.c_str());
                ServerDatasetMetadataMap.ServerDatasetMetadataArray.Add(ServerDatasetMetadata);
            }
            ServerDatasetMetadataMapArray.Add(ServerDatasetMetadataMap);
        }

        FFunctionGraphTask::CreateAndDispatchWhenReady([&] {
            GetDatasetMetaDataAsyncSuccessDelegate.Broadcast(ServerDatasetMetadataMapArray);
        }, TStatId(), nullptr, ENamedThreads::GameThread);
        
        bGettingNativeDatasetMetadata = false;
    });
}