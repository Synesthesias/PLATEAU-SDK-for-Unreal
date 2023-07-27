// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroup.h"
#include "JsonObjectConverter.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
#include "CityGML/PLATEAUCityObject.h"
#include <citygml/cityobject.h>


namespace {
    constexpr TCHAR GmlIdFieldName[]            = TEXT("gmlID");
    constexpr TCHAR CityObjectIndexFieldName[]  = TEXT("cityObjectIndex");
    constexpr TCHAR CityObjectTypeFieldName[]   = TEXT("cityObjectType");
    constexpr TCHAR AttributesFieldName[]       = TEXT("attributes");
    constexpr TCHAR KeyFieldName[]              = TEXT("key");
    constexpr TCHAR TypeFieldName[]             = TEXT("type");
    constexpr TCHAR ValueFieldName[]            = TEXT("value");
    constexpr TCHAR OutsideParentFieldName[]    = TEXT("outsideParent");
    constexpr TCHAR OutsideChildrenFieldName[]  = TEXT("outsideChildren");
    constexpr TCHAR CityObjectsFieldName[]      = TEXT("cityObjects");
    constexpr TCHAR ChildrenFieldName[]         = TEXT("children");

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
                AttributesJsonObject->SetStringField(KeyFieldName, UTF8_TO_TCHAR(key.c_str()));
                AttributesJsonObject->SetStringField(TypeFieldName, "AttributeSets");
                AttributesJsonObject->SetArrayField(ValueFieldName, AttributeSetJsonObjectArray);
                InAttributesJsonObjectArray.Emplace(MakeShared<FJsonValueObject>(AttributesJsonObject));
            } else {
                AttributesJsonObject->SetStringField(KeyFieldName, UTF8_TO_TCHAR(key.c_str()));

                switch (value.getType()) {
                case citygml::AttributeType::String:
                    AttributesJsonObject->SetStringField(TypeFieldName, "String");
                    break;
                case citygml::AttributeType::Double:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Double");
                    break;
                case citygml::AttributeType::Integer:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Integer");
                    break;
                case citygml::AttributeType::Date:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Date");
                    break;
                case citygml::AttributeType::Uri:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Uri");
                    break;
                case citygml::AttributeType::Measure:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Measure");
                    break;
                case citygml::AttributeType::Boolean:
                    AttributesJsonObject->SetStringField(TypeFieldName, "Boolean");
                    break;
                default: UE_LOG(LogTemp, Log, TEXT("Error citygml::AttributeType"));
                }

                AttributesJsonObject->SetStringField(ValueFieldName, UTF8_TO_TCHAR(value.asString().c_str()));
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
        
        CityJsonObject->SetStringField(GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(0));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(-1));
        CityJsonObject->SetArrayField(CityObjectIndexFieldName, CityObjectIndexArray);
        
        CityJsonObject->SetNumberField(CityObjectTypeFieldName, static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(AttributesFieldName, AttributesJsonObjectArray);

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
        
        CityJsonObject->SetStringField(GmlIdFieldName, UTF8_TO_TCHAR(InCityObject->getId().c_str()));

        TArray<TSharedPtr<FJsonValue>> CityObjectIndexArray;
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.primary_index));
        CityObjectIndexArray.Emplace(MakeShared<FJsonValueNumber>(CityObjectIndex.atomic_index));
        CityJsonObject->SetArrayField(CityObjectIndexFieldName, CityObjectIndexArray);
        
        CityJsonObject->SetNumberField(CityObjectTypeFieldName, static_cast<int64>(InCityObject->getType()));

        TArray<TSharedPtr<FJsonValue>> AttributesJsonObjectArray;
        GetAttributesJsonObjectRecursive(InCityObject->getAttributes(), AttributesJsonObjectArray);
        CityJsonObject->SetArrayField(AttributesFieldName, AttributesJsonObjectArray);

        CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObject));
        return CityObjectsJsonArray;
    }
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject) {
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);

    // 親はなし
    JsonRootObject->SetStringField(OutsideParentFieldName, "");

    // 子コンポーネント名取得
    TArray<TSharedPtr<FJsonValue>> OutsideChildrenJsonArray;
    for (int i = 0; i < InNode.getChildCount(); i++) {
        OutsideChildrenJsonArray.Emplace(MakeShared<FJsonValueString>(UTF8_TO_TCHAR(InNode.getChildAt(i).getName().c_str())));
    }
    JsonRootObject->SetArrayField(OutsideChildrenFieldName, OutsideChildrenJsonArray);

    // CityObjects取得
    JsonRootObject->SetArrayField(CityObjectsFieldName, GetCityObjectJsonValue(InCityObject));

    // Json書き出し
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh,
                                                  const FLoadInputData& InLoadInputData, const std::shared_ptr<const citygml::CityModel> InCityModel) {
    const auto& CityObjectList = InMesh.getCityObjectList();
    const std::vector<plateau::polygonMesh::CityObjectIndex> CityObjectIndices = *CityObjectList.getAllKeys();
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);
    JsonRootObject->SetStringField(OutsideParentFieldName, "");
    JsonRootObject->SetArrayField(OutsideChildrenFieldName, {});

    // 最小地物単位の親を求める（主要地物のIDを設定）
    if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity && AtomicGmlId != InNodeName) {
                JsonRootObject->SetStringField(OutsideParentFieldName, UTF8_TO_TCHAR(AtomicGmlId.c_str()));
            }
        }
    }

    // 地域単位
    if (plateau::polygonMesh::MeshGranularity::PerCityModelArea == InLoadInputData.ExtractOptions.mesh_granularity) {
        TArray<TSharedPtr<FJsonValue>> CityObjectsChildrenJsonArray;
		const TArray<TSharedPtr<FJsonValue>>* TempJsonArray;
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
            if (CityObject == nullptr)
                continue;

            if (JsonRootObject->TryGetArrayField(CityObjectsFieldName, TempJsonArray)) {
                CityObjectsChildrenJsonArray.Append(GetCityObjectJsonValue(CityObject, CityObjectIndex));
            } else {
                JsonRootObject->SetArrayField(CityObjectsFieldName, GetCityObjectJsonValue(CityObject, CityObjectIndex));
            }
        }
        JsonRootObject->SetArrayField(ChildrenFieldName, CityObjectsChildrenJsonArray);
    } else {
        // 最小値物単位・主要地物単位共通
        if (const auto& CityObject = InCityModel->getCityObjectById(InNodeName); CityObject != nullptr) {
            const auto& CityObjectIndex =  CityObjectList.getCityObjectIndex(InNodeName);         
            JsonRootObject->SetArrayField(CityObjectsFieldName, GetCityObjectJsonValue(CityObject, CityObjectIndex));
        }

        // 主要地物単位
        if (plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject == InLoadInputData.ExtractOptions.mesh_granularity) {
            TArray<TSharedPtr<FJsonValue>> CityObjectsChildrenJsonArray;
            for (const auto& CityObjectIndex : CityObjectIndices) {
                const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
                if (AtomicGmlId == InNodeName)
                    // 前の処理で既に情報抽出済み
                    continue;

                const auto& CityObject = InCityModel->getCityObjectById(AtomicGmlId);
                if (CityObject == nullptr)
                    continue;

                CityObjectsChildrenJsonArray.Append(GetCityObjectJsonValue(CityObject, CityObjectIndex));
            }
            JsonRootObject->SetArrayField(ChildrenFieldName, CityObjectsChildrenJsonArray);
        }
    }

	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
}

TArray<FPLATEAUCityObject> UPLATEAUCityObjectGroup::GetAllRootCityObjects() {
    if (0 < RootCityObjects.Num()) {
        return RootCityObjects;
    }
    
    TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(SerializedCityObjects);
    TSharedPtr<FJsonObject> JsonRootObject;
    FJsonSerializer::Deserialize(JsonReader, JsonRootObject);
    
    const auto& CityObjectsJsonArray = JsonRootObject->GetArrayField(CityObjectsFieldName);
    for (const auto& CityJsonValue : CityObjectsJsonArray) {
        FPLATEAUCityObject CityObject;
        const auto& CityJsonObject = CityJsonValue->AsObject();
        CityObject.SetGmlID(CityJsonObject->GetStringField(GmlIdFieldName));
        RootCityObjects.Add(CityObject);
    }

    return RootCityObjects;
}
