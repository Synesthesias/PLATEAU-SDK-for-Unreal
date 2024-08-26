// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "CityGML/Serialization/PLATEAUCityObjectSerializationCityModel.h"
#include <Component/PLATEAUCityObjectGroup.h>
#include "Util/PLATEAUGmlUtil.h"
#include <citygml/citygml.h>
#include <citygml/cityobject.h>

FString FPLATEAUCityObjectSerializationCityModel::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity) {

    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);
    // 親はなし
    JsonRootObject->SetStringField(plateau::CityObjectGroup::OutsideParentFieldName, "");

    // 子コンポーネント名取得
    TArray<TSharedPtr<FJsonValue>> OutsideChildrenJsonArray;
    for (int32 i = 0; i < InNode.getChildCount(); i++) {
        OutsideChildrenJsonArray.Emplace(MakeShared<FJsonValueString>(UTF8_TO_TCHAR(InNode.getChildAt(i).getName().c_str())));
    }
    JsonRootObject->SetArrayField(plateau::CityObjectGroup::OutsideChildrenFieldName, OutsideChildrenJsonArray);

    // CityObjects取得
    TArray<TSharedPtr<FJsonValue>> CityObjectsJsonArray;
    CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(InCityObject)));
    JsonRootObject->SetArrayField(plateau::CityObjectGroup::CityObjectsFieldName, CityObjectsJsonArray);

    // Json書き出し
    FString SerializedCityObjects;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
    return SerializedCityObjects;
}

FString FPLATEAUCityObjectSerializationCityModel::SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh,
    const plateau::polygonMesh::MeshGranularity& Granularity, std::shared_ptr<const citygml::CityModel> InCityModel) {

    const auto& CityObjectList = InMesh.getCityObjectList();
    const std::vector<plateau::polygonMesh::CityObjectIndex> CityObjectIndices = *CityObjectList.getAllKeys();
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);

    JsonRootObject->SetStringField(plateau::CityObjectGroup::OutsideParentFieldName, "");
    JsonRootObject->SetArrayField(plateau::CityObjectGroup::OutsideChildrenFieldName, {});

    // 最小地物単位の親を求める（主要地物のIDを設定）
    if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == Granularity) {
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            if (AtomicGmlId != InNodeName) {
                JsonRootObject->SetStringField(plateau::CityObjectGroup::OutsideParentFieldName, UTF8_TO_TCHAR(AtomicGmlId.c_str()));
            }
        }
    }

    if (plateau::polygonMesh::MeshGranularity::PerCityModelArea == Granularity) {
        // 地域単位
        TArray<TSharedPtr<FJsonValue>> CityObjectJsonArray;
        TSharedPtr<FJsonObject> CityJsonObjectParent = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> CityJsonObjectChildren;
        int32 CurrentPrimaryIndex = -1;
        for (int32 i = 0; i < CityObjectIndices.size(); i++) {
            const auto& CityObjectIndex = CityObjectIndices[i];
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
            if (CityObject == nullptr)
                continue;

            if (CityObjectIndex.primary_index != CurrentPrimaryIndex) {
                // 主要地物
                CurrentPrimaryIndex = CityObjectIndex.primary_index;
                CityJsonObjectParent = GetCityJsonObject(CityObject, CityObjectIndex);
                CityObjectJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObjectParent));
                CityJsonObjectChildren.Empty();
            }
            else {
                // 最小地物
                CityJsonObjectChildren.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(CityObject, CityObjectIndex)));
            }

            // データの区切りか？
            if (i + 1 == CityObjectIndices.size() || CityObjectIndices[i + 1].primary_index != CurrentPrimaryIndex) {
                CityJsonObjectParent->SetArrayField(plateau::CityObjectGroup::ChildrenFieldName, CityJsonObjectChildren);
            }
        }
        JsonRootObject->SetArrayField(plateau::CityObjectGroup::CityObjectsFieldName, CityObjectJsonArray);
    }
    else {
        // 最小地物単位・主要地物単位共通
        if (const auto& CityObjectParent = InCityModel->getCityObjectById(InNodeName); CityObjectParent != nullptr) {
            const auto& CityObjectParentIndex = CityObjectList.getCityObjectIndex(InNodeName);
            const auto& CityJsonObjectParent = GetCityJsonObject(CityObjectParent, CityObjectParentIndex);
            TArray<TSharedPtr<FJsonValue>> CityObjectJsonArray;
            CityObjectJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObjectParent));

            if (plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject == Granularity) {
                TArray<TSharedPtr<FJsonValue>> CityObjectsChildrenJsonArray;
                for (const auto& CityObjectIndex : CityObjectIndices) {
                    const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
                    if (AtomicGmlId == InNodeName)
                        // 親は前の処理で既に情報抽出済み
                        continue;

                    const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
                    if (CityObject == nullptr)
                        continue;

                    CityObjectsChildrenJsonArray.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(CityObject, CityObjectIndex)));
                }
                CityJsonObjectParent->SetArrayField(plateau::CityObjectGroup::ChildrenFieldName, CityObjectsChildrenJsonArray);
            }
            JsonRootObject->SetArrayField(plateau::CityObjectGroup::CityObjectsFieldName, CityObjectJsonArray);
        }
    }

    FString SerializedCityObjects;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
    return SerializedCityObjects;
}

