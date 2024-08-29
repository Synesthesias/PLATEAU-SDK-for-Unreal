// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUAttributeValue.h"
#include "Component/PLATEAUCityObjectGroup.h"

namespace {
    constexpr TCHAR EPLATEAUAttributeTypePath[] = TEXT("/Script/PLATEAURuntime.EPLATEAUAttributeType");
}

void FPLATEAUAttributeValue::SetType(const FString& InType) {
    if (const UEnum* EnumPtr = FindObject<UEnum>(nullptr, EPLATEAUAttributeTypePath)) {
        const auto EnumValue = EnumPtr->GetValueByName(FName(*InType));
        Type = static_cast<EPLATEAUAttributeType>(EnumValue);
    } else {
        UE_LOG(LogTemp, Error, TEXT("Enum pointer not found: %s"), *InType);
    }
}

void FPLATEAUAttributeValue::SetValue(const EPLATEAUAttributeType& InType, const TSharedPtr<FJsonObject>& InValue) {
    if (EPLATEAUAttributeType::AttributeSets == InType) {
        const auto& AttributeValue = InValue->GetArrayField(plateau::CityObjectGroup::ValueFieldName);
        SetValue(AttributeValue);
    } else {
        const auto& AttributeValue = InValue->GetStringField(plateau::CityObjectGroup::ValueFieldName);
        SetValue(InType, AttributeValue);
    }
}

void FPLATEAUAttributeValue::SetValue(const EPLATEAUAttributeType& InType, const FString& InValue) {
    switch (InType) {
    case EPLATEAUAttributeType::String:
    case EPLATEAUAttributeType::Date:
    case EPLATEAUAttributeType::Uri:
        StringValue = InValue;
        break;
    case EPLATEAUAttributeType::Double:
        StringValue = InValue;
        DoubleValue = FCString::Atod(*InValue);
        break;
    case EPLATEAUAttributeType::Integer:
    case EPLATEAUAttributeType::Boolean:
        StringValue = InValue;
        IntValue = FCString::Atoi(*InValue);
        break;
    case EPLATEAUAttributeType::Measure:
        StringValue = InValue;
        DoubleValue = FCString::Atod(*InValue);
        break;
    default: UE_LOG(LogTemp, Error, TEXT("Error InType: %d"), InType);
    }
}

void FPLATEAUAttributeValue::SetValue(const TArray<TSharedPtr<FJsonValue>>& InValue) {
    Attributes = MakeShared<FPLATEAUAttributeMap>();
    for (const auto& AttributeJsonValue : InValue) {
        const auto& AttributeJsonObject = AttributeJsonValue->AsObject();
        const auto& AttributeKey = AttributeJsonObject->GetStringField(plateau::CityObjectGroup::KeyFieldName);
        const auto& AttributeType = AttributeJsonObject->GetStringField(plateau::CityObjectGroup::TypeFieldName);

        FPLATEAUAttributeValue PLATEAUAttributeValue;
        PLATEAUAttributeValue.SetType(AttributeType);
        if (EPLATEAUAttributeType::AttributeSets == PLATEAUAttributeValue.Type) {
            const auto& AttributeValue = AttributeJsonObject->GetArrayField(plateau::CityObjectGroup::ValueFieldName);
            PLATEAUAttributeValue.SetValue(AttributeValue);
        } else {
            const auto& AttributeValue = AttributeJsonObject->GetStringField(plateau::CityObjectGroup::ValueFieldName);
            PLATEAUAttributeValue.SetValue(PLATEAUAttributeValue.Type, AttributeValue);
        }

        Attributes->AttributeMap.Add(AttributeKey, PLATEAUAttributeValue);
    }
}

