// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUAsyncLoadedFeatureInfoPanel.h"
#include "PLATEAUExtentEditorVPClient.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "PLATEAUFeatureInfoDisplay.h"
#include "Components/StaticMeshComponent.h"
#include "StaticMeshAttributes.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"

using namespace plateau::dataset;

namespace {
    UStaticMeshComponent* CreatePanelMeshComponent(UMaterialInstanceDynamic* Material) {
        const FName MeshName = MakeUniqueObjectName(
            GetTransientPackage(),
            UStaticMeshComponent::StaticClass(),
            TEXT("Panel"));

        UStaticMeshComponent* PanelComponent = NewObject<UStaticMeshComponent>(
            GetTransientPackage(),
            MeshName, RF_Transient);
        PanelComponent->SetMaterial(0, Material);
        const auto StaticMeshName = TEXT("/Engine/BasicShapes/Plane");
        const auto Mesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, StaticMeshName));
        Mesh->AddMaterial(Material);
        PanelComponent->SetStaticMesh(Mesh);
        PanelComponent->SetMobility(EComponentMobility::Static);
        PanelComponent->CastShadow = false;
        PanelComponent->SetHiddenInGame(true);

        return PanelComponent;
    }
}

FPLATEAUAsyncLoadedFeatureInfoPanel::FPLATEAUAsyncLoadedFeatureInfoPanel(
    const TWeakPtr<FPLATEAUFeatureInfoDisplay> Owner,
    const TWeakPtr<FPLATEAUExtentEditorViewportClient> ViewportClient)
    : Owner(Owner)
    , ViewportClient(ViewportClient)
    , bFullyLoaded(false)
    , Visibility(EPLATEAUFeatureInfoVisibility::Hidden)
    , BackPanelComponent(nullptr) {}

EPLATEAUFeatureInfoVisibility FPLATEAUAsyncLoadedFeatureInfoPanel::GetVisibility() const {
    return Visibility;
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::SetVisibility(const EPLATEAUFeatureInfoVisibility Value) {
    if (Visibility == Value)
        return;

    Visibility = Value;
    ApplyVisibility();
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::ApplyVisibility() const {
    TArray<USceneComponent*> Components;
    Components.Append(IconComponents);
    Components.Append(DetailedIconComponents);
    Components.Add(BackPanelComponent);

    for (const auto Component : IconComponents) {
        if (Component == nullptr)
            continue;

        Component->SetVisibility(Visibility == EPLATEAUFeatureInfoVisibility::Visible);
    }

    for (const auto Component : DetailedIconComponents) {
        if (Component == nullptr)
            continue;

        Component->SetVisibility(Visibility == EPLATEAUFeatureInfoVisibility::Detailed);
    }

    BackPanelComponent->SetVisibility(Visibility != EPLATEAUFeatureInfoVisibility::Hidden);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadAsync(const FPLATEAUFeatureInfoPanelInput& Input, const FBox& InBox) {
    Box = InBox;

    GetMaxLodTask = Async(EAsyncExecution::Thread,
        [Input]() mutable {
            TMap<PredefinedCityModelPackage, int> MaxLods;

            for (const auto& Entry : Input) {
                if (Entry.Value->empty())
                    continue;

                int MaxLod = 0;
                for (auto& GmlFile : *Entry.Value) {
                    // GMLファイル内を検索して最大LODを取得
                    MaxLod = FMath::Max(GmlFile.getMaxLod(), MaxLod);
                }

                MaxLods.Add(Entry.Key, MaxLod);
            }

            return MaxLods;
        });
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::Tick() {
    constexpr float PanelScaleMultiplier = 2.0f;

    if (bFullyLoaded)
        return;

    if (!GetMaxLodTask.IsReady())
        return;

    const auto Input = GetMaxLodTask.Get();

    if (Input.IsEmpty())
        return;

    const auto OwnerStrongPtr = Owner.Pin();

    // パネルコンポーネント生成
    for (const auto& Package : FPLATEAUFeatureInfoDisplay::GetDisplayedPackages()) {
        FPLATEAUFeatureInfoMaterialKey Key = {};
        Key.bDetailed = false;
        Key.Package = Package;

        if (!Input.Find(Package)) {
            Key.bGrayout = true;
            Key.Lod = 0;
        } else {
            Key.bGrayout = false;
            Key.Lod = Input[Package];
        }

        const auto PanelMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        IconComponents.Add(CreatePanelMeshComponent(PanelMaterial));

        Key.bDetailed = true;
        const auto DetailedPanelMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        DetailedIconComponents.Add(CreatePanelMeshComponent(DetailedPanelMaterial));
    }
    BackPanelComponent = CreatePanelMeshComponent(OwnerStrongPtr->GetBackPanelMaterial());

    const auto PreviewScene = ViewportClient.Pin()->GetPreviewScene();

    if (PreviewScene == nullptr)
        return;

    // シーンにパネルを配置
    for (int i = 0; i < 4; ++i) {
        const auto IconTransform = FTransform(FRotator(0, 0, 0),
            Box.GetCenter() + FVector::UpVector * 2 + FVector((80 * PanelScaleMultiplier * i) - 120 * PanelScaleMultiplier, 0, 0),
            FVector3d(0.8, 0.8, 0.8) * PanelScaleMultiplier);

        PreviewScene->AddComponent(IconComponents[i], IconTransform);
        PreviewScene->AddComponent(DetailedIconComponents[i], IconTransform);
    }
    const auto BackPanelTransform = FTransform(FRotator(0, 0, 0),
        Box.GetCenter() + FVector::UpVector,
        FVector3d(4, 1.3, 1) * PanelScaleMultiplier);

    PreviewScene->AddComponent(BackPanelComponent, BackPanelTransform);

    ApplyVisibility();

    bFullyLoaded = true;
}

