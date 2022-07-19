// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUCityMapDetails.h"

#include "SlateOptMacros.h"
#include "Components/StaticMeshComponent.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SSlider.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "PLATEAUCityMap.h"
#include "Engine/StaticMesh.h"

#include "citygml/citygml.h"
#include "citygml/citymodel.h"
#include "citygml/geometry.h"
#include "plateau/mesh/primary_city_object_types.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"

TSharedRef<IDetailCustomization> FPLATEAUCityMapDetails::MakeInstance() {
    return MakeShareable(new FPLATEAUCityMapDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void FPLATEAUCityMapDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    // Create a category so this is displayed early in the properties
    auto& cityModelCategory = DetailBuilder.EditCategory("CityModel", LOCTEXT("CityModel", "都市モデル"));

    const auto MetadataProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityMap, Metadata));
    const auto BuildingPlacementSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityMap, BuildingPlacementSettings));
    BuildingPlacementModeProperty = BuildingPlacementSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBuildingPlacementSettings, FeaturePlacementMode));

    auto& BuildingPlacementSettingsRow = cityModelCategory.AddProperty(BuildingPlacementSettingsProperty);
    BuildingPlacementSettingsRow.DisplayName(LOCTEXT("Building", "建築物"));
    BuildingPlacementSettingsRow.CustomWidget()
        .NameContent()[BuildingPlacementSettingsProperty->CreatePropertyNameWidget()]
        .ValueContent()
        [SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [SNew(SComboButton)
        .OnGetMenuContent(this, &FPLATEAUCityMapDetails::OnGetFeaturePlacementModeComboContent)
        .ContentPadding(0.0f)
        .ButtonStyle(FEditorStyle::Get(), "ToggleButton")
        .ForegroundColor(FSlateColor::UseForeground())
        .VAlign(VAlign_Center)
        .ButtonContent()
        [SNew(STextBlock).Text_Lambda(
            [this]() {
                uint8 out;
                BuildingPlacementModeProperty->GetValue(out);
                return FeaturePlacementTexts()[static_cast<EFeaturePlacementMode>(out)];
            })
        ]]
    + SVerticalBox::Slot()
        [SNew(SHorizontalBox)
        .Visibility_Lambda(
            [this]() {
                uint8 out;
                BuildingPlacementModeProperty->GetValue(out);
                return static_cast<EFeaturePlacementMode>(out) != EFeaturePlacementMode::DontPlace ? EVisibility::Visible : EVisibility::Hidden;
            }
        )
        + SHorizontalBox::Slot()
                [SNew(STextBlock).Text(LOCTEXT("LOD", "LOD"))]
            + SHorizontalBox::Slot()
                [SNew(SSlider)
                .MaxValue(3)
                .MinValue(0)
                .StepSize(1)
                ]]];


    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    cityModelCategory.AddCustomRow(FText::FromString("PlaceCityModel"))
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

TSharedRef<SWidget> FPLATEAUCityMapDetails::OnGetFeaturePlacementModeComboContent() const {
    // Fill the combo menu with presets of common screen resolutions
    FMenuBuilder MenuBuilder(true, NULL);
    auto Items = FeaturePlacementTexts();
    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
        FText ItemText = ItemIter->Value;
        FUIAction ItemAction(FExecuteAction::CreateSP(const_cast<FPLATEAUCityMapDetails*>(this), &FPLATEAUCityMapDetails::CommitPlacementMode, ItemIter->Key));
        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
    }

    return MenuBuilder.MakeWidget();
}

void FPLATEAUCityMapDetails::CommitPlacementMode(EFeaturePlacementMode NewMode) {
    BuildingPlacementModeProperty->SetValue(static_cast<uint8>(NewMode));
}







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

    for (const auto Entry : Actor.Metadata->StaticMeshes) {
        PlaceCityModel(Actor, *ActorRootComponent, Entry.Key, 2, true);
    }
    GEngine->BroadcastLevelActorListChanged();
}

void FPLATEAUCityMapDetails::PlaceCityModel(APLATEAUCityMap& Actor, USceneComponent& RootComponent, int SourceGmlIndex, int TargetLOD, bool bShouldPlaceLowerLODs) {
    // GML読み込み
    const auto GmlPath = FPaths::ProjectContentDir().Append("/PLATEAU/").Append(Actor.Metadata->SourceGmlFiles[SourceGmlIndex]);
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
    for (const auto StaticMesh : Actor.Metadata->StaticMeshes[SourceGmlIndex].Value) {
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
