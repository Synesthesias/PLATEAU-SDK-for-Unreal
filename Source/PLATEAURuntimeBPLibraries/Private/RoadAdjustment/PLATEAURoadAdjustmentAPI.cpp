// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjustment/PLATEAURoadAdjustmentAPI.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"
#include "PLATEAUImportSettings.h"
#include "RoadNetwork/Structure/PLATEAURnStructureModel.h"
#include "RoadAdjustment/RoadReproducer.h"
#include "RoadAdjustment/RrTargetModel.h"

#include "RoadAdjustment/ICrosswalkPlacementRule.h"

void UPLATEAURoadAdjustmentAPI::CreateRnModel(APLATEAUInstancedCityModel* TargetCityModel,
                                              APLATEAURnStructureModel* DestActor)
{
#if WITH_EDITOR
    DestActor->CreateRnModelAsync(TargetCityModel);
#else
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("この機能は、エディタのみでご利用いただけます。")));
#endif
}

void UPLATEAURoadAdjustmentAPI::GenerateRoadMarking(APLATEAURnStructureModel* DestActor) {
#if WITH_EDITOR
    // DestActorがnullptrであることを許容しない
    if (!DestActor) {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("DestActorがnullptrです。")));
        return;
    }
    if (!DestActor->Model) {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Modelがnullptrです。")));
        return;
    }
    auto model = NewObject<URrTargetModel>();
    model->Initialize(DestActor->Model);
    TObjectPtr<URoadReproducer> RoadReproducerPtr = NewObject<URoadReproducer>();
    RoadReproducerPtr->Generate(model, ECrosswalkFrequency::None);
#else
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("この機能は、エディタのみでご利用いただけます。")));
#endif

}
