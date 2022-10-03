// Copyright Epic Games, Inc. All Rights Reserved.

#include "PLATEAUEditor.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUWindow.h"
#include "CityModelLoaderDetails/PLATEAUCityModelLoaderDetails.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

IPLATEAUEditorModule& IPLATEAUEditorModule::Get() {
    return FModuleManager::LoadModuleChecked<IPLATEAUEditorModule>("PLATEAUEditor");
}

bool IPLATEAUEditorModule::IsAvailable() {
    return FModuleManager::Get().IsModuleLoaded("PLATEAUEditor");
}

class FPLATEAUEditorModule : public IPLATEAUEditorModule {
public:
    virtual void StartupModule() override {
        ExtentEditor = MakeShareable(new FPLATEAUExtentEditor());

        Window.Startup();

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(
            APLATEAUCityModelLoader::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityModelLoaderDetails::MakeInstance)
        );

        RegisterExtentEditorTabSpawner();
    }

    virtual void ShutdownModule() override {
        Window.Shutdown();

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomPropertyTypeLayout(
            APLATEAUCityModelLoader::StaticClass()->GetFName()
        );

        const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
        ExtentEditor->UnregisterTabSpawner(GlobalTabManager);
    }

    virtual TSharedRef<FPLATEAUExtentEditor> GetExtentEditor() override {
        return ExtentEditor.ToSharedRef();
    }

private:
    FPLATEAUWindow Window;
    TSharedPtr<FPLATEAUExtentEditor> ExtentEditor;

    void RegisterExtentEditorTabSpawner() const {
        const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
        ExtentEditor->RegisterTabSpawner(GlobalTabManager);
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAUEditorModule, PLATEAUEditor)
