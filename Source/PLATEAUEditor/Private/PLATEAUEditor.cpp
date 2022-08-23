// Copyright Epic Games, Inc. All Rights Reserved.

#include "PLATEAUEditor.h"

#include "PLATEAUCityModelLoader.h"
#include "CityMapDetails/PLATEAUCityMapDetails.h"
#include "PLATEAUCityModelImportWindow.h"


void FPLATEAUEditorModule::StartupModule()
{
    FPLATEAUCityModelImportWindow::GetInstance()->Startup();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        APLATEAUCityModelLoader::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityMapDetails::MakeInstance)
    );
}

void FPLATEAUEditorModule::ShutdownModule()
{
    FPLATEAUCityModelImportWindow::GetInstance()->Shutdown();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.UnregisterCustomPropertyTypeLayout(
        APLATEAUCityModelLoader::StaticClass()->GetFName()
    );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAUEditorModule, PLATEAUEditor)