// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlateauSDK.h"

#include "PLATEAUCityMap.h"
#include "CityMapDetails/PLATEAUCityMapDetails.h"

#define LOCTEXT_NAMESPACE "FPlateauSDKModule"

void FPlateauSDKModule::StartupModule()
{
    m_instance.startup();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.RegisterCustomClassLayout(
        APLATEAUCityMap::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUCityMapDetails::MakeInstance)
    );
}

void FPlateauSDKModule::ShutdownModule()
{
    m_instance.shutdown();

    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    PropertyModule.UnregisterCustomPropertyTypeLayout(
        APLATEAUCityMap::StaticClass()->GetFName()
    );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlateauSDKModule, PlateauSDK)