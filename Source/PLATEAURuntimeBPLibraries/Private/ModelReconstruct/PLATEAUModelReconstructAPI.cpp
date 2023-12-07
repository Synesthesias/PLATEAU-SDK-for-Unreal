// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ModelReconstruct/PLATEAUModelReconstructAPI.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"
#include "PLATEAUImportSettings.h"

TArray<UActorComponent*> UPLATEAUModelReconstructAPI::GetSelectedComponents(AActor* Actor) {
    TArray<UActorComponent*> arr;
    if (Actor != nullptr) {
        for (auto& c : Actor->GetComponents())
            if (c->IsSelected())
                arr.Add(c);
    }
    return arr;
}

TArray<UActorComponent*> UPLATEAUModelReconstructAPI::GetSelectedComponentsByClass(AActor* Actor, UClass* Class) {
    TArray<UActorComponent*> arr;
    if (Actor != nullptr) {
        for (auto& c : Actor->GetComponents())
            if (c->IsSelected() && c->GetClass() == Class)
                arr.Add(c);
    }
    return arr;
}

EPLATEAUMeshGranularity UPLATEAUModelReconstructAPI::GetMeshGranularityFromIndex(int index) {
    switch (index) {
    case 0: return EPLATEAUMeshGranularity::PerCityModelArea; 
    case 1: return EPLATEAUMeshGranularity::PerPrimaryFeatureObject;
    case 2: return EPLATEAUMeshGranularity::PerAtomicFeatureObject;
    }
    return EPLATEAUMeshGranularity::PerPrimaryFeatureObject;
}

void UPLATEAUModelReconstructAPI::ReconstructModel(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal ) {
    TargetCityModel->ReconstructModel(TargetComponents, ReconstructType, false, bDestroyOriginal);
}