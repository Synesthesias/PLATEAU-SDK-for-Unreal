// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/PLATEAUAttributeValue.h"
#include "PLATEAUCityObjectGroup.h"


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
