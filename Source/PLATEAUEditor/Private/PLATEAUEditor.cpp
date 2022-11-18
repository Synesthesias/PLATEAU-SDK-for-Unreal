// Copyright Epic Games, Inc. All Rights Reserved.

#include "PLATEAUEditor.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUInstancedCityModelDetails.h"
#include "PLATEAUWindow.h"
#include "CityModelLoaderDetails/PLATEAUCityModelLoaderDetails.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

#include "Styling/ISlateStyle.h"

IPLATEAUEditorModule& IPLATEAUEditorModule::Get() {
    return FModuleManager::LoadModuleChecked<IPLATEAUEditorModule>("PLATEAUEditor");
}

bool IPLATEAUEditorModule::IsAvailable() {
    return FModuleManager::Get().IsModuleLoaded("PLATEAUEditor");
}

class FPLATEAUEditorModule : public IPLATEAUEditorModule {
public:
    virtual void StartupModule() override {
        Style = MakeShareable(new FPLATEAUEditorStyle());
        Window = MakeShareable(new FPLATEAUWindow(Style.ToSharedRef()));
        ExtentEditor = MakeShareable(new FPLATEAUExtentEditor());

        Window->Startup();

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.RegisterCustomClassLayout(
            APLATEAUCityModelLoader::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityModelLoaderDetails::MakeInstance)
        );
        PropertyModule.RegisterCustomClassLayout(
            APLATEAUInstancedCityModel::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUInstancedCityModelDetails::MakeInstance)
        );

        RegisterExtentEditorTabSpawner();
    }

    virtual void ShutdownModule() override {
        Window->Shutdown();

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
    TSharedPtr<FPLATEAUWindow> Window;
    TSharedPtr<FPLATEAUExtentEditor> ExtentEditor;
    TSharedPtr<FPLATEAUEditorStyle> Style;

    void RegisterExtentEditorTabSpawner() const {
        const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
        ExtentEditor->RegisterTabSpawner(GlobalTabManager);
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAUEditorModule, PLATEAUEditor)
