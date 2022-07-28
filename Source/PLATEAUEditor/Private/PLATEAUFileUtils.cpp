#include <PLATEAUFileUtils.h>
#include <plateau/mesh/mesh_converter.h>
#include <plateau/udx/gml_file_info.h>

#include "AssetSelection.h"
#include "AutomatedAssetImportData.h"
#include "Factories/FbxSceneImportFactory.h"
#include "UObject/UObjectIterator.h"

#include "AssetTools/Private/AssetTools.h"
#include "AssetToolsModule.h"
#include "CityModelImportData.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "CityMapDetails/PLATEAUCityMapDetails.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Factories/CityModelImportDataFactory.h"
#include "Factories/FbxSceneImportOptions.h"
#include "Kismet/GameplayStatics.h"
#include "CityMapDetails/PLATEAUCityMapDetails.h"

namespace {
    /// <summary>
    /// <typeparamref name="TFactory"/>型のCDOを検索します。
    /// </summary>
    /// <typeparam name="TFactory">UFactoryの継承型</typeparam>
    /// <returns><typeparamref name="TFactory"/>型のCDO</returns>
    template <class TFactory>
    TFactory* FindFactory() {
        for (UClass* object : TObjectRange<UClass>()) {
            if (object->IsChildOf<TFactory>()) {
                return object->GetDefaultObject<TFactory>();
            }
        }
        return nullptr;
    }

    /// <summary>
    /// <paramref name="objects"/>から<typeparamref name="TObject"/>型の最初の要素を取得します。
    /// </summary>
    /// <typeparam name="TObject"></typeparam>
    /// <param name="objects"></param>
    /// <returns></returns>
    template <class TObject>
    TObject* FindObject(const TArray<UObject*> objects) {
        for (UObject* object : objects) {
            if (object->IsA(TObject::StaticClass())) {
                return Cast<TObject>(object);
            }
        }
        return nullptr;
    }

    /// <summary>
    /// <paramref name="objects"/>から<typeparamref name="TObject"/>型の要素を全て取得します。
    /// </summary>
    /// <typeparam name="TObject"></typeparam>
    /// <param name="objects"></param>
    /// <returns></returns>
    template <class TObject>
    TArray<TObject*> FindObjects(const TArray<UObject*> objects) {
        TArray<TObject*> Result;
        for (UObject* object : objects) {
            if (object->IsA(TObject::StaticClass())) {
                Result.Add(Cast<TObject>(object));
            }
        }
        return Result;
    }

    UCityModelImportData* CreateMetadataAsset(const FString& DestinationPath) {
        const auto* metadataFactory = FindFactory<UCityModelImportDataFactory>();
        if (metadataFactory != nullptr) {
            const FString assetName = TEXT("CityMapMetadata");
            const FString packageName = DestinationPath + TEXT("/") + assetName;
            UPackage* assetPackage = CreatePackage(*packageName);
            const EObjectFlags flags = RF_Public | RF_Standalone;

            auto* metadata =
                Cast<UCityModelImportData>(metadataFactory->CreateOrOverwriteAsset(metadataFactory->GetSupportedClass(), assetPackage, FName(*assetName), flags));
            assetPackage->SetDirtyFlag(true);
            return metadata;
        }
        return nullptr;
    }
}

