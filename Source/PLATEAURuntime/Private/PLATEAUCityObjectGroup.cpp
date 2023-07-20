// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroup.h"

#include "JsonObjectConverter.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObject.h"
#include "PLATEAUMeshLoader.h"
#include "CityGML/PLATEAUCityObject.h"
#include "UObject/UnrealTypePrivate.h"


struct FAttribute {
    FString Key;
    FString Type;
    FString Value;
};

// class FCityObjects {
//     FString gmlID;
//     TArray<int> cityObjectIndex;
//     FString cityObjectType;
//     TArray<FAttribute> attributes;
// };

void UPLATEAUCityObjectGroup::SetNodeName(const std::string& InNodeName) {
    NodeName = InNodeName;
}

void UPLATEAUCityObjectGroup::InitializeSerializedCityObjects(const plateau::polygonMesh::Mesh& Mesh, const FLoadInputData& LoadInputData,
                                                              const std::shared_ptr<const citygml::CityModel> CityModel) {
    const auto& CityObjectList = Mesh.getCityObjectList();
    const std::vector<plateau::polygonMesh::CityObjectIndex> CityObjectIndices = *CityObjectList.getAllKeys();
    const TSharedPtr<FJsonObject> JsonRootObject = MakeShareable(new FJsonObject);
    JsonRootObject->SetStringField("parent", "");

    // 最小地物単位の親を求める（主要地物のIDを設定）
    if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == LoadInputData.ExtractOptions.mesh_granularity) {
        for (const auto& CityObjectIndex : CityObjectIndices) {
            const auto& AtomicGmlId = CityObjectList.getAtomicGmlID(CityObjectIndex);
            if (plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject == LoadInputData.ExtractOptions.mesh_granularity && AtomicGmlId != NodeName) {
                JsonRootObject->SetStringField("parent", UTF8_TO_TCHAR(AtomicGmlId.c_str()));
            }
        }
    }

    if (plateau::polygonMesh::MeshGranularity::PerCityModelArea == LoadInputData.ExtractOptions.mesh_granularity) {
    } else {
        if (const auto& CityObject = CityModel->getCityObjectById(NodeName); CityObject != nullptr) {
            const auto& CityObjectIndex =  CityObjectList.getCityObjectIndex(NodeName);         
                
            FPLATEAUCityJsonObject PLATEAUCityJsonObject;
            PLATEAUCityJsonObject.GmlId = UTF8_TO_TCHAR(CityObject->getId().c_str());
            PLATEAUCityJsonObject.CityObjectIndex = TArray{CityObjectIndex.primary_index, CityObjectIndex.atomic_index};
            PLATEAUCityJsonObject.CityObjectType = static_cast<int64>(CityObject->getType());
            // const auto& AttributesMap = CityObject->getAttributes();
            TSharedPtr<FJsonObject> CityJsonObject = FJsonObjectConverter::UStructToJsonObject(PLATEAUCityJsonObject);

            TArray<TSharedPtr<FJsonValue>> CityObjectsJsonArray;
            CityObjectsJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObject));
            JsonRootObject->SetArrayField("cityObjects", CityObjectsJsonArray);
        }
    }

    const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
    UE_LOG(LogTemp, Log, TEXT("SerializedCityObjects: %s"), *SerializedCityObjects);
}
