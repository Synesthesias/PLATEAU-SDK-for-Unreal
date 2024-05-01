// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ModelLandscape/PLATEAUModelLandscapeAPI.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"
#include <Reconstruct/PLATEAUHeightMapCreator.h>

void UPLATEAUModelLandscapeAPI::CreateLandscape(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, bool bDestroyOriginal) {
#if WITH_EDITOR
    TargetCityModel->CreateLandscape(TargetComponents, bDestroyOriginal);
#else
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("この機能は、エディタのみでご利用いただけます。")));
#endif  
}


void UPLATEAUModelLandscapeAPI::CreateLandscapeDummy(const UObject* Context) {

    const auto World = Context->GetWorld();
    FPLATEAUHeightMapCreator(false).CreateLandScape(World);
}