// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "../PLATEAUAutomationTestBase.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"
#include <PLATEAURuntime.h>
#include "CityGML/Serialization/PLATEAUCityObjectSerialization.h"
#include "CityGML/Serialization//PLATEAUCityObjectDeserialization.h"


namespace FPLATEAUTest_CityObjectGroup_Serialize_Local {

    FString CityObjectsSerialized = TEXT("{\"outsideParent\":\"\",\"outsideChildren\":[],\"cityObjects\":[\
        {\"gmlID\":\"bldg_000000-0000-0000-0000-000000000000\",\"cityObjectIndex\":[0, -1],\"cityObjectType\":\"Building\",\"attributes\":[\
        {\"key\":\"test:attr:key\",\"type\":\"String\",\"value\":\"TestAttrValue\"}],\
        \"children\":[{\"gmlID\":\"bldg_00000000_BuildingInstallation_0000\",\"cityObjectIndex\":[0, 0],\"cityObjectType\":\"BuildingInstallation\",\"attributes\":[]}]\
        }]}");
}


/// <summary>
/// シリアライズ・デシリアライズ
/// </summary>
IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_CityObjectGroup_Serialize, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.CityObjectGroup.Serialize", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_CityObjectGroup_Serialize::RunTest(const FString& Parameters) {
    InitializeTest("CityObjectGroup.Serialize");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");
   
    // Deserialize 
    TArray<FPLATEAUCityObject> OutCityObjects;
    TArray<TObjectPtr<USceneComponent>> AttachChildren;
    FString OutsideParent;
    FPLATEAUCityObjectDeserialization Deserializer;
    Deserializer.DeserializeCityObjects(FPLATEAUTest_CityObjectGroup_Serialize_Local::CityObjectsSerialized, AttachChildren, OutCityObjects, OutsideParent);

    // Assertions Deserialize
    FPLATEAUCityObject CityObject = OutCityObjects[0];
    TestEqual("Deser CityObject Gml ID", CityObject.GmlID, "bldg_000000-0000-0000-0000-000000000000");
    TestEqual("Deser CityObject Attr Value ", CityObject.Attributes.AttributeMap["test:attr:key"].StringValue, "TestAttrValue");
    TestEqual("Deser CityObject Num Children ", CityObject.Children.Num(), 1);
    TestEqual("Deser CityObject child Gml ID ", CityObject.Children[0].GmlID, "bldg_00000000_BuildingInstallation_0000");

    // Simple Serialize    
    TArray<FString> Children;
    FPLATEAUCityObjectSerialization Serializer;
    FString Serialized = Serializer.SerializeCityObject(CityObject, OutsideParent, Children);

    // Assertions Serialize
    FString SerializedFormatted = Serialized.Replace(TEXT("\n"), TEXT("")).Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\t"), TEXT("")).Replace(TEXT(" "), TEXT("")); //改行、Tab, スペース
    FString OriginalFormatted = FPLATEAUTest_CityObjectGroup_Serialize_Local::CityObjectsSerialized.Replace(TEXT("\n"), TEXT("")).Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\t"), TEXT("")).Replace(TEXT(" "), TEXT(""));
    TestEqual("Reserialized CityObject", SerializedFormatted, OriginalFormatted);

    return true;
}
