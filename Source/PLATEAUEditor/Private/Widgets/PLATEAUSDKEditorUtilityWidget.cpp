// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditor/Public/Widgets/PLATEAUSDKEditorUtilityWidget.h"

#include "MeshDescription.h"
#include "PLATEAUCityObjectGroup.h"
#include "Selection.h"
#include "Async/Async.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/LineBatchComponent.h"


namespace {
    // 描画ボックスでオブジェクトを囲むためのオフセット値
    constexpr double DrawBoxXYZOffset = 100;

    // ラインパラメータ
    constexpr bool GBPersistentLines = true;
    constexpr float Thickness = 20;
    constexpr float LineLifeTime = -1;
    constexpr uint8 DepthPriority = SDPG_World;
    
    /**
     * @brief 入力されたメッシュのポリゴンから全ポリゴンを囲むエッジとなる頂点を求める    
     * @return 全ポリゴンを囲むエッジとなる頂点情報を持つ構造体の配列
     */
    TArray<FVectorMap> GetVectorMapPerimeterArray(const FMeshDescription* MeshDescription) {
        TArray<FVectorMap> VectorMapArray;
        const auto PolygonCount = MeshDescription->Polygons().Num();
        for (int32 i = 0; i < PolygonCount; i++) {
            const auto& EdgeIdArray = MeshDescription->GetPolygonPerimeterEdges(i);
            for (const auto& EdgeId : EdgeIdArray) {
                // エッジの頂点は必ず２つある
                for (int32 j = 0; j < 2; j++) {
                    const auto& VertexId = MeshDescription->GetEdgeVertex(EdgeId, j);
                    const auto& VertexPos = FVector(MeshDescription->GetVertexPosition(VertexId));
                    bool ExistPos = false;
                    for (const auto& VectorMap : VectorMapArray) {
                        if (VectorMap.VertexID == VertexId || (VectorMap.VertexPos - VertexPos).Length() < DBL_EPSILON) {
                            ExistPos = true;
                            break;
                        }
                    }
                    // 同じ頂点ID、同じ座標は追加しない
                    if (!ExistPos) {
                        VectorMapArray.Emplace(FVectorMap(VertexId, VertexPos));
                    }
                }
            }
            // 頂点IDでソート
            VectorMapArray.Sort([](const FVectorMap& A, const FVectorMap& B) { return A.VertexID < B.VertexID; });
        }

        return VectorMapArray;
    }

    /**
     * @brief 入力されたメッシュ頂点のZ軸を等しくした頂点を求める    
     * @return Z軸が等しい頂点情報を持つ構造体の配列
     */
    TArray<FVectorMap> GetVectorMapArray(const FMeshDescription* MeshDescription) {
        TArray<FVectorMap> VectorMapArray;
        const auto PolygonCount = MeshDescription->Polygons().Num();
        for (int32 i = 0; i < PolygonCount; i++) {
            const auto& EdgeIdArray = MeshDescription->GetPolygonPerimeterEdges(i);
            for (const auto& EdgeId : EdgeIdArray) {
                // エッジの頂点は必ず２つある
                for (int32 j = 0; j < 2; j++) {
                    const auto& VertexId = MeshDescription->GetEdgeVertex(EdgeId, j);
                    VectorMapArray.Emplace(FVectorMap(VertexId, FVector(MeshDescription->GetVertexPosition(VertexId))));
                }
            }
        }

        return VectorMapArray;
    }

    /**
     * @brief Z軸の最大最小頂点の差分からその高さの半分を取得
     * @param VectorMapArray 頂点情報を格納したマップ
     * @return Z軸の最大最小頂点の差分からその高さを半分にした値
     */
    double GetVertexMapHalfZ(const TArray<FVectorMap>& VectorMapArray) {
        if (VectorMapArray.Num() <= 0) {
            return 0;
        }

        double MaxZ = VectorMapArray[0].VertexPos.Z;
        double MinZ = VectorMapArray[0].VertexPos.Z;
        const auto InVertsNum = VectorMapArray.Num();
        for (int32 i = 1; i < InVertsNum; i++) {
            if (MaxZ < VectorMapArray[i].VertexPos.Z) {
                MaxZ = VectorMapArray[i].VertexPos.Z;
            }
            if (VectorMapArray[i].VertexPos.Z < MinZ) {
                MinZ = VectorMapArray[i].VertexPos.Z;
            }
        }

        return MaxZ - (MaxZ - MinZ) * 0.5;
    }

    /**
     * @brief ULineBatchComponent取得
     * @param InWorld 現在のワールド
     * @param bPersistentLines 常にラインを表示するか？
     * @param LifeTime ラインのライフタイム
     * @param bDepthIsForeground 深度はForegroundか？
     * @return ULineBatchComponent
     */
    ULineBatchComponent* GetDebugLineBatch(const UWorld* InWorld, const bool bPersistentLines, const float LifeTime, const bool bDepthIsForeground) {
        return InWorld
                   ? (bDepthIsForeground
                          ? InWorld->ForegroundLineBatcher
                          : bPersistentLines || LifeTime > 0.f
                          ? InWorld->PersistentLineBatcher
                          : InWorld->LineBatcher)
                   : nullptr;
    }
}

void UPLATEAUSDKEditorUtilityWidget::AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const {
    AreaSelectSuccessDelegate.Broadcast(ReferencePoint, PackageMask);
}

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

void UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary::DrawPrimaryAttrInfo(const UWorld* WorldContextObject, const USceneComponent* SceneComponent) {
    const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(SceneComponent);
    if (StaticMeshComponent == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr"));
        return;
    }
    
    const auto& WorldLocation = StaticMeshComponent->GetComponentLocation();
    const auto& MeshDescription = StaticMeshComponent->GetStaticMesh()->GetMeshDescription(0);
    const auto& VectorMapArray = GetVectorMapArray(MeshDescription);
    TArray<FVector> PointArray;
    for (const auto& VectorMap : VectorMapArray) {
        auto VertexPos = VectorMap.VertexPos;
        VertexPos.Z = 0;
        PointArray.Add(VertexPos);
    }
    
    FVector OutRectCenter;
    FRotator OutRectRotation = FRotator::ZeroRotator;
    float OutRectLengthX;
    float OutRectLengthY;
    UKismetMathLibrary::MinAreaRectangle(nullptr, PointArray, FVector(0, 0, 1), OutRectCenter, OutRectRotation, OutRectLengthX, OutRectLengthY);
    const auto VertexMapHalfZ = GetVertexMapHalfZ(VectorMapArray);
    OutRectCenter.Z = VertexMapHalfZ + WorldLocation.Z;
    if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) {
        DrawDebugBox(World, OutRectCenter, FVector(OutRectLengthX, OutRectLengthY, VertexMapHalfZ) * 0.5 + DrawBoxXYZOffset,
                     OutRectRotation.Quaternion(), FColor::Red, GBPersistentLines, LineLifeTime, DepthPriority, Thickness);
    }
}

void UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary::DrawPrimaryAndAtomAttrInfo(const UWorld* WorldContextObject,
                                                                                const TArray<USceneComponent*> ChildSceneComponents,
                                                                                const USceneComponent* ChildSceneComponent) {
    if (ChildSceneComponents.Num() <= 0) {
        UE_LOG(LogTemp, Error, TEXT("ChildSceneComponents.Num() <= 0"));
        return;
    }

    const auto& ChildStaticMeshComponent = Cast<UStaticMeshComponent>(ChildSceneComponent);
    if (ChildStaticMeshComponent == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("ChildStaticMeshComponent == nullptr"));
        return;
    }

    // 子の頂点座標を全て取得
    TArray<FVector> PointArray;
    TArray<FVectorMap> ChildrenVectorMap;
    for (const auto& SceneComponent : ChildSceneComponents) {
        if (const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(SceneComponent)) {
            const auto& MeshDescription = StaticMeshComponent->GetStaticMesh()->GetMeshDescription(0);
            const auto& VectorMapArray = GetVectorMapArray(MeshDescription);
            ChildrenVectorMap.Append(VectorMapArray);
            for (const auto& VectorMap : VectorMapArray) {
                auto VertexPos = VectorMap.VertexPos;
                VertexPos.Z = 0;
                PointArray.Add(VertexPos);
            }
        }
    }

    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("World == nullptr"));
        return;
    }

    FVector OutRectCenter;
    FRotator OutRectRotation;
    float OutRectLengthX;
    float OutRectLengthY;
    UKismetMathLibrary::MinAreaRectangle(nullptr, PointArray, FVector(0, 0, 1), OutRectCenter, OutRectRotation, OutRectLengthX, OutRectLengthY);
    const auto VertexMapHalfZ = GetVertexMapHalfZ(ChildrenVectorMap);
    const auto& WorldLocation = ChildSceneComponent->GetComponentLocation();
    OutRectCenter.Z = VertexMapHalfZ + WorldLocation.Z;

    // 親を描画
    DrawDebugBox(World, OutRectCenter, FVector(OutRectLengthX, OutRectLengthY, VertexMapHalfZ) * 0.5 + DrawBoxXYZOffset,
                 OutRectRotation.Quaternion(), FColor::Red, GBPersistentLines, LineLifeTime, DepthPriority, Thickness);

    // 子を描画
    if (GEngine->GetNetMode(World) != NM_DedicatedServer) {
        const auto& VectorMapPerimeterArray = GetVectorMapPerimeterArray(ChildStaticMeshComponent->GetStaticMesh()->GetMeshDescription(0));
        const auto InVertsNum = VectorMapPerimeterArray.Num();
        ULineBatchComponent* const LineBatch = GetDebugLineBatch(World, GBPersistentLines, LineLifeTime, false);
        for (int32 Vert0 = InVertsNum - 1, Vert1 = 0; Vert1 < InVertsNum; Vert0 = Vert1++) {
            LineBatch->DrawLine(VectorMapPerimeterArray[Vert0].VertexPos, VectorMapPerimeterArray[Vert1].VertexPos, FColor::Green, DepthPriority, Thickness, LineLifeTime);
        }
    }
}

UStaticMeshComponent* UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary::GetParentStaticMeshComponent(USceneComponent* SceneComponent) {
    const auto& PLATEAUCityObjectGroup = Cast<UPLATEAUCityObjectGroup>(SceneComponent);
    if (PLATEAUCityObjectGroup == nullptr)
        return nullptr;

    PLATEAUCityObjectGroup->GetAllRootCityObjects();
    const auto& OutsideParent = PLATEAUCityObjectGroup->OutsideParent;
    if (OutsideParent.IsEmpty()) {
        return nullptr;
    }

    // 親を探す
    USceneComponent* ParentIterator = PLATEAUCityObjectGroup->GetAttachParent();
    while (ParentIterator != nullptr) {
        if (const auto& Parent = Cast<UPLATEAUCityObjectGroup>(ParentIterator); Parent->GetName().Contains(OutsideParent)) {
            return Cast<UStaticMeshComponent>(ParentIterator);
        }
        ParentIterator = ParentIterator->GetAttachParent();
    }

    return nullptr;
}
