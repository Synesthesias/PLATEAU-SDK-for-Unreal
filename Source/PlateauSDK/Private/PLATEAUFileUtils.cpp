#include <PLATEAUFileUtils.h>

#include "AssetSelection.h"
#include "AutomatedAssetImportData.h"
#include "Factories/FbxSceneImportFactory.h"
#include "UObject/UObjectIterator.h"

#include "AssetTools/Private/AssetTools.h"
#include "AssetToolsModule.h"
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
    const TObject* FindObject(const TArray<UObject*> objects) {
        for (const UObject* object : objects) {
            if (object->IsA(TObject::StaticClass())) {
                return Cast<TObject>(object);
            }
        }
        return nullptr;
    }

    /// <summary>
    /// CityMapアクタをレベル内に生成します。
    /// </summary>
    /// <param name="blueprints">CityMapに含めるブループリント</param>
    void CreateCityMapActor(const TArray<const UBlueprint*> blueprints) {
        UBlueprint* NewBluePrintActor = nullptr;
        AActor* RootActorContainer = nullptr;
        USceneComponent* ActorRootComponent = nullptr;
        TMap<uint64, USceneComponent*> NewSceneComponentNameMap;

        auto* Factory = GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
        const auto EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
        //This is a group create an empty actor that just have a transform
        auto* EmptyActorAsset = EmptyActorAssetData.GetAsset();
        //Place an empty actor
        RootActorContainer = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
        check(RootActorContainer != nullptr);
        ActorRootComponent = NewObject<USceneComponent>(RootActorContainer, USceneComponent::GetDefaultSceneRootVariableName());
        check(ActorRootComponent != nullptr);
        ActorRootComponent->Mobility = EComponentMobility::Static;
        ActorRootComponent->bVisualizeComponent = true;
        RootActorContainer->SetRootComponent(ActorRootComponent);
        RootActorContainer->AddInstanceComponent(ActorRootComponent);
        ActorRootComponent->RegisterComponent();
        RootActorContainer->SetActorLabel(TEXT("CityMap"));
        RootActorContainer->SetFlags(RF_Transactional);
        ActorRootComponent->SetFlags(RF_Transactional);
        
        for (const auto blueprint : blueprints) {
            const auto nodes = blueprint->SimpleConstructionScript->GetAllNodes();
            for (const auto node : nodes) {
                USceneComponent* SceneComponent = NewObject<USceneComponent>(RootActorContainer, node->ComponentClass, node->GetVariableName(), RF_NoFlags, node->ComponentTemplate);

                //Add the component to the owner actor and register it
                RootActorContainer->AddInstanceComponent(SceneComponent);
                SceneComponent->RegisterComponent();
            }
        }
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
            metadataFactory->CreateOrOverwriteAsset(metadataFactory->GetSupportedClass(), assetPackage, FName(*assetName), flags);
            assetPackage->SetDirtyFlag(true);

            TArray<const UBlueprint*> blueprints;
            blueprints.Add(blueprint);
            CreateCityMapActor(blueprints);
        }

        // FbxSceneImporterFactoryによって開かれるBluePrintエディタを閉じるための措置
        // TODO: 他のアセットエディタは閉じないように変更。現状では全てのアセットエディタを閉じる挙動。
        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllAssetEditors();
    }
}