void PLATEAUFileUtils::ImportFbx(const TArray<FString>& GmlFiles, const FString& DestinationPath, const TMap<ECityModelPackage, MeshConvertOptions>& MeshConvertOptionsMap) {
    const auto Metadata = CreateMetadataAsset(DestinationPath);
    const auto DestinationFullPath = DestinationPath.Replace(*FString("/Game"), *FPaths::ProjectContentDir());

    // メッシュ変換
    MeshConverter MeshConverter;
    auto& CityModelInfoArray = Metadata->ImportedCityModelInfoArray;
    CityModelInfoArray.SetNum(GmlFiles.Num());
    for (int i = 0; i < CityModelInfoArray.Num(); ++i) {
        const auto& GmlFile = GmlFiles[i];
        auto& CityModelInfo = CityModelInfoArray[i];
        // TODO: Refactor
        FString RelativeGmlPath = GmlFile.RightChop(GmlFile.Find(_TEXT("PLATEAU"), ESearchCase::CaseSensitive, ESearchDir::FromEnd) + 8);
        CityModelInfo.GmlFilePath = RelativeGmlPath;

        const auto FileInfo = GmlFileInfo(TCHAR_TO_UTF8(*GmlFile));
        const ECityModelPackage Package = FCityModelPlacementSettings::GetPackage(UTF8_TO_TCHAR(FileInfo.getFeatureType().c_str()));
        CityModelInfo.Package = Package;

        const auto MeshConvertOptions = MeshConvertOptionsMap[Package];
        Metadata->MeshConvertSettings.IsPerCityModelArea = MeshConvertOptions.mesh_granularity == MeshGranularity::PerCityModelArea;
        MeshConverter.setOptions(MeshConvertOptions);
        const auto MeshFilePathVector = MeshConverter.convert(TCHAR_TO_UTF8(*DestinationFullPath), FileInfo.getPath(),
            nullptr);
        TArray<FString> MeshFilePaths;
        MeshFilePaths.SetNum(MeshFilePathVector->size());
        for (int MeshIndex = 0; MeshIndex < MeshFilePathVector->size(); ++MeshIndex) {
            MeshFilePaths[MeshIndex] = UTF8_TO_TCHAR((*MeshFilePathVector)[MeshIndex].c_str());
        }

        // アセット化
        auto* sceneImportFactory = FindFactory<UFbxSceneImportFactory>();

        if (sceneImportFactory) {
            auto* automatedData = NewObject<UAutomatedAssetImportData>();
            automatedData->DestinationPath = DestinationPath;
            automatedData->Filenames = MeshFilePaths;
            automatedData->Factory = sceneImportFactory;
            sceneImportFactory->SceneImportOptions->bCreateContentFolderHierarchy = 1;
            sceneImportFactory->SceneImportOptions->HierarchyType = EFBXSceneOptionsCreateHierarchyType::FBXSOCHT_CreateBlueprint;

            const FAssetToolsModule& assetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
            const auto importedAssets = assetToolsModule.Get().ImportAssetsAutomated(automatedData);

            const auto Blueprints = FindObjects<UBlueprint>(importedAssets);

            for (const auto Blueprint : Blueprints) {
                // BluePrintエディタ開いている必要あり(FbxSceneImporterFactoryの内部で開かれる)
                const auto nodes = Blueprint->SimpleConstructionScript->GetAllNodes();

                for (const auto node : nodes) {
                    if (node->ComponentClass != UStaticMeshComponent::StaticClass())
                        continue;
                    const auto StaticMeshComponent = Cast<UStaticMeshComponent>(node->ComponentTemplate);
                    CityModelInfo.StaticMeshes.Add(StaticMeshComponent->GetStaticMesh());
                }
            }

            TArray<AActor*> ActorsToFind;
            UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), ActorsToFind);
            for (const auto Actor : ActorsToFind) {
                // TODO: 生成されたアクタだけ消したい。。。
                if (Actor->GetName().Find(_TEXT("FbxScene_LOD")) == 0) {
                    Actor->Destroy();
                }
            }

            // FbxSceneImporterFactoryによって開かれるBluePrintエディタを閉じるための措置
            // TODO: 他のアセットエディタは閉じないように変更。現状では全てのアセットエディタを閉じる挙動。
            GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
        }
    }

    FAssetData EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
    UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
    auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
    Cast<APLATEAUCityModelLoader>(Actor)->Metadata = Metadata;
    FPLATEAUCityMapDetails::PlaceMeshes(*Cast<APLATEAUCityModelLoader>(Actor));
}
