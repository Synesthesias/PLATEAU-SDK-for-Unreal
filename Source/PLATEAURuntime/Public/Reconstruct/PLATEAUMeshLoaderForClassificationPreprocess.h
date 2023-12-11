// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForClassificationPreprocess : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForClassificationPreprocess() {
        bAutomationTest = false;
    }

    FPLATEAUMeshLoaderForClassificationPreprocess(const bool InbAutomationTest) {
        bAutomationTest = InbAutomationTest;
    }

    //Material分け時のタイプリストをセットします
    void SetClassificationTypes(TArray<EPLATEAUCityObjectsType> Types);


protected:
    bool CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;
    UMaterialInstanceDynamic* GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;
    bool UseCachedMaterial() override;

private:

    EPLATEAUCityObjectsType GetCityObjectsTypeFromComponent(UPLATEAUCityObjectGroup* Component);

    //Material分け時のタイプリスト
    TArray<EPLATEAUCityObjectsType>  ClassificationTypes;

};
