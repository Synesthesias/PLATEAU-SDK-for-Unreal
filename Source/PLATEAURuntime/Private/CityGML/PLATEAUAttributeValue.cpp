// Fill out your copyright notice in the Description page of Project Settings.


#include "CityGML/PLATEAUAttributeValue.h"
#include "citygml/attributesmap.h"

EPLATEAUAttributeType UPLATEAUAttributeValueBlueprintLibrary::GetType(const FPLATEAUAttributeValue& Value) {
    if (Value.Data == nullptr)
        return EPLATEAUAttributeType::String;

    return static_cast<EPLATEAUAttributeType>(Value.Data->getType());
}

FString UPLATEAUAttributeValueBlueprintLibrary::GetString(const FPLATEAUAttributeValue& Value) {
    if (Value.Data == nullptr)
        return "";

    return UTF8_TO_TCHAR(Value.Data->asString().c_str());
}

TMap<FString, FPLATEAUAttributeValue>& UPLATEAUAttributeValueBlueprintLibrary::GetAttributeMap(FPLATEAUAttributeValue& Value) {
    if (Value.AttributeMapCache != nullptr)
        return *Value.AttributeMapCache;

    Value.AttributeMapCache = MakeShared<FPLATEAUAttributeMap>();

    if (Value.Data == nullptr)
        return *Value.AttributeMapCache;

    const auto& AttributeMapData = Value.Data->asAttributeSet();

    for (const auto& [Key, Val] : AttributeMapData) {
        Value.AttributeMapCache->Add(UTF8_TO_TCHAR(Key.c_str()), FPLATEAUAttributeValue(&Val));
    }
    return *Value.AttributeMapCache;
}

