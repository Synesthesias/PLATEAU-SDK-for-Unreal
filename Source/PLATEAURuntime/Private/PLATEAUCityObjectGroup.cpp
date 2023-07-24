// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroup.h"

#include "JsonObjectConverter.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
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
                AttributesJsonObject->SetStringField(TEXT("key"), UTF8_TO_TCHAR(key.c_str()));
                AttributesJsonObject->SetStringField(TEXT("type"), "AttributeSets");
                AttributesJsonObject->SetArrayField(TEXT("value"), AttributeSetJsonObjectArray);
                InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
            } else {
                AttributesJsonObject->SetStringField(TEXT("key"), UTF8_TO_TCHAR(key.c_str()));

                switch (value.getType()) {
                case citygml::AttributeType::String:
                    AttributesJsonObject->SetStringField(TEXT("type"), "String");
                    break;
                case citygml::AttributeType::Double:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Double");
                    break;
                case citygml::AttributeType::Integer:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Integer");
                    break;
                case citygml::AttributeType::Date:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Date");
                    break;
                case citygml::AttributeType::Uri:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Uri");
                    break;
                case citygml::AttributeType::Measure:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Measure");
                    break;
                case citygml::AttributeType::Boolean:
                    AttributesJsonObject->SetStringField(TEXT("type"), "Boolean");
                    break;
                default: UE_LOG(LogTemp, Log, TEXT("Error citygml::AttributeType"));
                }

                AttributesJsonObject->SetStringField(TEXT("value"), UTF8_TO_TCHAR(value.asString().c_str()));
                InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
            }
        }
    }

    /**
     * @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     * @return シティオブジェクト情報
     */
    TArray<TSharedPtr<FJsonValue>> GetCityObjectJsonValue(const citygml::CityObject* InCityObject) {
        TArray<TSharedPtr<FJsonValue>> CityObjectsJsonArray;
		TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();
        
        CityJsonObject->SetStringField(TEXT("gmlID"), UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(0));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(-1));
        CityJsonObject->SetArrayField(TEXT("cityObjectIndex"), CityObjectIndexArray);
        
        CityJsonObject->SetNumberField(TEXT("cityObjectType"), static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(TEXT("attributes"), AttributesJsonObjectArray);

        CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObject));
        return CityObjectsJsonArray;
    }

    /**
     * @brief シティオブジェクトからシリアライズに必要な情報を抽出してJsonValue配列として返却
     * @param InCityObject CityModelから得られるシティオブジェクト情報
     * @param CityObjectIndex CityObjectListが持つインデックス情報
     * @return シティオブジェクト情報
     */
    TArray<TSharedPtr<FJsonValue>> GetCityObjectJsonValue(const citygml::CityObject* InCityObject, const plateau::polygonMesh::CityObjectIndex& CityObjectIndex) {
        TArray<TSharedPtr<FJsonValue>> CityObjectsJsonArray;
        TSharedRef<FJsonObject> CityJsonObject = MakeShared<FJsonObject>();
        
        CityJsonObject->SetStringField(TEXT("gmlID"), UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.primary_index));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.atomic_index));
        CityJsonObject->SetArrayField(TEXT("cityObjectIndex"), CityObjectIndexArray);
        
        CityJsonObject->SetNumberField(TEXT("cityObjectType"), static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(TEXT("attributes"), AttributesJsonObjectArray);

        CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObject));
        return CityObjectsJsonArray;
    }
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject) {
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);

    // 親はなし
    JsonRootObject->SetStringField("outsideParent", "");

    // 子コンポーネント名取得
    TArray<TSharedPtr<FJsonValue>> OutsideChildrenJsonArray;
    for (int i = 0; i < InNode.getChildCount(); i++) {
        OutsideChildrenJsonArray.Emplace(MakeShared<FJsonValueString>(UTF8_TO_TCHAR(InNode.getChildAt(i).getName().c_str())));
    }
    JsonRootObject->SetArrayField("outsideChildren", OutsideChildrenJsonArray);

    // CityObjects取得
    NodeName = InNode.getName();
    JsonRootObject->SetArrayField("cityObjects", GetCityObjectJsonValue(InCityObject));

    // Json書き出し
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    // const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
    UE_LOG(LogTemp, Log, TEXT("SerializedCityObjects: %s"), *SerializedCityObjects);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh,
                                                  const FLoadInputData& InLoadInputData, const std::shared_ptr<const citygml::CityModel> InCityModel) {
    NodeName = InNodeName;
    const auto& CityObjectList = InMesh.getCityObjectList();
    const std::vector<plateau::polygonMesh::CityObjectIndex> CityObjectIndices = *CityObjectList.getAllKeys();
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);
    JsonRootObject->SetStringField("outsideParent", "");
    JsonRootObject->SetArrayField("outsideChildren", {});

    // 最小地物単位の親を求める（主要地物のIDを設定）
    if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity && AtomicGmlId != NodeName) {
                JsonRootObject->SetStringField("outsideParent", UTF8_TO_TCHAR(AtomicGmlId.c_str()));
            }
        }
    }

    // 地物単位毎にCityObjects情報抽出
    if (plateau::polygonMesh::MeshGranularity::PerCityModelArea == InLoadInputData.ExtractOptions.mesh_granularity) {
    } else {
        if (const auto& CityObject = InCityModel->getCityObjectById(NodeName); CityObject != nullptr) {
            const auto& CityObjectIndex =  CityObjectList.getCityObjectIndex(NodeName);         
            JsonRootObject->SetArrayField("cityObjects", GetCityObjectJsonValue(CityObject, CityObjectIndex));
        }
        
        if (plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
            TArray<TSharedPtr<FJsonValue>> CityObjectsChildrenJsonArray;
            for (const auto& CityObjectIndex : CityObjectIndices) {
                const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
                if (AtomicGmlId == NodeName)
                    continue;

                const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
                if (CityObject == nullptr)
                    continue;

                CityObjectsChildrenJsonArray.Append(GetCityObjectJsonValue(CityObject, CityObjectIndex));
            }
            JsonRootObject->SetArrayField("children", CityObjectsChildrenJsonArray);
        }
    }

	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    // const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
    UE_LOG(LogTemp, Log, TEXT("SerializedCityObjects: %s"), *SerializedCityObjects);
}
