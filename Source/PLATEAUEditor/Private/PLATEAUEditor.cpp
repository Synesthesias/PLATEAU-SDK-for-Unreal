// Copyright Epic Games, Inc. All Rights Reserved.

#include "PLATEAUEditor.h"

#include "PLATEAUCityModelLoader.h"
#include "CityMapDetails/PLATEAUCityMapDetails.h"

#define LOCTEXT_NAMESPACE "FPlateauSDKModule"

void FPLATEAUEditorModule::StartupModule()
{
    m_instance.startup();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        APLATEAUCityModelLoader::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityMapDetails::MakeInstance)
    );
}

void FPLATEAUEditorModule::ShutdownModule()
{
    m_instance.shutdown();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.UnregisterCustomPropertyTypeLayout(
        APLATEAUCityModelLoader::StaticClass()->GetFName()
    );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPLATEAUEditorModule, PLATEAUEditor)