/**
* @brief 再帰的に属性マップから属性情報を取得
* @param InAttributesMap 属性マップ
* @param InAttributesJsonObjectArray 属性情報を格納する配列
*/
void FPLATEAUCityObjectSerializationCityModel::GetAttributesJsonObjectRecursive(const citygml::AttributesMap& InAttributesMap, TArray<TSharedPtr<FJsonValue>>& InAttributesJsonObjectArray) {
    for (const auto& [key, value] : InAttributesMap) {
        TSharedRef<FJsonObject> AttributesJsonObject = MakeShared<FJsonObject>();
        if (citygml::AttributeType::AttributeSet == value.getType()) {
            TArray<TSharedPtr<FJsonValue>> AttributeSetJsonObjectArray;
            GetAttributesJsonObjectRecursive(value.asAttributeSet(), AttributeSetJsonObjectArray);
            AttributesJsonObject->SetStringField(plateau::CityObjectGroup::KeyFieldName, UTF8_TO_TCHAR(key.c_str()));
            AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "AttributeSets");
            AttributesJsonObject->SetArrayField(plateau::CityObjectGroup::ValueFieldName, AttributeSetJsonObjectArray);
            InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
        }
        else {
            AttributesJsonObject->SetStringField(plateau::CityObjectGroup::KeyFieldName, UTF8_TO_TCHAR(key.c_str()));

            switch (value.getType()) {
            case citygml::AttributeType::String:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "String");
                break;
            case citygml::AttributeType::Double:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Double");
                break;
            case citygml::AttributeType::Integer:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Integer");
                break;
            case citygml::AttributeType::Date:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Date");
                break;
            case citygml::AttributeType::Uri:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Uri");
                break;
            case citygml::AttributeType::Measure:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Measure");
                break;
            case citygml::AttributeType::Boolean:
                AttributesJsonObject->SetStringField(plateau::CityObjectGroup::TypeFieldName, "Boolean");
                break;
            default: UE_LOG(LogTemp, Log, TEXT("Error citygml::AttributeType"));
            }

            AttributesJsonObject->SetStringField(plateau::CityObjectGroup::ValueFieldName, UTF8_TO_TCHAR(value.asString().c_str()));
            InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
        }
    }
}

/**
* @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
* @param InCityObject CityModelから得られるシティオブジェクト情報
* @return シティオブジェクト情報
*/

TSharedRef<FJsonObject> FPLATEAUCityObjectSerializationCityModel::GetCityJsonObject(const citygml::CityObject* InCityObject) {
    TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();

    CityJsonObject->SetStringField(plateau::CityObjectGroup::GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

    TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
    CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(0));
    CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(-1));
    CityJsonObject->SetArrayField(plateau::CityObjectGroup::CityObjectIndexFieldName, CityObjectIndexArray);

    CityJsonObject->SetStringField(plateau::CityObjectGroup::CityObjectTypeFieldName, plateau::CityObject::CityObjectsTypeToString(InCityObject->getType()));

    TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
    GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
    CityJsonObject->SetArrayField(plateau::CityObjectGroup::AttributesFieldName, AttributesJsonObjectArray);

    return CityJsonObject;
}

/**
* @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
* @param InCityObject CityModelから得られるシティオブジェクト情報
* @param CityObjectIndex CityObjectListが持つインデックス情報
* @return シティオブジェクト情報
*/
TSharedRef<FJsonObject> FPLATEAUCityObjectSerializationCityModel::GetCityJsonObject(const citygml::CityObject* InCityObject, const plateau::polygonMesh::CityObjectIndex& CityObjectIndex) {
    TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();

    CityJsonObject->SetStringField(plateau::CityObjectGroup::GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

    TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
    CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.primary_index));
    CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.atomic_index));
    CityJsonObject->SetArrayField(plateau::CityObjectGroup::CityObjectIndexFieldName, CityObjectIndexArray);

    CityJsonObject->SetStringField(plateau::CityObjectGroup::CityObjectTypeFieldName, plateau::CityObject::CityObjectsTypeToString(InCityObject->getType()));

    TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
    GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
    CityJsonObject->SetArrayField(plateau::CityObjectGroup::AttributesFieldName, AttributesJsonObjectArray);

    return CityJsonObject;
}