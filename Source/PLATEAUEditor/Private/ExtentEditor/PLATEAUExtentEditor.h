// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Misc/NotifyHook.h"
#include "EditorUndoClient.h"
#include "ISocketManager.h"
#include "TickableEditorObject.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewSceneModule.h"
#include "AssetEditorViewportLayout.h"

class FEditorViewportClient;
class SDockTab;
class FViewportTabContent;

class FPLATEAUExtentEditor : public TSharedFromThis<FPLATEAUExtentEditor> {
public:
    FPLATEAUExtentEditor();
    ~FPLATEAUExtentEditor();

    static const FName TabId;

    void RegisterTabSpawner(const TSharedRef<class FTabManager>& TabManager);
    void UnregisterTabSpawner(const TSharedRef<class FTabManager>& TabManager);

    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args);
    const FString& GetSourcePath() const;
    void SetSourcePath(const FString& Path);

private:
    FString SourcePath;

    FAdvancedPreviewSceneModule::FOnPreviewSceneChanged OnPreviewSceneChangedDelegate;
};
