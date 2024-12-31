#pragma once
#include <memory>
//#include "CityGML/PLATEAUCityObject.h"
//#include "Serialization/JsonWriter.h"
//#include "Serialization/JsonReader.h"
//#include "Serialization/JsonSerializer.h"
#include "SubDividedCityObject.h"
#include "../../Component/PLATEAUSceneComponent.h"
#include "PLATEAUSubDividedCityObjectGroup.generated.h"
UCLASS()
class PLATEAURUNTIME_API UPLATEAUSubDividedCityObjectGroup : public UPLATEAUSceneComponent
{
    GENERATED_BODY()
public:
    // --------------------
    // start:フィールド
    // --------------------
    // 保存された中間データ

private:
    TArray<FSubDividedCityObject> cityObjects;
};
