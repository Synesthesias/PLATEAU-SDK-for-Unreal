// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlateauSDK.h"

#define LOCTEXT_NAMESPACE "FPlateauSDKModule"

void FPlateauSDKModule::StartupModule()
{
    instance_.startup();
}

void FPlateauSDKModule::ShutdownModule()
{
    instance_.shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPlateauSDKModule, PlateauSDK)