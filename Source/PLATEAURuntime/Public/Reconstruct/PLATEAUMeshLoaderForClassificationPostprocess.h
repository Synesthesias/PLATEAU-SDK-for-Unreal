// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForClassificationPostprocess : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForClassificationPostprocess() {
        bAutomationTest = false;
    }

    FPLATEAUMeshLoaderForClassificationPostprocess(const bool InbAutomationTest) {
        bAutomationTest = InbAutomationTest;
    }

    //Material分け時のマテリアルリストをセットします
    void SetClassificationMaterials(TMap<EPLATEAUCityObjectsType, UMaterialInterface*>& Materials);

protected:
    bool CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;
    UMaterialInstanceDynamic* GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;

private:

    //Material分け時のマテリアルリスト
    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
};
