// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "AdvancedPreviewSceneModule.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUEditor/Private/ExtentEditor/PLATEAUMeshCodeGizmo.h"
#include <plateau/network/client.h>

class SDockTab;

namespace plateau::dataset {
    enum class PredefinedCityModelPackage : uint32;
}

/**
 * @brief 範囲選択画面の表示、操作、情報取得、設定を行うためのインスタンスメソッドを提供します。
 *
 */
class PLATEAUEDITOR_API FPLATEAUExtentEditor : public TSharedFromThis<FPLATEAUExtentEditor> {
public:
    FPLATEAUExtentEditor();
    ~FPLATEAUExtentEditor();

    static const FName TabId;

    void RegisterTabSpawner(const TSharedRef<class FTabManager>& TabManager);
    void UnregisterTabSpawner(const TSharedRef<class FTabManager>& TabManager);

    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);

    const FString& GetSourcePath() const;
    void SetSourcePath(const FString& Path);

    const FString& GetAreaSourcePath() const;
    void SetAreaSourcePath(const FString& InAreaSourcePath);

    bool IsSelectedArea() const;
    TArray<FString> GetSelectedCodes(const bool InbImportFromServer) const;
    TMap<FString, FPLATEAUMeshCodeGizmo> GetAreaMeshCodeMap() const;
    void SetAreaMeshCodeMap(const FString& MeshCode, const FPLATEAUMeshCodeGizmo& MeshCodeGizmo);
    void ResetAreaMeshCodeMap();

    FPLATEAUGeoReference GetGeoReference() const;
    void SetGeoReference(const FPLATEAUGeoReference& InGeoReference);

    bool IsImportFromServer() const;
    void SetImportFromServer(const bool InbImportFromServer);

    std::shared_ptr<plateau::network::Client> GetClientPtr() const;
    void SetClientPtr(const std::shared_ptr<plateau::network::Client>& InClientPtr);

    const std::string& GetServerDatasetID() const;
    void SetServerDatasetID(const std::string& InID);

    const plateau::dataset::PredefinedCityModelPackage& GetLocalPackageMask() const;
    void SetLocalPackageMask(const plateau::dataset::PredefinedCityModelPackage& InPackageMask);

    const plateau::dataset::PredefinedCityModelPackage& GetServerPackageMask() const;
    plateau::geometry::GeoCoordinate GetSelectedCenterLatLon(const bool InbImportFromServer) const;
    FVector3d GetSelectedCenterPoint(const int InZoneID, const bool InbImportFromServer) const;
    void SetServerPackageMask(const plateau::dataset::PredefinedCityModelPackage& InPackageMask);
private:
    FString SourcePath;
    FString AreaSourcePath;
    TMap<FString, FPLATEAUMeshCodeGizmo> LocalAreaMeshCodeMap;
    TMap<FString, FPLATEAUMeshCodeGizmo> ServerAreaMeshCodeMap;
    FPLATEAUGeoReference GeoReference;

    bool bImportFromServer = false;
    std::shared_ptr<plateau::network::Client> ClientPtr;
    std::string ServerDatasetID;
    plateau::dataset::PredefinedCityModelPackage LocalPackageMask;
    plateau::dataset::PredefinedCityModelPackage ServerPackageMask;

    FAdvancedPreviewSceneModule::FOnPreviewSceneChanged OnPreviewSceneChangedDelegate;
    TWeakObjectPtr<class APLATEAUCityModelLoader> Loader;
};
