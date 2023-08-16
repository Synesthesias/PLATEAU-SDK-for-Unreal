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
struct FVectorMap {
    GENERATED_BODY()

    FVectorMap() {
    }

    FVectorMap(const FVertexID InVertexID, const FVector InVertexPos) : VertexID(InVertexID), VertexPos(InVertexPos) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries")
    FVertexID VertexID;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries")
    FVector VertexPos;
};

UCLASS()
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidgetBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /**
     * @brief 対象のコンポーネントを線で囲む
     * @param WorldContextObject 現在のワールド
     * @param HitResult レイキャスト結果
     * @param SceneComponent 線で囲むシーンコンポーネント
     */
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries")
    static void DrawPrimaryAttrInfo(const UWorld* WorldContextObject, const FHitResult& HitResult, USceneComponent* SceneComponent);

    /**
     * @brief 親と子のコンポーネントを線で囲む
     * @param WorldContextObject 現在のワールド
     * @param HitResult レイキャスト結果
     * @param ChildSceneComponent 線で囲む子のシーンコンポーネント
     * @param ChildSceneComponents 線で囲む親の持つ全ての子のシーンコンポーネント
     */
    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries")
    static void DrawPrimaryAndAtomAttrInfo(const UWorld* WorldContextObject, const FHitResult& HitResult, USceneComponent* ChildSceneComponent,
                                           const TArray<USceneComponent*> ChildSceneComponents);

    /**
     * @brief 対象コンポーネントの親スタティックメッシュを取得
     * @param SceneComponent 親を持つか確認するシーンコンポーネント
     * @return 親のスタティックメッシュコンポーネント
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PLATEAU|BPLibraries")
    static UStaticMeshComponent* GetParentStaticMeshComponent(USceneComponent* SceneComponent);
};
