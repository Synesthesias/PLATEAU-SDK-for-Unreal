#include <PLATEAUFileUtils.h>

#include "AutomatedAssetImportData.h"
#include "Factories/FbxSceneImportFactory.h"
#include "UObject/UObjectIterator.h"

#include "AssetTools/Private/AssetTools.h"
#include "AssetToolsModule.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Factories/CityMapMetadataFactory.h"
#include "Factories/FbxSceneImportData.h"
#include "Factories/FbxSceneImportOptions.h"

namespace {
    template <class TFactory>
    TFactory* FindFactory() {
        for (UClass* object : TObjectRange<UClass>()) {
            if (object->IsChildOf<TFactory>()) {
                return object->GetDefaultObject<TFactory>();
            }
        }
        return nullptr;
    }

    template <class TObject>
    const TObject* FindObject(const TArray<UObject*> objects) {
        for (const UObject* object : objects) {
            if (object->IsA(TObject::StaticClass())) {
                return Cast<TObject>(object);
            }
        }
        return nullptr;
    }
}

void PLATEAUFileUtils::ImportFbx(const TArray<FString>& files, FString destinationPath) {
    // Mimicking FileHelper.cpp
    // Find SceneImportFactory from given factories.
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

        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();

        auto* blueprint = FindObject<UBlueprint>(importedAssets);

        const auto nodes = blueprint->SimpleConstructionScript->GetAllNodes();
        //for (const auto node : nodes) {
        //    node->GetVariableName()
        //}

        const auto* metadataFactory = FindFactory<UCityMapMetadataFactory>();
        if (metadataFactory != nullptr && blueprint != nullptr) {
            const FString assetName = TEXT("CityMapMetadata");
            const FString packageName = destinationPath + TEXT("/") + assetName;
            UPackage* assetPackage = CreatePackage(*packageName);
            const EObjectFlags flags = RF_Public | RF_Standalone;
            metadataFactory->CreateOrOverwriteAsset(metadataFactory->GetSupportedClass(), assetPackage, FName(*assetName), flags);
            assetPackage->SetDirtyFlag(true);
        }
    }
}