void FPLATEAUAttributeValue::SetAttributeValue(const citygml::AttributeValue& Value) {
    SetType(UPLATEAUAttributeValueBlueprintLibrary::AttributeTypeToString(Value.getType()));
    switch (Value.getType()) {
    case citygml::AttributeType::String:
    case citygml::AttributeType::Date:
    case citygml::AttributeType::Uri:
        StringValue = UTF8_TO_TCHAR(Value.asString().c_str());
        break;
    case citygml::AttributeType::Double:
        DoubleValue = Value.asDouble();
        StringValue = FString::SanitizeFloat(DoubleValue);
        break;
    case citygml::AttributeType::Integer:
    case citygml::AttributeType::Boolean:
        IntValue = Value.asInteger();
        StringValue = FString::FromInt(IntValue);
        break;
    case citygml::AttributeType::Measure:
        DoubleValue = Value.asDouble();
        StringValue = FString::SanitizeFloat(DoubleValue);
        break;
    case citygml::AttributeType::AttributeSet:
        Attributes = MakeShared<FPLATEAUAttributeMap>();
        citygml::AttributesMap CityGmlAttrMap = Value.asAttributeSet();
        for (const auto& AttrKeyVal : CityGmlAttrMap) {    
            const FString& AttrKey = UTF8_TO_TCHAR(AttrKeyVal.first.c_str());
            FPLATEAUAttributeValue PLATEAUAttributeValue;
            PLATEAUAttributeValue.SetAttributeValue(AttrKeyVal.second);
            Attributes->AttributeMap.Add(AttrKey, PLATEAUAttributeValue);
        }
        break;
    }
}

EPLATEAUAttributeType UPLATEAUAttributeValueBlueprintLibrary::GetType(const FPLATEAUAttributeValue& Value) {
    return Value.Type;
}

int UPLATEAUAttributeValueBlueprintLibrary::GetInt(const FPLATEAUAttributeValue& Value) {
    return Value.IntValue;
}

double UPLATEAUAttributeValueBlueprintLibrary::GetDouble(const FPLATEAUAttributeValue& Value) {
    return Value.DoubleValue;
}

FString UPLATEAUAttributeValueBlueprintLibrary::GetString(const FPLATEAUAttributeValue& Value) {
    return Value.StringValue;
}

FPLATEAUAttributeMap UPLATEAUAttributeValueBlueprintLibrary::GetAttributes(const FPLATEAUAttributeValue& Value) {
    return Value.Attributes->AttributeMap;
}

TArray<FPLATEAUAttributeValue> UPLATEAUAttributeValueBlueprintLibrary::GetAttributesByKey(UPARAM(ref) const FString& Key, UPARAM(ref) const FPLATEAUAttributeMap& AttributeMap) {
    TArray<FString> Keys;
    int32 Length = Key.ParseIntoArray(Keys, TEXT("/"), false);
    FString FirstKey = Length > 0 ? Keys[0] : "";
    TArray<FPLATEAUAttributeValue> Values;
    if (AttributeMap.AttributeMap.Contains(FirstKey)) {
        const auto& attr = AttributeMap.AttributeMap[FirstKey];
        if (attr.Type != EPLATEAUAttributeType::AttributeSets) {
            Values.Add(attr);
        }
        else {
            Keys.RemoveSingle(FirstKey);
            FString NewKey = FString::Join<TArray<FString>>(Keys, TEXT("/"));
            const auto& ChildAttr = attr.Attributes.Get();
            return GetAttributesByKey(NewKey, *ChildAttr);
        }
    }
    return Values;
}

FString UPLATEAUAttributeValueBlueprintLibrary::AttributeTypeToString(const citygml::AttributeType InType) {
    switch (InType) {
    case citygml::AttributeType::String:
        return "String";
    case citygml::AttributeType::Double:
        return "Double";
    case citygml::AttributeType::Integer:
        return "Integer";
    case citygml::AttributeType::Date:
        return "Date";
    case citygml::AttributeType::Uri:
        return "Uri";
    case citygml::AttributeType::Measure:
        return "Measure";
    case citygml::AttributeType::Boolean:
        return "Boolean";
    case citygml::AttributeType::AttributeSet:
        return "AttributeSets";
    }
    return "String";
}
