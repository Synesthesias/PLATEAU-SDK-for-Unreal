// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUCityMapDetails.h"

#include "SlateOptMacros.h"
#include "Components/StaticMeshComponent.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PLATEAUCityMap.h"
#include "Engine/StaticMesh.h"
#include "PropertyCustomizationHelpers.h"

#include "citygml/citygml.h"
#include "citygml/citymodel.h"
#include "citygml/geometry.h"
#include "plateau/mesh/primary_city_object_types.h"
#include "plateau/udx/gml_file_info.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"

namespace {
    FName GetFeaturePlacementSettingsPropertyName(ECityModelPackage Package) {
        switch (Package) {
        case ECityModelPackage::Building: return GET_MEMBER_NAME_CHECKED(FCityModelPlacementSettings, BuildingPlacementSettings);
        case ECityModelPackage::Road: return GET_MEMBER_NAME_CHECKED(FCityModelPlacementSettings, RoadPlacementSettings);
        default: return GET_MEMBER_NAME_CHECKED(FCityModelPlacementSettings, OtherPlacementSettings);
        }
    }
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FPLATEAUCityMapDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    auto& CityModelCategory = DetailBuilder.EditCategory("CityModel", LOCTEXT("CityModel", "都市モデル"));
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    auto CityMap = Cast<APLATEAUCityMap>(ObjectsBeingCustomized[0].Get());

    const auto MetadataProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityMap, Metadata));
    const auto DetailBuilderPtr = &DetailBuilder;
    CityModelCategory.AddProperty(MetadataProperty)
        .CustomWidget()
        .NameContent()
        [
            SNew(STextBlock).Text(LOCTEXT("CityMapMetadata", "メタデータ"))
        ]
    .ValueContent()
        [
            SNew(SObjectPropertyEntryBox)
            .AllowedClass(UCityMapMetadata::StaticClass())
        .OnObjectChanged_Lambda(
            [this, DetailBuilderPtr, CityMap](const FAssetData& InAssetData) {
                auto* MetadataAsset = Cast<UCityMapMetadata>(InAssetData.GetAsset());
                CityMap->Metadata = MetadataAsset;

                if (DetailBuilderPtr != nullptr)
                    DetailBuilderPtr->ForceRefreshDetails();
            })
        .ObjectPath_Lambda(
            [this, CityMap]() {
                if (CityMap == nullptr)
                    return FString("");
                if (CityMap->Metadata == nullptr)
                    return FString("");
                return CityMap->Metadata->GetPathName();
            })
                //.OnShouldFilterAsset(this, &SDataprepInstanceParentWidget::ShouldFilterAsset)
                //        .ObjectPath(this, &SDataprepInstanceParentWidget::GetDataprepInstanceParent);
        ];


    const auto Metadata = CityMap->Metadata;
    if (Metadata == nullptr)
        return;

    const auto CityModelPlacementSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityMap, CityModelPlacementSettings));

    const auto CityModelInfoArray = Metadata->ImportedCityModelInfoArray;
    for (const auto& CityModelInfo : CityModelInfoArray) {
        const auto FileInfo = GmlFileInfo(TCHAR_TO_UTF8(*CityModelInfo.GmlFilePath));
        const ECityModelPackage Package = FCityModelPlacementSettings::GetPackage(UTF8_TO_TCHAR(FileInfo.getFeatureType().c_str()));
        // TODO: Refactor
        if (!FeaturePlacementRows.Find(Package)) {
            FeaturePlacementRows.Add(Package, FFeaturePlacementRow(Package));

            const auto PropertyName = GetFeaturePlacementSettingsPropertyName(Package);
            const auto FeaturePlacementSettingsProperty = CityModelPlacementSettingsProperty->GetChildHandle(PropertyName);
            FeaturePlacementRows[Package].AddToCategory(CityModelCategory, FeaturePlacementSettingsProperty);
        }
    }

    CityModelCategory.AddCustomRow(FText::FromString("PlaceCityModel"))
        .NameContent()
        [
            SNew(STextBlock).Text(LOCTEXT("PlaceCityModel", "都市モデルを配置"))
        ]
    .ValueContent()
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &FPLATEAUCityMapDetails::OnClickPlace)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Place Button", "配置"))
        ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION







FReply FPLATEAUCityMapDetails::OnClickPlace() {
    for (auto Object : ObjectsBeingCustomized) {
        auto* Actor = Cast<APLATEAUCityMap>(Object.Get());
        if (Actor != nullptr)
            PlaceMeshes(*Actor);
    }
    return FReply::Handled();
}

void FPLATEAUCityMapDetails::PlaceMeshes(APLATEAUCityMap& Actor) {
    if (Actor.Metadata == nullptr) {
        return;
    }

    USceneComponent* ActorRootComponent = NewObject<USceneComponent>(&Actor,
        USceneComponent::GetDefaultSceneRootVariableName());

    check(ActorRootComponent != nullptr);
    ActorRootComponent->Mobility = EComponentMobility::Static;
    ActorRootComponent->bVisualizeComponent = true;
    Actor.SetRootComponent(ActorRootComponent);
    Actor.AddInstanceComponent(ActorRootComponent);
    ActorRootComponent->RegisterComponent();
    Actor.SetFlags(RF_Transactional);
    ActorRootComponent->SetFlags(RF_Transactional);

    for (const auto CityModelInfo : Actor.Metadata->ImportedCityModelInfoArray) {
        PlaceCityModel(Actor, *ActorRootComponent, CityModelInfo, 2, true);
    }
    GEngine->BroadcastLevelActorListChanged();
}

