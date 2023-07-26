// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditor.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"
#include "PLATEAUCityObjectGroupDetails.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUInstancedCityModelDetails.h"
#include "PLATEAUWindow.h"
#include "CityModelLoaderDetails/PLATEAUCityModelLoaderDetails.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Settings/EditorLoadingSavingSettings.h"

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

        FAutoReimportWildcard WildcardToInject1;
        WildcardToInject1.Wildcard = TEXT("PLATEAU/Datasets/*");
        WildcardToInject1.bInclude = false;

        FAutoReimportWildcard WildcardToInject2;
        WildcardToInject2.Wildcard = TEXT("PLATEAU/Basemap/*");
        WildcardToInject2.bInclude = false;

        auto Default = GetMutableDefault<UEditorLoadingSavingSettings>();
        bool HasChanged = false;
        for (auto& Setting : Default->AutoReimportDirectorySettings) {
            bool Found1 = false;
            bool Found2 = false;
            for (const auto& Wildcard : Setting.Wildcards) {
                if (Wildcard.Wildcard == WildcardToInject1.Wildcard) {
                    Found1 = true;
                }
                else if (Wildcard.Wildcard == WildcardToInject2.Wildcard) {
                    Found2 = true;
                }
            }
            if (!Found1) {
                Setting.Wildcards.Add(WildcardToInject1);
                HasChanged = true;
                Found1 = false;
            }
            if (!Found2) {
                Setting.Wildcards.Add(WildcardToInject2);
                HasChanged = true;
                Found2 = false;
            }
        }
        if (HasChanged) {
            Default->PostEditChange();
        }

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
        PropertyModule.RegisterCustomClassLayout(
            UPLATEAUCityObjectGroup::StaticClass()->GetFName(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityObjectGroupDetails::MakeInstance)
        );
        
        RegisterExtentEditorTabSpawner();
    }

    virtual void ShutdownModule() override {
        Window->Shutdown();

        FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
        PropertyModule.UnregisterCustomPropertyTypeLayout(
            APLATEAUCityModelLoader::StaticClass()->GetFName()
        );
        PropertyModule.UnregisterCustomPropertyTypeLayout(
            UPLATEAUCityObjectGroup::StaticClass()->GetFName()
        );
        
        const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
        ExtentEditor->UnregisterTabSpawner(GlobalTabManager);
    }

    virtual TSharedRef<FPLATEAUWindow> GetWindow() override {
        return Window.ToSharedRef();
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
