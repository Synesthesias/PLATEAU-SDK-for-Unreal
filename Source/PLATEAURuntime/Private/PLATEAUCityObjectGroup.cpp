// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroup.h"
#include "JsonObjectConverter.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
#include "CityGML/PLATEAUCityObject.h"
#include <citygml/cityobject.h>


namespace {
    /**
     * @brief 再帰的に属性マップから属性情報を取得
     * @param InAttributesMap 属性マップ 
     * @param InAttributesJsonObjectArray 属性情報を格納する配列
     */
    void GetAttributesJsonObjectRecursive(const citygml::AttributesMap& InAttributesMap, TArray<TSharedPtr<FJsonValue>>& InAttributesJsonObjectArray) {
        for (const auto& [key, value] : InAttributesMap) {
            TSharedRef<FJsonObject> AttributesJsonObject = MakeShared<FJsonObject>();
            if (citygml::AttributeType::AttributeSet == value.getType()) {
                TArray<TSharedPtr<FJsonValue>> AttributeSetJsonObjectArray;
                GetAttributesJsonObjectRecursive(value.asAttributeSet(), AttributeSetJsonObjectArray);
                AttributesJsonObject->SetStringField(plateau::CityObject::KeyFieldName, UTF8_TO_TCHAR(key.c_str()));
                AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "AttributeSets");
                AttributesJsonObject->SetArrayField(plateau::CityObject::ValueFieldName, AttributeSetJsonObjectArray);
                InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
            } else {
                AttributesJsonObject->SetStringField(plateau::CityObject::KeyFieldName, UTF8_TO_TCHAR(key.c_str()));

                switch (value.getType()) {
                case citygml::AttributeType::String:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "String");
                    break;
                case citygml::AttributeType::Double:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Double");
                    break;
                case citygml::AttributeType::Integer:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Integer");
                    break;
                case citygml::AttributeType::Date:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Date");
                    break;
                case citygml::AttributeType::Uri:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Uri");
                    break;
                case citygml::AttributeType::Measure:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Measure");
                    break;
                case citygml::AttributeType::Boolean:
                    AttributesJsonObject->SetStringField(plateau::CityObject::TypeFieldName, "Boolean");
                    break;
                default: UE_LOG(LogTemp, Log, TEXT("Error citygml::AttributeType"));
                }

                AttributesJsonObject->SetStringField(plateau::CityObject::ValueFieldName, UTF8_TO_TCHAR(value.asString().c_str()));
                InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
            }
        }
    }

    /**
     * @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     * @return シティオブジェクト情報
     */
    TSharedRef<FJsonObject> GetCityJsonObject(const citygml::CityObject* InCityObject) {
        TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();

        CityJsonObject->SetStringField(plateau::CityObject::GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(0));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(-1));
        CityJsonObject->SetArrayField(plateau::CityObject::CityObjectIndexFieldName, CityObjectIndexArray);

        CityJsonObject->SetNumberField(plateau::CityObject::CityObjectTypeFieldName, static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(plateau::CityObject::AttributesFieldName, AttributesJsonObjectArray);

        return CityJsonObject;
    }

    /**
     * @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     * @param CityObjectIndex CityObjectListが持つインデックス情報
     * @return シティオブジェクト情報
     */
    TSharedRef<FJsonObject> GetCityJsonObject(const citygml::CityObject* InCityObject, const plateau::polygonMesh::CityObjectIndex& CityObjectIndex) {
        TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();

        CityJsonObject->SetStringField(plateau::CityObject::GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.primary_index));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.atomic_index));
        CityJsonObject->SetArrayField(plateau::CityObject::CityObjectIndexFieldName, CityObjectIndexArray);

        CityJsonObject->SetNumberField(plateau::CityObject::CityObjectTypeFieldName, static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(plateau::CityObject::AttributesFieldName, AttributesJsonObjectArray);

        return CityJsonObject;
    }

    FPLATEAUCityObject GetCityObject(TSharedPtr<FJsonValue> CityJsonValue) {
        FPLATEAUCityObject CityObject;
        const auto& CityJsonObject = CityJsonValue->AsObject();
        CityObject.SetGmlID(CityJsonObject->GetStringField(plateau::CityObject::GmlIdFieldName));

        const auto& CityObjectIndexJsonValueArray = CityJsonObject->GetArrayField(plateau::CityObject::CityObjectIndexFieldName);
        const auto PrimaryIndexJsonValue = CityObjectIndexJsonValueArray[0];
        const auto AtomicIndexJsonValue = CityObjectIndexJsonValueArray[1];
        plateau::polygonMesh::CityObjectIndex CityObjectIndex{
            static_cast<int>(PrimaryIndexJsonValue->AsNumber()),
            static_cast<int>(AtomicIndexJsonValue->AsNumber())
        };
        CityObject.SetCityObjectIndex(CityObjectIndex);

        CityObject.SetCityObjectsType(static_cast<int64>(CityJsonObject->GetNumberField(plateau::CityObject::CityObjectTypeFieldName)));

        TMap<FString, FPLATEAUAttributeValue> AttributeMap;
        const auto& AttributesJsonValueArray = CityJsonObject->GetArrayField(plateau::CityObject::AttributesFieldName);
        for (const auto& AttributeJsonValue : AttributesJsonValueArray) {
            const auto& AttributeJsonObject = AttributeJsonValue->AsObject();
            const auto& AttributeKey = AttributeJsonObject->GetStringField(plateau::CityObject::KeyFieldName);
            const auto& AttributeType = AttributeJsonObject->GetStringField(plateau::CityObject::TypeFieldName);

            FPLATEAUAttributeValue PLATEAUAttributeValue;
            PLATEAUAttributeValue.SetType(AttributeType);
            PLATEAUAttributeValue.SetValue(PLATEAUAttributeValue.Type, AttributeJsonObject);
            AttributeMap.Add(AttributeKey, PLATEAUAttributeValue);
        }
        CityObject.SetAttribute(AttributeMap);

        const TArray<TSharedPtr<FJsonValue>>* CityObjectsChildren;
        if (CityJsonObject->TryGetArrayField(plateau::CityObject::ChildrenFieldName, CityObjectsChildren)) {
            for (const auto& CityObjectChild : *CityObjectsChildren) {
                CityObject.Children.Emplace(GetCityObject(CityObjectChild));
            }
        }

        return CityObject;
    }
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject) {
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);

    // 親はなし
    JsonRootObject->SetStringField(plateau::CityObject::OutsideParentFieldName, "");

    // 子コンポーネント名取得
    TArray<TSharedPtr<FJsonValue>> OutsideChildrenJsonArray;
    for (int i = 0; i < InNode.getChildCount(); i++) {
        OutsideChildrenJsonArray.Emplace(MakeShared<FJsonValueString>(UTF8_TO_TCHAR(InNode.getChildAt(i).getName().c_str())));
    }
    JsonRootObject->SetArrayField(plateau::CityObject::OutsideChildrenFieldName, OutsideChildrenJsonArray);

    // CityObjects取得
    TArray<TSharedPtr<FJsonValue>> CityObjectsJsonArray;
    CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(InCityObject)));
    JsonRootObject->SetArrayField(plateau::CityObject::CityObjectsFieldName, CityObjectsJsonArray);

    // Json書き出し
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& InLoadInputData, const std::shared_ptr<const citygml::CityModel> InCityModel) {
    const auto& CityObjectList = InMesh.getCityObjectList();
    const std::vector<plateau::polygonMesh::CityObjectIndex> CityObjectIndices = *CityObjectList.getAllKeys();
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);
    JsonRootObject->SetStringField(plateau::CityObject::OutsideParentFieldName, "");
    JsonRootObject->SetArrayField(plateau::CityObject::OutsideChildrenFieldName, {});

    // 最小地物単位の親を求める（主要地物のIDを設定）
    if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            if (AtomicGmlId != InNodeName) {
                JsonRootObject->SetStringField(plateau::CityObject::OutsideParentFieldName, UTF8_TO_TCHAR(AtomicGmlId.c_str()));
            }
        }
    }

    if (plateau::polygonMesh::MeshGranularity::PerCityModelArea == InLoadInputData.ExtractOptions.mesh_granularity) {
        TSharedPtr<FJsonObject> CityJsonObjectParent = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> CityObjectsChildrenJsonArray;
        const TArray<TSharedPtr<FJsonValue>>* TempJsonArray;
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
            if (CityObject == nullptr)
                continue;

            if (JsonRootObject->TryGetArrayField(plateau::CityObject::CityObjectsFieldName, TempJsonArray)) {
                CityObjectsChildrenJsonArray.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(CityObject, CityObjectIndex)));
            } else {
                CityJsonObjectParent = GetCityJsonObject(CityObject, CityObjectIndex);
                JsonRootObject->SetObjectField(plateau::CityObject::CityObjectsFieldName, CityJsonObjectParent);
            }
        }
        CityJsonObjectParent->SetArrayField(plateau::CityObject::ChildrenFieldName, CityObjectsChildrenJsonArray);
    } else {
        // 最小地物単位・主要地物単位共通
        if (const auto& CityObjectParent = InCityModel->getCityObjectById(InNodeName); CityObjectParent != nullptr) {
            const auto& CityObjectParentIndex = CityObjectList.getCityObjectIndex(InNodeName);
            const auto& CityJsonObjectParent = GetCityJsonObject(CityObjectParent, CityObjectParentIndex);
            TArray<TSharedPtr<FJsonValue>> CityObjectJsonArray;
            CityObjectJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObjectParent));
            JsonRootObject->SetArrayField(plateau::CityObject::CityObjectsFieldName, CityObjectJsonArray);

            if (plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
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
                CityJsonObjectParent->SetArrayField(plateau::CityObject::ChildrenFieldName, CityObjectsChildrenJsonArray);
            }
        }
    }

    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetCityObjectByID(const FString& GmlID) {
    if (RootCityObjects.Num() <= 0) {
        GetAllRootCityObjects();
    }

    for (const auto& RootCityObject : RootCityObjects) {
        if (GmlID.Contains(RootCityObject.GmlID)) {
            return RootCityObject;
        }
    }

    return FPLATEAUCityObject();
}

