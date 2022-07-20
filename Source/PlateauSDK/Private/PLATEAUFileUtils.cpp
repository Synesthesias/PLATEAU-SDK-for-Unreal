#include <PLATEAUFileUtils.h>

#include "AssetSelection.h"
#include "AutomatedAssetImportData.h"
#include "Factories/FbxSceneImportFactory.h"
#include "UObject/UObjectIterator.h"

#include "AssetTools/Private/AssetTools.h"
#include "AssetToolsModule.h"
#include "CityMapMetadata.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Factories/CityMapMetadataFactory.h"
#include "Factories/FbxSceneImportOptions.h"

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
}

void PLATEAUFileUtils::ImportFbx(const TArray<FString>& files, const FString& destinationPath) {
    auto* sceneImportFactory = FindFactory<UFbxSceneImportFactory>();

    if (sceneImportFactory) {
        auto* automatedData = NewObject<UAutomatedAssetImportData>();
        automatedData->DestinationPath = destinationPath;
        automatedData->Filenames = files;
        automatedData->Factory = sceneImportFactory;
        sceneImportFactory->SceneImportOptions->bCreateContentFolderHierarchy = 1;
        sceneImportFactory->SceneImportOptions->HierarchyType = EFBXSceneOptionsCreateHierarchyType::FBXSOCHT_CreateBlueprint;

        const FAssetToolsModule& assetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
        const auto importedAssets = assetToolsModule.Get().ImportAssetsAutomated(automatedData);

        auto* blueprint = FindObject<UBlueprint>(importedAssets);

        const auto* metadataFactory = FindFactory<UCityMapMetadataFactory>();
        if (metadataFactory != nullptr && blueprint != nullptr) {
            const FString assetName = TEXT("CityMapMetadata");
            const FString packageName = destinationPath + TEXT("/") + assetName;
            UPackage* assetPackage = CreatePackage(*packageName);
            const EObjectFlags flags = RF_Public | RF_Standalone;

            // メタデータアセット作成
            auto* metadata =
                Cast<UCityMapMetadata>(metadataFactory->CreateOrOverwriteAsset(metadataFactory->GetSupportedClass(), assetPackage, FName(*assetName), flags));

            // BluePrintエディタ開いている必要あり
            const auto nodes = blueprint->SimpleConstructionScript->GetAllNodes();

            //metadata->ImportedCityModelInfoArray.SetNum(files.Num());

            for (const auto node : nodes) {
                if (node->ComponentClass != UStaticMeshComponent::StaticClass())
                    continue;
                auto staticMeshComponent = Cast<UStaticMeshComponent>(node->ComponentTemplate);
                //auto& container = metadata->StaticMeshes.FindOrAdd(0);
                //container.Value.Add(staticMeshComponent->GetStaticMesh());
            }

            assetPackage->SetDirtyFlag(true);

            //CreateCityMapActor(*metadata);
        }

        // FbxSceneImporterFactoryによって開かれるBluePrintエディタを閉じるための措置
        // TODO: 他のアセットエディタは閉じないように変更。現状では全てのアセットエディタを閉じる挙動。
        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
    }
}
