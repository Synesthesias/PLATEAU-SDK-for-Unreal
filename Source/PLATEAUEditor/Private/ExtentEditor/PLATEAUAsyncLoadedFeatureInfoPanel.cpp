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
    constexpr float PanelScaleMultiplier = 2.0f;

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

    FTransform CalculateIconTransform(const FBox& Box, const int IconIndex, const int IconCount) {
        FTransform Transform{};
        Transform.SetIdentity();

        const auto Center = Box.GetCenter();
        FVector Offset{
            // アイコン1つの横幅は100fなため、-100f * {アイコン数}/2が左端のx座標になる。
            // 中心座標を指定するため50f足している。
            100.0f * IconIndex - 100.0f * IconCount / 2.0f + 50.0f,
            0.0f,
            // バックパネルより手前に表示
            2.0f
        };
        Offset *= PanelScaleMultiplier;
        Transform.SetTranslation(Center + Offset);

        Transform.SetScale3D(PanelScaleMultiplier * FVector3d::One());

        return Transform;
    }

    FTransform CalculateBackPanelTransform(const FBox& Box, const int IconCount) {
        FTransform Transform{};
        Transform.SetIdentity();

        auto Translation = Box.GetCenter();
        // ベースマップより手前に表示
        Translation.Z += 1.0f;
        Transform.SetTranslation(Translation);

        Transform.SetScale3D(FVector3d(IconCount + 0.4, 1.5, 1.0) * PanelScaleMultiplier);

        return Transform;
    }
}

FPLATEAUAsyncLoadedFeatureInfoPanel::FPLATEAUAsyncLoadedFeatureInfoPanel(
    const TWeakPtr<FPLATEAUFeatureInfoDisplay> Owner,
    const TWeakPtr<FPLATEAUExtentEditorViewportClient> ViewportClient)
    : Owner(Owner)
    , ViewportClient(ViewportClient)
    , Status(EPLATEAUFeatureInfoPanelStatus::Idle)
    , Visibility(EPLATEAUFeatureInfoVisibility::Hidden)
    , BackPanelComponent(nullptr) {}

EPLATEAUFeatureInfoPanelStatus FPLATEAUAsyncLoadedFeatureInfoPanel::GetStatus() const {
    return Status;
}

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

    if (BackPanelComponent == nullptr)
        return;

    BackPanelComponent->SetVisibility(Visibility != EPLATEAUFeatureInfoVisibility::Hidden);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::CreatePanelComponents(const TMap<PredefinedCityModelPackage, int>& MaxLods)
{
    if (MaxLods.IsEmpty())
        return;

    const auto OwnerStrongPtr = Owner.Pin();

    // パネルコンポーネント生成
    for (const auto& Package : FPLATEAUFeatureInfoDisplay::GetDisplayedPackages()) {
        FPLATEAUFeatureInfoMaterialKey Key = {};
        Key.bDetailed = false;
        Key.Package = Package;

        if (!MaxLods.Find(Package))
            continue;

        Key.Lod = MaxLods[Package];

        const auto IconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        const auto IconComponent = CreatePanelMeshComponent(IconMaterial);
        // 常にバックパネルより後に描画
        IconComponent->SetTranslucentSortPriority(-1);
        IconComponents.Add(IconComponent);

        Key.bDetailed = true;
        const auto DetailedIconMaterial = OwnerStrongPtr->GetFeatureInfoIconMaterial(Key);
        const auto DetailedIconComponent = CreatePanelMeshComponent(DetailedIconMaterial);
        // 常にバックパネルより後に描画
        DetailedIconComponent->SetTranslucentSortPriority(-1);
        DetailedIconComponents.Add(DetailedIconComponent);
    }
    BackPanelComponent = CreatePanelMeshComponent(OwnerStrongPtr->GetBackPanelMaterial());

    const auto PreviewScene = ViewportClient.Pin()->GetPreviewScene();

    if (PreviewScene == nullptr)
        return;

    // シーンにパネルを配置
    check(IconComponents.Num() == DetailedIconComponents.Num());
    for (int i = 0; i < IconComponents.Num(); ++i) {
        const auto Transform = CalculateIconTransform(Box, i, IconComponents.Num());
        PreviewScene->AddComponent(IconComponents[i], Transform);
        PreviewScene->AddComponent(DetailedIconComponents[i], Transform);
    }

    const auto BackPanelTransform = CalculateBackPanelTransform(Box, IconComponents.Num());
    PreviewScene->AddComponent(BackPanelComponent, BackPanelTransform);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::LoadMaxLodAsync(const FPLATEAUFeatureInfoPanelInput& Input, const FBox& InBox) {
    Box = InBox;
    Status = EPLATEAUFeatureInfoPanelStatus::Loading;

    GetMaxLodTask = UE::Tasks::Launch(TEXT("GetMaxLODTask"),
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
        }, LowLevelTasks::ETaskPriority::BackgroundHigh);
}

void FPLATEAUAsyncLoadedFeatureInfoPanel::Tick() {
    if (Status == EPLATEAUFeatureInfoPanelStatus::FullyLoaded)
        return;

    if (!GetMaxLodTask.IsCompleted())
        return;

    CreatePanelComponents(GetMaxLodTask.GetResult());

    ApplyVisibility();

    Status = EPLATEAUFeatureInfoPanelStatus::FullyLoaded;
}