TArray<FPLATEAUCityObject> UPLATEAUCityObjectGroup::GetAllRootCityObjects() {
    if (0 < RootCityObjects.Num()) {
        return RootCityObjects;
    }

    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(SerializedCityObjects);
    TSharedPtr<FJsonObject> JsonRootObject;
    FJsonSerializer::Deserialize(JsonReader, JsonRootObject);

    const auto& CityObjectsJsonArray = JsonRootObject->GetArrayField(plateau::CityObject::CityObjectsFieldName);
    for (const auto& CityJsonValue : CityObjectsJsonArray) {
        RootCityObjects.Emplace(GetCityObject(CityJsonValue));
    }

    // 最小地物単位
    const auto& OutsideChildrenJsonArray = JsonRootObject->GetArrayField(plateau::CityObject::OutsideChildrenFieldName);
    if (0 < OutsideChildrenJsonArray.Num() && 0 < RootCityObjects.Num()) {
        for (const auto& ChildComponent : GetAttachChildren()) {
            const auto& PLATEAUCityObjectGroup = Cast<UPLATEAUCityObjectGroup>(ChildComponent);
            RootCityObjects[0].Children.Append(PLATEAUCityObjectGroup->GetAllRootCityObjects());
        }
    }

    return RootCityObjects;
}
