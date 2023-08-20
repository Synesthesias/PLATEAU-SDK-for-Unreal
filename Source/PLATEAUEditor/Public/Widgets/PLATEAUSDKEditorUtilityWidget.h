// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include <plateau/network/client.h>
#include "CoreMinimal.h"
#include "Blutility/Classes/EditorUtilityWidget.h"
#include "PLATEAUSDKEditorUtilityWidget.generated.h"

class UPLATEAUCityObjectGroup;

UENUM(BlueprintType)
enum class ETopMenuPanel : uint8 {
    None,
    ImportPanel,
    ModelAdjustmentPanel,
    ExportPanel,
    AttrInfoPanel
};

USTRUCT(BlueprintType)
struct FServerDatasetMetadata {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    FString Title;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    FString Description;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    FString ID;
};

USTRUCT(BlueprintType)
struct FServerDatasetMetadataMap {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    FString GroupTitle;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ImportPanel")
    TArray<FServerDatasetMetadata> ServerDatasetMetadataArray;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAreaSelectSuccessDelegate, FVector3d, ReferencePoint, int64, PackageMask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetDatasetMetaDataAsyncSuccessDelegate, const TArray<FServerDatasetMetadataMap>&, PLATEAUServerDatasetMetadataMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSelectionChangedDelegate, AActor*, SelectionActor, USceneComponent*, SelectionComponent, bool, IsActorChanged);

UCLASS(Blueprintable)
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidget : public UEditorUtilityWidget {
    GENERATED_BODY()
public:
    /**
     * @brief 範囲選択完了通知
     * @param ReferencePoint リファレンス位置 
     * @param PackageMask パッケージマスク
     */
    void AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const;

    /**
     * @brief クライアントポインタ取得
     * @return クライアントポインタ
     */
    std::shared_ptr<plateau::network::Client> GetClientPtr() {
        return ClientPtr;
    }

    /**
     * @brief 範囲選択成功デリゲート
     */
    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FAreaSelectSuccessDelegate AreaSelectSuccessDelegate;

    /**
     * @brief サーバのメタデータ受信成功デリゲート
     */
    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FGetDatasetMetaDataAsyncSuccessDelegate GetDatasetMetaDataAsyncSuccessDelegate;

    /**
     * @brief サーバから非同期でデータセット情報を取得
     * @param InServerURL サーバURL
     * @param InToken トークン
     */
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    void GetDatasetMetadataAsync(const FString& InServerURL, const FString& InToken);

    /**
     * @brief 選択イベント通知の実行制御
     * @param TopMenuPanel イベント通知するパネル
     */
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelAdjustmentPanel")
    void SetEnableSelectionChangedEvent(const ETopMenuPanel TopMenuPanel);

    /**
     * @brief 選択が変更された時に通知
     * @param InSelection 選択されたオブジェクト
     */
    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries")
    FOnSelectionChangedDelegate OnSelectionChangedDelegate;
private:
    bool bGettingNativeDatasetMetadata;
    std::shared_ptr<plateau::network::Client> ClientPtr;
    TArray<FServerDatasetMetadataMap> ServerDatasetMetadataMapArray;

    void OnSelectionChanged(UObject* InSelection);
    FDelegateHandle SelectionChangedEventHandle;
    AActor* SelectionActor;
    USceneComponent* SelectionComponent;
};

USTRUCT(BlueprintType)
struct FVertexData {
    GENERATED_BODY()

    FVertexData() {
    }

    FVertexData(const FVertexID InVertexID, const FVector InVertexPos) : VertexID(InVertexID), VertexPos(InVertexPos) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries")
    FVertexID VertexID;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries")
    FVector VertexPos;
};

struct FEdgeData {
    FEdgeData() {
    }

    FEdgeData(const FVector InVertexPos0, const FVector InVertexPos1) : VertexPos0(InVertexPos0), VertexPos1(InVertexPos1) {
    }

    FVector VertexPos0;
    FVector VertexPos1;
};

UCLASS()
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /**
     * @brief 親と子のコンポーネントを線で囲む
     * @param WorldContextObject 現在のワールド
     * @param HitResult レイキャスト結果
     * @param LodIndex LOD
     */
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries")
    static void DrawAttrInfo(const UWorld* WorldContextObject, const FHitResult& HitResult, const int32 LodIndex = 0);

    /**
     * @brief 親と子のコンポーネントを線で囲む
     * @param WorldContextObject 現在のワールド
     * @param HitResult レイキャスト結果
     * @param ChildSceneComponents 線で囲む親の持つ全ての子のシーンコンポーネント
     * @param LodIndex LOD
     */
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries")
    static void DrawAttrInfoWithChildSceneComponents(const UWorld* WorldContextObject, const FHitResult& HitResult, const TArray<USceneComponent*> ChildSceneComponents, const int32 LodIndex = 0);

    /**
     * @brief 対象コンポーネントの親スタティックメッシュを取得
     * @param SceneComponent 親を持つか確認するシーンコンポーネント
     * @return 親のスタティックメッシュコンポーネント
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|BPLibraries")
    static UStaticMeshComponent* GetParentStaticMeshComponent(USceneComponent* SceneComponent);
};
