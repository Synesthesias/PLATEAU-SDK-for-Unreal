// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUEditor/Public/Widgets/PLATEAUSDKEditorUtilityWidget.h"
#include <algorithm>
#include "MeshDescription.h"
#include "PLATEAUCityObjectGroup.h"
#include "Selection.h"
#include "StaticMeshAttributes.h"
#include "Async/Async.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/LineBatchComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsSettings.h"


namespace {
    // 線パラメータ
    constexpr float DrawLineThickness = 20;
    constexpr float DrawLineLifeTime = -1;
    constexpr bool GBPersistentLines = true;
    constexpr uint8 DepthPriority = SDPG_World;

    /**
     * @brief 対象のエッジデータ配列内に同じエッジが存在するか？
     * @param EdgeDataArray チェック対象のエッジデータ配列
     * @param CheckEdgeData チェック対象のエッジデータ
     * @param ExistingIndex 同じエッジが存在したインデックス
     * @return 同じエッジが存在したか否か？
     */
    bool IsExistingEdge(const TArray<FEdgeData> EdgeDataArray, const FEdgeData CheckEdgeData, int32& ExistingIndex) {
        for (int32 i = 0; i < EdgeDataArray.Num(); i++) {
            // 同じ頂点を持つエッジか？
            if (EdgeDataArray[i].VertexPos0 == CheckEdgeData.VertexPos0 && EdgeDataArray[i].VertexPos1 == CheckEdgeData.VertexPos1 ||
                EdgeDataArray[i].VertexPos0 == CheckEdgeData.VertexPos1 && EdgeDataArray[i].VertexPos1 == CheckEdgeData.VertexPos0) {
                ExistingIndex = i;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 入力されたメッシュのポリゴンから全ポリゴンを囲むエッジとなる頂点を求める    
     * @param AtomicMeshDescription 最小地物のメッシュディスクリプション
     * @return 全ポリゴンを囲むエッジとなる頂点情報を持つ構造体の配列
     */
    TArray<FVertexData> GetVertexDataPerimeterArray(const FMeshDescription* AtomicMeshDescription) {
        TArray<FVertexData> VectorMapArray;
        const auto PolygonCount = AtomicMeshDescription->Polygons().Num();
        for (int32 i = 0; i < PolygonCount; i++) {
            const auto& EdgeIdArray = AtomicMeshDescription->GetPolygonPerimeterEdges(i);
            for (const auto& EdgeId : EdgeIdArray) {
                // エッジの頂点は必ず２つある
                for (int32 j = 0; j < 2; j++) {
                    const auto& VertexId = AtomicMeshDescription->GetEdgeVertex(EdgeId, j);
                    const auto& VertexPos = FVector(AtomicMeshDescription->GetVertexPosition(VertexId));
                    bool ExistPos = false;
                    for (const auto& VectorMap : VectorMapArray) {
                        if (VectorMap.VertexID == VertexId || (VectorMap.VertexPos - VertexPos).Length() < DBL_EPSILON) {
                            ExistPos = true;
                            break;
                        }
                    }
                    // 同じ頂点ID、同じ座標は追加しない
                    if (!ExistPos) {
                        VectorMapArray.Emplace(FVertexData(VertexId, VertexPos));
                    }
                }
            }
        }
        // 頂点IDでソート
        VectorMapArray.Sort([](const FVertexData& A, const FVertexData& B) { return A.VertexID < B.VertexID; });

        return VectorMapArray;
    }

    /**
     * @brief 入力された頂点座標から全ポリゴンを囲むエッジとなる頂点をエッジを結ぶ順番を考慮して求める    
     * @param EdgeDataArray 対象のエッジデータ配列
     * @return 全ポリゴンを囲むエッジとなる頂点情報を持つ構造体の配列
     */
    TArray<FEdgeData> GetEdgeDataPerimeterArray(const TArray<FEdgeData> EdgeDataArray) {
        // 同じエッジは取り除く
        TArray<FEdgeData> DistinctEdgeDataArray;
        for (const auto& EdgeData : EdgeDataArray) {
            if (int32 ExistingIndex; IsExistingEdge(DistinctEdgeDataArray, EdgeData, ExistingIndex)) {
                DistinctEdgeDataArray.RemoveAt(ExistingIndex);
            } else {
                DistinctEdgeDataArray.Emplace(EdgeData);
            }
        }

        // エッジ同士を結ぶ順番にソート
        for (int32 i = 0; i < DistinctEdgeDataArray.Num(); i++) {
            for (int32 j = 0; j < DistinctEdgeDataArray.Num(); j++) {
                // 自分自身との比較は行わない
                if (DistinctEdgeDataArray[i].VertexPos0 == DistinctEdgeDataArray[j].VertexPos0 && DistinctEdgeDataArray[i].VertexPos1 == DistinctEdgeDataArray[j].VertexPos1)
                    continue;

                // エッジの結び先か？
                if (DistinctEdgeDataArray[i].VertexPos1 == DistinctEdgeDataArray[j].VertexPos0) {
                    int32 SwapIndex = i + 1;
                    if (DistinctEdgeDataArray.Num() <= SwapIndex)
                        SwapIndex = 0;

                    DistinctEdgeDataArray.Swap(SwapIndex, j);
                    break;
                }
            }
        }

        return DistinctEdgeDataArray;
    }

    /**
     * @brief 入力されたMeshDescriptionから全ての頂点を求める    
     * @return メッシュの全頂点
     */
    TArray<FVertexData> GetVertexDataArray(const FMeshDescription* MeshDescription) {
        TArray<FVertexData> VectorMapArray;
        const auto PolygonCount = MeshDescription->Polygons().Num();
        for (int32 i = 0; i < PolygonCount; i++) {
            const auto& EdgeIdArray = MeshDescription->GetPolygonPerimeterEdges(i);
            for (const auto& EdgeId : EdgeIdArray) {
                // エッジの頂点は必ず２つある
                for (int32 j = 0; j < 2; j++) {
                    const auto& VertexId = MeshDescription->GetEdgeVertex(EdgeId, j);
                    VectorMapArray.Emplace(FVertexData(VertexId, FVector(MeshDescription->GetVertexPosition(VertexId))));
                }
            }
        }

        return VectorMapArray;
    }

    /**
     * @brief Z軸の最大最小値取得
     * @param VectorMapArray 頂点情報を格納したマップ
     * @param MinValue Z軸の最大値
     * @param MaxValue Z軸の最小値
     */
    void GetMinMaxZ(const TArray<FVertexData>& VectorMapArray, int32& MinValue, int32& MaxValue) {
        if (VectorMapArray.Num() <= 0) {
            MinValue = 0;
            MaxValue = 0;
            return;
        }

        double MinZ = VectorMapArray[0].VertexPos.Z;
        double MaxZ = VectorMapArray[0].VertexPos.Z;
        const auto InVertsNum = VectorMapArray.Num();
        for (int32 i = 1; i < InVertsNum; i++) {
            MinZ = std::min(MinZ, VectorMapArray[i].VertexPos.Z);
            MaxZ = std::max(MaxZ, VectorMapArray[i].VertexPos.Z);
        }
        MinValue = MinZ;
        MaxValue = MaxZ;
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

    /**
     * @brief レイキャストヒットしたメッシュ内で指定UVと同じUVを持つエッジを重複なしで全て取得
     * @param HitResult レイキャストヒット結果
     * @param UV 取得したい頂点のUV
     * @param UVChannel 対象のUVチャンネル
     * @param LodIndex メッシュのLOD
     * @return 指定UVと同じUVを持つエッジ配列
     */
    TArray<FEdgeData> GetEdgesByUV(const FHitResult& HitResult, const FVector2D& UV, const int32 UVChannel = 3, const int32 LodIndex = 0) {
        if (!UPhysicsSettings::Get()->bSupportUVFromHitResults) {
            UE_LOG(LogTemp, Warning, TEXT("Calling FindCollisionUV but 'Support UV From Hit Results' is not enabled in project settings. This is required for finding UV for collision results."));
            return TArray<FEdgeData>();
        }

        const auto& HitPrimitiveComponent = HitResult.Component.Get();
        if (HitPrimitiveComponent == nullptr) {
            UE_LOG(LogTemp, Error, TEXT("HitPrimitiveComponent == nullptr"));
            return TArray<FEdgeData>();
        }

        const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(HitPrimitiveComponent);
        if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr) {
            UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr or StaticMeshComponent->GetStaticMesh() == nullptr"));
            return TArray<FEdgeData>();
        }

        const auto& BodySetup = HitPrimitiveComponent->GetBodySetup();
        if (BodySetup == nullptr) {
            UE_LOG(LogTemp, Error, TEXT("BodySetup == nullptr"));
            return TArray<FEdgeData>();
        }

        TArray<FEdgeData> EdgeDataArray;
        const auto& StaticMesh = StaticMeshComponent->GetStaticMesh();
        const auto& [IndexBuffer, VertPositions, VertUVs] = BodySetup->UVInfo;
        for (int32 FaceIndex = 0; FaceIndex < StaticMesh->GetNumTriangles(LodIndex); ++FaceIndex) {
            if (VertUVs.IsValidIndex(UVChannel) && IndexBuffer.IsValidIndex(FaceIndex * 3 + 2)) {
                const int32 Index0 = IndexBuffer[FaceIndex * 3 + 0];
                const int32 Index1 = IndexBuffer[FaceIndex * 3 + 1];
                const int32 Index2 = IndexBuffer[FaceIndex * 3 + 2];

                FVector Pos0 = VertPositions[Index0];
                FVector Pos1 = VertPositions[Index1];
                FVector Pos2 = VertPositions[Index2];

                FVector2D UV0 = VertUVs[UVChannel][Index0];
                FVector2D UV1 = VertUVs[UVChannel][Index1];
                FVector2D UV2 = VertUVs[UVChannel][Index2];

                // Find barycentric coords
                // 第一引数に自身の頂点を与えることで必ず同じBaryCoordsが得られるようにしている（FaceIndexによってのみUVが変化する）
                const FVector BaryCoords = FMath::ComputeBaryCentric2D(Pos0, Pos0, Pos1, Pos2);
                // Use to blend UVs
                const auto& TargetUV = BaryCoords.X * UV0 + BaryCoords.Y * UV1 + BaryCoords.Z * UV2;
                // 同じUVを持つ頂点取得
                if (static_cast<int32>(TargetUV.X) == static_cast<int32>(UV.X) && static_cast<int32>(TargetUV.Y) == static_cast<int32>(UV.Y)) {
                    EdgeDataArray.Emplace(Pos0, Pos1);
                    EdgeDataArray.Emplace(Pos1, Pos2);
                    EdgeDataArray.Emplace(Pos2, Pos0);
                }
            }
        }

        return EdgeDataArray;
    }
}

void UPLATEAUSDKEditorUtilityWidget::AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const {
    AreaSelectSuccessDelegate.Broadcast(ReferencePoint, PackageMask);
}

void UPLATEAUSDKEditorUtilityWidget::CloseAreaSelectionWindowInvoke() const {
    CloseAreaSelectionWindowDelegate.Broadcast();
}

void UPLATEAUSDKEditorUtilityWidget::ClosePLATEAUSDKEuwInvoke() const {
    ClosePLATEAUSDKEuwDelegate.Broadcast();
}

void UPLATEAUSDKEditorUtilityWidget::GetDatasetMetadataAsync(const FString& InServerURL, const FString& InToken) {
    if (bGettingNativeDatasetMetadata) return;

    ClientPtr = std::make_shared<plateau::network::Client>(TCHAR_TO_UTF8(*InServerURL), TCHAR_TO_UTF8(*InToken));

    Async(EAsyncExecution::Thread, [
        bGettingNativeDatasetMetadata = bGettingNativeDatasetMetadata,
        ServerDatasetMetadataMapArray = &ServerDatasetMetadataMapArray,
        ClientPtr = ClientPtr,
        GetDatasetMetaDataAsyncSuccessDelegate = &GetDatasetMetaDataAsyncSuccessDelegate
        ]() mutable {
        bGettingNativeDatasetMetadata = true;
        ServerDatasetMetadataMapArray->Reset();
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
            ServerDatasetMetadataMapArray->Emplace(ServerDatasetMetadataMap);
        }

        FFunctionGraphTask::CreateAndDispatchWhenReady([GetDatasetMetaDataAsyncSuccessDelegate, ServerDatasetMetadataMapArray] {
            GetDatasetMetaDataAsyncSuccessDelegate->Broadcast(*ServerDatasetMetadataMapArray);
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

void UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary::DrawAttrInfo(const UWorld* WorldContextObject, const FHitResult& HitResult, const int32 LodIndex) {
    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("World == nullptr"));
        return;
    }

    const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(HitResult.Component.Get());
    if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr or StaticMeshComponent->GetStaticMesh() == nullptr"));
        return;
    }

    // 親の頂点座標を平面座標として全て取得
    const auto& StaticMesh = StaticMeshComponent->GetStaticMesh();
    const auto& MeshDescription = StaticMesh->GetMeshDescription(LodIndex);
    const auto& VectorMapArray = GetVertexDataArray(MeshDescription);
    TArray<FVector> ParentVerticesArray;
    for (const auto& VectorMap : VectorMapArray) {
        auto VertexPos = VectorMap.VertexPos;
        VertexPos.Z = 0;
        ParentVerticesArray.Add(VertexPos);
    }

    FVector OutRectCenter;
    FRotator OutRectRotation = FRotator::ZeroRotator;
    float OutRectLengthX;
    float OutRectLengthY;
    UKismetMathLibrary::MinAreaRectangle(nullptr, ParentVerticesArray, FVector(0, 0, 1), OutRectCenter, OutRectRotation, OutRectLengthX, OutRectLengthY);
    const auto& WorldLocation = StaticMeshComponent->GetComponentLocation();
    int32 MinZ;
    int32 MaxZ;
    GetMinMaxZ(VectorMapArray, MinZ, MaxZ);
    OutRectCenter.Z = MinZ + (MaxZ - MinZ) * 0.5 + WorldLocation.Z;
    
    // 親のバウンディングボックス描画
    DrawDebugBox(World, OutRectCenter, FVector(OutRectLengthX, OutRectLengthY, MaxZ - MinZ) * 0.5, OutRectRotation.Quaternion(),
                 FColor::Magenta, GBPersistentLines, DrawLineLifeTime, DepthPriority, DrawLineThickness);

    // 親の属性情報表示
    const auto& CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(StaticMeshComponent);
    if (CityObjectGroup != nullptr) {
        const auto& [GmlID, CityObjectIndex, Type, Attributes, Children] = CityObjectGroup->GetPrimaryCityObjectByRaycast(HitResult);
    OutRectCenter.Z = MaxZ + WorldLocation.Z;
        FString AttrInfoString;
        for (const auto& AttributeMap : Attributes.AttributeMap) {
            if (AttributeMap.Value.Type == EPLATEAUAttributeType::String) {
                AttrInfoString = FString::Format(TEXT("Key: {0}, Value: {1}"), {AttributeMap.Key, AttributeMap.Value.StringValue});
                break;
            }
        }
        const FString DrawString = FString::Format(TEXT("{0}\n{1}"), {GmlID, AttrInfoString});
        DrawDebugString(World, OutRectCenter, DrawString, nullptr, FColor::Magenta, -1);        
    }

    // 子のバウンディングボックス描画
    FVector2d UV;
    UPLATEAUCityObjectGroup::FindCollisionUV(HitResult, UV);
    const auto& ChildEdgeArray = GetEdgesByUV(HitResult, UV);
    const auto& EdgePerimeterArray = GetEdgeDataPerimeterArray(ChildEdgeArray);
    ULineBatchComponent* const LineBatch = GetDebugLineBatch(World, GBPersistentLines, DrawLineLifeTime, false);
    for (int32 i = 0; i < EdgePerimeterArray.Num(); i++) {
        LineBatch->DrawLine(EdgePerimeterArray[i].VertexPos0, EdgePerimeterArray[i].VertexPos1, FColor::Green, DepthPriority, DrawLineThickness, DrawLineLifeTime);
    }
    
    // 子の属性情報表示
    const auto& [GmlID, CityObjectIndex, Type, Attributes, Children] = CityObjectGroup->GetAtomicCityObjectByRaycast(HitResult);
    FString AttrInfoString;
    for (const auto& AttributeMap : Attributes.AttributeMap) {
        if (AttributeMap.Value.Type == EPLATEAUAttributeType::String) {
            AttrInfoString = FString::Format(TEXT("Key: {0}, Value: {1}"), {AttributeMap.Key, AttributeMap.Value.StringValue});
            break;
        }
    }
    FVector SumVertPos;
    for (int32 i = 0; i < EdgePerimeterArray.Num(); i++) {
        SumVertPos += EdgePerimeterArray[i].VertexPos0;
    }
    SumVertPos /= EdgePerimeterArray.Num();
    const FString DrawString = FString::Format(TEXT("{0}\n{1}"), {GmlID, AttrInfoString});
    DrawDebugString(World, SumVertPos + WorldLocation, DrawString, nullptr, FColor::Green, -1);    
}

void UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary::DrawAttrInfoWithChildSceneComponents(const UWorld* WorldContextObject, const FHitResult& HitResult,
                                                                                          const TArray<USceneComponent*> ChildSceneComponents,
                                                                                          const int32 LodIndex) {
    if (ChildSceneComponents.Num() <= 0) {
        UE_LOG(LogTemp, Error, TEXT("ChildSceneComponents.Num() <= 0"));
        return;
    }

    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (World == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("World == nullptr"));
        return;
    }

    const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(HitResult.Component.Get());
    if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr or StaticMeshComponent->GetStaticMesh() == nullptr"));
        return;
    }
    
    // 子の頂点座標を平面座標として全て取得
    TArray<FVector> ChildVerticesArray;
    TArray<FVertexData> ChildrenVectorMap;
    for (const auto& ChildSceneComponent : ChildSceneComponents) {
        if (const auto& ChildStaticMeshComponent = Cast<UStaticMeshComponent>(ChildSceneComponent)) {
            const auto& MeshDescription = ChildStaticMeshComponent->GetStaticMesh()->GetMeshDescription(0);
            const auto& VectorMapArray = GetVertexDataArray(MeshDescription);
            ChildrenVectorMap.Append(VectorMapArray);
            for (const auto& VectorMap : VectorMapArray) {
                auto VertexPos = VectorMap.VertexPos;
                VertexPos.Z = 0;
                ChildVerticesArray.Add(VertexPos);
            }
        }
    }

    FVector OutRectCenter;
    FRotator OutRectRotation;
    float OutRectLengthX;
    float OutRectLengthY;
    UKismetMathLibrary::MinAreaRectangle(nullptr, ChildVerticesArray, FVector(0, 0, 1), OutRectCenter, OutRectRotation, OutRectLengthX, OutRectLengthY);
    const auto& WorldLocation = StaticMeshComponent->GetComponentLocation();
    int32 MinZ;
    int32 MaxZ;
    GetMinMaxZ(ChildrenVectorMap, MinZ, MaxZ);
    OutRectCenter.Z = MinZ + (MaxZ - MinZ) * 0.5 + WorldLocation.Z;

    // 親のバウンディングボックス描画
    DrawDebugBox(World, OutRectCenter, FVector(OutRectLengthX, OutRectLengthY, MaxZ - MinZ) * 0.5, OutRectRotation.Quaternion(),
                 FColor::Magenta, GBPersistentLines, DrawLineLifeTime, DepthPriority, DrawLineThickness);

    // 親の属性情報表示
    const auto& CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(StaticMeshComponent);
    if (CityObjectGroup != nullptr) {
        const auto& [GmlID, CityObjectIndex, Type, Attributes, Children] = CityObjectGroup->GetPrimaryCityObjectByRaycast(HitResult);
        OutRectCenter.Z = MaxZ + WorldLocation.Z;
        FString AttrInfoString;
        for (const auto& AttributeMap : Attributes.AttributeMap) {
            if (AttributeMap.Value.Type == EPLATEAUAttributeType::String) {
                AttrInfoString = FString::Format(TEXT("Key: {0}, Value: {1}"), {AttributeMap.Key, AttributeMap.Value.StringValue});
                break;
            }
        }
        const FString DrawString = FString::Format(TEXT("{0}\n{1}"), {GmlID, AttrInfoString});
        DrawDebugString(World, OutRectCenter, DrawString, nullptr, FColor::Magenta, -1);
    }

    // 子のバウンディングボックス描画
    const auto& VertexDataPerimeterArray = GetVertexDataPerimeterArray(StaticMeshComponent->GetStaticMesh()->GetMeshDescription(LodIndex));
    const auto InVertsNum = VertexDataPerimeterArray.Num();
    ULineBatchComponent* const LineBatch = GetDebugLineBatch(World, GBPersistentLines, DrawLineLifeTime, false);
    for (int32 Vert0 = InVertsNum - 1, Vert1 = 0; Vert1 < InVertsNum; Vert0 = Vert1++) {
        LineBatch->DrawLine(VertexDataPerimeterArray[Vert0].VertexPos, VertexDataPerimeterArray[Vert1].VertexPos, FColor::Green, DepthPriority, DrawLineThickness, DrawLineLifeTime);
    }

    // 子の属性情報表示
    const auto& [GmlID, CityObjectIndex, Type, Attributes, Children] = CityObjectGroup->GetAtomicCityObjectByRaycast(HitResult);
    FString AttrInfoString;
    for (const auto& AttributeMap : Attributes.AttributeMap) {
        if (AttributeMap.Value.Type == EPLATEAUAttributeType::String) {
            AttrInfoString = FString::Format(TEXT("Key: {0}, Value: {1}"), {AttributeMap.Key, AttributeMap.Value.StringValue});
            break;
        }
    }
    FVector SumVertPos;
    for (int32 Vert0 = 0; Vert0 < InVertsNum; Vert0++) {
        SumVertPos += VertexDataPerimeterArray[Vert0].VertexPos;
    }
    SumVertPos /= InVertsNum;
    const FString DrawString = FString::Format(TEXT("{0}\n{1}"), {GmlID, AttrInfoString});
    DrawDebugString(World, SumVertPos + WorldLocation, DrawString, nullptr, FColor::Green, -1);
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
