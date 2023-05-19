// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include <plateau/network/client.h>
#include "CoreMinimal.h"
#include "Blutility/Classes/EditorUtilityWidget.h"
#include "PLATEAUSDKEditorUtilityWidget.generated.h"


UENUM(BlueprintType)
enum class ETopMenuPanel : uint8 {
    None,
    ImportPanel,
    ModelAdjustmentPanel,
    ExportPanel
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

UCLASS(Blueprintable)
class PLATEAUEDITOR_API UPLATEAUSDKEditorUtilityWidget : public UEditorUtilityWidget {
    GENERATED_BODY()
public:
    void AreaSelectSuccessInvoke(const FVector3d& ReferencePoint, const int64& PackageMask) const;

    std::shared_ptr<plateau::network::Client> GetClientPtr() {
        return ClientPtr;
    }

    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FAreaSelectSuccessDelegate AreaSelectSuccessDelegate;

    UPROPERTY(BlueprintAssignable, Category = "PLATEAU|BPLibraries|ImportPanel")
    FGetDatasetMetaDataAsyncSuccessDelegate GetDatasetMetaDataAsyncSuccessDelegate;

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    void GetDatasetMetadataAsync(const FString& InServerURL, const FString& InToken);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelAdjustmentPanel")
    void SetEnableSelectionChangedEvent(const ETopMenuPanel TopMenuPanel);

    UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "PLATEAU|BPLibraries|ModelAdjustmentPanel")
    void OnEditorSelectionChanged();
private:
    bool bGettingNativeDatasetMetadata;
    std::shared_ptr<plateau::network::Client> ClientPtr;
    TArray<FServerDatasetMetadataMap> ServerDatasetMetadataMapArray;

    void OnSelectionChanged(UObject* InSelection);
    FDelegateHandle SelectionChangedEventHandle;
};
