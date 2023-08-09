// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditor/Public/Widgets/PLATEAUSDKEditorUtilityWidget.h"

#include "Selection.h"
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
    if (bGettingNativeDatasetMetadata) return;

    ClientPtr = std::make_shared<plateau::network::Client>(TCHAR_TO_UTF8(*InServerURL), TCHAR_TO_UTF8(*InToken));

    Async(EAsyncExecution::Thread, [bGettingNativeDatasetMetadata = bGettingNativeDatasetMetadata, ServerDatasetMetadataMapArray = ServerDatasetMetadataMapArray, ClientPtr = ClientPtr, GetDatasetMetaDataAsyncSuccessDelegate = GetDatasetMetaDataAsyncSuccessDelegate]() mutable {
        bGettingNativeDatasetMetadata = true;
        ServerDatasetMetadataMapArray.Reset();
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

        FFunctionGraphTask::CreateAndDispatchWhenReady([GetDatasetMetaDataAsyncSuccessDelegate, ServerDatasetMetadataMapArray] {
            GetDatasetMetaDataAsyncSuccessDelegate.Broadcast(ServerDatasetMetadataMapArray);
        }, TStatId(), nullptr, ENamedThreads::GameThread);

        bGettingNativeDatasetMetadata = false;
    });
}

/**
 * @brief 選択イベント通知の実行制御
 * @param TopMenuPanel イベント通知するパネル
 */
void UPLATEAUSDKEditorUtilityWidget::SetEnableSelectionChangedEvent(const ETopMenuPanel TopMenuPanel) {
    switch (TopMenuPanel) {
    case ETopMenuPanel::ImportPanel:
        if (SelectionChangedEventHandle.IsValid()) {
            USelection::SelectionChangedEvent.Remove(SelectionChangedEventHandle);
            SelectionChangedEventHandle.Reset();
        }
        break;
    case ETopMenuPanel::ModelAdjustmentPanel:
    case ETopMenuPanel::ExportPanel:
    case ETopMenuPanel::AttrInfoPanel:
        if (!SelectionChangedEventHandle.IsValid()) {
            SelectionChangedEventHandle = USelection::SelectionChangedEvent.AddUObject(this, &UPLATEAUSDKEditorUtilityWidget::OnSelectionChanged);
        }
        break;
    default:
        if (SelectionChangedEventHandle.IsValid()) {
            USelection::SelectionChangedEvent.Remove(SelectionChangedEventHandle);
            SelectionChangedEventHandle.Reset();
        }
    }
}

/**
 * @brief 選択が変更された時に通知
 * @param InSelection 選択されたオブジェクト
 */
void UPLATEAUSDKEditorUtilityWidget::OnSelectionChanged(UObject* InSelection) {
    if (GEditor) {
        if (GEditor->GetSelectedActors()->Num() <= 0 && GEditor->GetSelectedComponents()->Num() <= 0) {
            OnSelectionChangedDelegate.Broadcast(nullptr, nullptr, true);
            SelectionActor = nullptr;
            SelectionComponent = nullptr;
            return;
        }

        if (const auto& BottomActor = GEditor->GetSelectedActors()->GetBottom<AActor>(); BottomActor && SelectionActor != BottomActor) {
            SelectionActor = BottomActor;
            OnSelectionChangedDelegate.Broadcast(BottomActor, SelectionComponent, true);
        }

        if (const auto& BottomComponent = GEditor->GetSelectedComponents()->GetBottom<USceneComponent>(); BottomComponent && SelectionComponent !=
            BottomComponent) {
            SelectionComponent = BottomComponent;
            OnSelectionChangedDelegate.Broadcast(SelectionActor, BottomComponent, false);
        }
    }
}
