// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlateauSDK.h"

#define LOCTEXT_NAMESPACE "FPlateauSDKModule"

void FPlateauSDKModule::StartupModule()
{
    m_instance.startup();
}

void FPlateauSDKModule::ShutdownModule()
{
    m_instance.shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlateauSDKModule, PlateauSDK)