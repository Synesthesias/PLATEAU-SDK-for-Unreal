// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include <plateau/network/client.h>
#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAreaSelectSuccessDelegate, FVector, ReferencePoint, int64, PackageMask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCloseAreaSelectionWindowDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetDatasetMetaDataAsyncSuccessDelegate, const TArray<FServerDatasetMetadataMap>&, PLATEAUServerDatasetMetadataMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSelectionChangedDelegate, AActor*, SelectionActor, USceneComponent*, SelectionComponent, bool, IsActorChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FClosePLATEAUSDKEuwDelegate);

UCLASS(Blueprintable)
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidget : public UEditorUtilityWidget {
    GENERATED_BODY()
public:
    /**
     * @brief 範囲選択完了通知
     * @param ReferencePoint リファレンス位置 
     * @param PackageMask パッケージマスク
     */
    void AreaSelectSuccessInvoke(const FVector& ReferencePoint, const int64& PackageMask) const;

    /**
     * @brief 範囲選択ウィンドウクローズ通知
     */
    void CloseAreaSelectionWindowInvoke() const;

    /**
    * @brief SDKウィンドウクローズ通知
    */
    void ClosePLATEAUSDKEuwInvoke() const;
    
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
     * @brief 範囲選択ウィンドウクローズデリゲート
     */
    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FCloseAreaSelectionWindowDelegate CloseAreaSelectionWindowDelegate;
    
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

    /**
     * @brief SDKウィンドウクローズデリゲート
     */
    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries")
    FClosePLATEAUSDKEuwDelegate ClosePLATEAUSDKEuwDelegate;

private:
    bool bGettingNativeDatasetMetadata;
    std::shared_ptr<plateau::network::Client> ClientPtr;
    TArray<FServerDatasetMetadataMap> ServerDatasetMetadataMapArray;

    void OnSelectionChanged(UObject* InSelection);
    FDelegateHandle SelectionChangedEventHandle;
    AActor* SelectionActor;
    USceneComponent* SelectionComponent;
};