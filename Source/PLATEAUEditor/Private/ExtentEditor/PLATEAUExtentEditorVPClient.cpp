// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUExtentEditorVPClient.h"

#include "EditorModeManager.h"
#include "EngineGlobals.h"
#include "RawIndexBuffer.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "Engine/StaticMesh.h"
#include "Editor.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Engine/Canvas.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
//#include "IStaticMeshEditor.h"
#include "UnrealEngine.h"

#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"
#include "SPLATEAUExtentEditorViewport.h"

#include "Interfaces/IAnalyticsProvider.h"
#include "EngineAnalytics.h"
#include "AI/Navigation/NavCollisionBase.h"
#include "PhysicsEngine/BodySetup.h"

#include "Engine/AssetUserData.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "AssetViewerSettings.h"
#include "UnrealWidget.h"
#include "EditorViewportClient.h"

#define LOCTEXT_NAMESPACE "FPLATEAUExtentEditorViewportClient"

FPLATEAUExtentEditorViewportClient::FPLATEAUExtentEditorViewportClient(
    //TWeakPtr<IStaticMeshEditor> InStaticMeshEditor,
    const TSharedRef<SPLATEAUExtentEditorViewport>& InPLATEAUExtentEditorViewport,
    const TSharedRef<FAdvancedPreviewScene>& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene.Get(), StaticCastSharedRef<SEditorViewport>(InPLATEAUExtentEditorViewport)) {
    InPreviewScene->SetFloorVisibility(false);
}

FPLATEAUExtentEditorViewportClient::~FPLATEAUExtentEditorViewportClient() {
    UAssetViewerSettings::Get()->OnAssetViewerSettingsChanged().RemoveAll(this);
}

void FPLATEAUExtentEditorViewportClient::InitCamera() {
    ToggleOrbitCamera(false);
    SetCameraSetup(
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        FVector(0.0f, 0, 10000.0f),
        FVector::Zero(),
        FVector(0, 0, 10000),
        FRotator(-90, 0, 0)
    );
}


#undef LOCTEXT_NAMESPACE