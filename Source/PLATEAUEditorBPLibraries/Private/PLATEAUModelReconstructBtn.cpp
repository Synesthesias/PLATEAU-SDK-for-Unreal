// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUModelReconstructBtn.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"

TArray<UActorComponent*> UPLATEAUModelReconstructBtn::GetSelectedComponents(AActor* Actor) {
    TArray<UActorComponent*> arr;
    if (Actor != nullptr) {
        for (auto& c : Actor->GetComponents())
            if (c->IsSelected())
                arr.Add(c);
    }
    return arr;
}

TArray<UActorComponent*> UPLATEAUModelReconstructBtn::GetSelectedComponentsByClass(AActor* Actor, UClass* Class) {
    TArray<UActorComponent*> arr;
    if (Actor != nullptr) {
        for (auto& c : Actor->GetComponents())
            if (c->IsSelected() && c->GetClass() == Class)
                arr.Add(c);
    }
    return arr;
}

void UPLATEAUModelReconstructBtn::ReconstructModel(APLATEAUInstancedCityModel* TargetCityModel, TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const uint8 ReconstructType, bool bDivideGrid) {
    
    //TODO: 分割・結合　処理
    
    UE_LOG(LogTemp, Log, TEXT("ReconstructModel: %s %d %d %s"), *TargetCityModel->GetActorNameOrLabel(), TargetCityObjects.Num(), ReconstructType, bDivideGrid ? TEXT("True"): TEXT("False"));


}