void FPLATEAUCityMapDetails::PlaceCityModel(APLATEAUCityMap& Actor, USceneComponent& RootComponent, const FPLATEAUImportedCityModelInfo& CityModelInfo, int TargetLOD, bool bShouldPlaceLowerLODs) {
    // GML読み込み
    const auto GmlPath = FPaths::ProjectContentDir().Append("/PLATEAU/").Append(CityModelInfo.GmlFilePath);
    citygml::ParserParams params;
    params.tesselate = false;
    const auto CityModel = citygml::load(TCHAR_TO_UTF8(*GmlPath), params);
    if (CityModel == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load gml: %s"), *GmlPath);
        return;
    }

    // 親コンポーネント作成
    const auto RootComponentName = FName(FPaths::GetBaseFilename(GmlPath));
    const auto CityModelRootComponent = PlaceEmptyComponent(Actor, RootComponent, RootComponentName);

    // ハッシュテーブル作成
    TMap<FString, UStaticMesh*> StaticMeshMap;
    for (const auto StaticMesh : CityModelInfo.StaticMeshes) {
        const auto Key = StaticMesh->GetName();
        StaticMeshMap.Add(Key, StaticMesh);
    }

    // インスタンス生成
    const auto PrimaryTypeMask = PrimaryCityObjectTypes::getPrimaryTypeMask();
    const auto PrimaryCityObjects = CityModel->getAllCityObjectsOfType(PrimaryTypeMask);
    int DefaultTargetLOD = TargetLOD;
    for (const auto& CityObject : *PrimaryCityObjects) {
        // 配置可能なジオメトリのLODを探す
        while (TargetLOD >= 0) {
            bool bHasTargetLOD = false;
            for (unsigned i = 0; i < CityObject->getGeometriesCount(); i++) {
                if (CityObject->getGeometry(i).getLOD() == TargetLOD) {
                    bHasTargetLOD = true;
                    break;
                }
            }
            if (bHasTargetLOD)
                break;
            TargetLOD--;
        }

        if (bShouldPlaceLowerLODs && TargetLOD != DefaultTargetLOD)
            continue;
        if (TargetLOD < 0)
            continue;

        USceneComponent* PrimaryGeometryComponent;
        const auto PrimaryGeometryComponentName = GetMeshName(TargetLOD, UTF8_TO_TCHAR(CityObject->getId().c_str()));
        if (const auto StaticMeshPtr = StaticMeshMap.Find(PrimaryGeometryComponentName)) {
            PrimaryGeometryComponent = PlaceStaticMesh(Actor, *CityModelRootComponent, *StaticMeshPtr);
        } else {
            PrimaryGeometryComponent = PlaceEmptyComponent(Actor, *CityModelRootComponent, FName(PrimaryGeometryComponentName));
        }

        // LOD2以上の細分化されたジオメトリの配置
        if (TargetLOD < 2)
            continue;
        for (unsigned i = 0; i < CityObject->getChildCityObjectsCount(); ++i) {
            const auto& ChildCityObject = CityObject->getChildCityObject(i);
            if (PrimaryCityObjectTypes::isPrimary(ChildCityObject.getType()))
                continue;

            if (const auto StaticMeshPtr = StaticMeshMap.Find(GetMeshName(TargetLOD, UTF8_TO_TCHAR(ChildCityObject.getId().c_str()))))
                PlaceStaticMesh(Actor, *PrimaryGeometryComponent, *StaticMeshPtr);
        }
    }
}

UStaticMeshComponent* FPLATEAUCityMapDetails::PlaceStaticMesh(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, UStaticMesh* StaticMesh) {
    const auto Component = NewObject<UStaticMeshComponent>(&Actor, NAME_None);
    Component->SetStaticMesh(StaticMesh);
    Component->DepthPriorityGroup = SDPG_World;
    // TODO: SetStaticMeshComponentOverrideMaterial(StaticMeshComponent, NodeInfo);
    FString NewUniqueName = StaticMesh->GetName();
    if (!Component->Rename(*NewUniqueName, nullptr, REN_Test)) {
        NewUniqueName = MakeUniqueObjectName(&Actor, USceneComponent::StaticClass(), FName(StaticMesh->GetName())).ToString();
    }
    Component->Rename(*NewUniqueName, nullptr, REN_DontCreateRedirectors);
    Actor.AddInstanceComponent(Component);
    Component->RegisterComponent();
    Component->AttachToComponent(&ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
    Component->PostEditChange();
    return Component;
}

USceneComponent* FPLATEAUCityMapDetails::PlaceEmptyComponent(APLATEAUCityMap& Actor, USceneComponent& ParentComponent, const FName& Name) {
    USceneComponent* SceneComponent = NewObject<USceneComponent>(&Actor, Name);
    Actor.AddInstanceComponent(SceneComponent);
    SceneComponent->RegisterComponent();
    SceneComponent->AttachToComponent(&ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
    return SceneComponent;
}


FString FPLATEAUCityMapDetails::GetMeshName(int LOD, FString CityObjectID) {
    return FString("LOD") + FString::FromInt(LOD) + FString("_") + CityObjectID;
}


#undef LOCTEXT_NAMESPACE
