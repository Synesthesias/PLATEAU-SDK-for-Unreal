// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroup.h"
#include "JsonObjectConverter.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
#include "CityGML/PLATEAUCityObject.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsSettings.h"
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

        CityJsonObject->SetNumberField(plateau::CityObject::CityObjectTypeFieldName, static_cast<double>(InCityObject->getType()));

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

        CityJsonObject->SetNumberField(plateau::CityObject::CityObjectTypeFieldName, static_cast<double>(InCityObject->getType()));

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

        CityObject.SetCityObjectsType(static_cast<citygml::CityObject::CityObjectsType>(CityJsonObject->GetNumberField(plateau::CityObject::CityObjectTypeFieldName)));

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

void UPLATEAUCityObjectGroup::FindCollisionUV(const FHitResult& HitResult, FVector2D& UV, const int32 UVChannel) {
    if (!UPhysicsSettings::Get()->bSupportUVFromHitResults) {
        UE_LOG(LogTemp, Warning, TEXT("Calling FindCollisionUV but 'Support UV From Hit Results' is not enabled in project settings. This is required for finding UV for collision results."));
        return;
    }

    const auto& HitPrimitiveComponent = HitResult.Component.Get();
    if (HitPrimitiveComponent == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("HitPrimitiveComponent == nullptr"));
        return;
    }

    const auto& StaticMeshComponent = Cast<UStaticMeshComponent>(HitPrimitiveComponent);
    if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("StaticMeshComponent == nullptr or StaticMeshComponent->GetStaticMesh() == nullptr"));
        return;
    }

    const auto& BodySetup = HitPrimitiveComponent->GetBodySetup();
    if (BodySetup == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("BodySetup == nullptr"));
        return;
    }

    const auto& [IndexBuffer, VertPositions, VertUVs] = BodySetup->UVInfo;
    if (VertUVs.IsValidIndex(UVChannel) && IndexBuffer.IsValidIndex(HitResult.FaceIndex * 3 + 2)) {
        const int32 Index0 = IndexBuffer[HitResult.FaceIndex * 3 + 0];
        const int32 Index1 = IndexBuffer[HitResult.FaceIndex * 3 + 1];
        const int32 Index2 = IndexBuffer[HitResult.FaceIndex * 3 + 2];

        const FVector Pos0 = VertPositions[Index0];
        const FVector Pos1 = VertPositions[Index1];
        const FVector Pos2 = VertPositions[Index2];

        const FVector2D UV0 = VertUVs[UVChannel][Index0];
        const FVector2D UV1 = VertUVs[UVChannel][Index1];
        const FVector2D UV2 = VertUVs[UVChannel][Index2];

        // Find barycentric coords
        // 第一引数に自身の頂点を与えることで必ず同じBaryCoordsが得られるようにしている（FaceIndexによってのみUVが変化する）
        const FVector BaryCoords = FMath::ComputeBaryCentric2D(Pos0, Pos0, Pos1, Pos2);
        // Use to blend UVs
        UV = BaryCoords.X * UV0 + BaryCoords.Y * UV1 + BaryCoords.Z * UV2;
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
        // 地域単位
        TArray<TSharedPtr<FJsonValue>> CityObjectJsonArray;
        TSharedPtr<FJsonObject> CityJsonObjectParent = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> CityJsonObjectChildren;
        int CurrentPrimaryIndex = -1;
        for (int i = 0; i < CityObjectIndices.size(); i++) {
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
            } else {
                // 最小地物
                CityJsonObjectChildren.Emplace(MakeShared<FJsonValueObject>(GetCityJsonObject(CityObject, CityObjectIndex)));
            }

            // データの区切りか？
            if (i + 1 == CityObjectIndices.size() || CityObjectIndices[i + 1].primary_index != CurrentPrimaryIndex) {
                CityJsonObjectParent->SetArrayField(plateau::CityObject::ChildrenFieldName, CityJsonObjectChildren);
            }
        }
        JsonRootObject->SetArrayField(plateau::CityObject::CityObjectsFieldName, CityObjectJsonArray);
    } else {
        // 最小地物単位・主要地物単位共通
        if (const auto& CityObjectParent = InCityModel->getCityObjectById(InNodeName); CityObjectParent != nullptr) {
            const auto& CityObjectParentIndex = CityObjectList.getCityObjectIndex(InNodeName);
            const auto& CityJsonObjectParent = GetCityJsonObject(CityObjectParent, CityObjectParentIndex);
            TArray<TSharedPtr<FJsonValue>> CityObjectJsonArray;
            CityObjectJsonArray.Emplace(MakeShared<FJsonValueObject>(CityJsonObjectParent));

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
            JsonRootObject->SetArrayField(plateau::CityObject::CityObjectsFieldName, CityObjectJsonArray);
        }
    }

    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&SerializedCityObjects);
    FJsonSerializer::Serialize(JsonRootObject.ToSharedRef(), Writer);
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetPrimaryCityObjectByRaycast(const FHitResult& HitResult) {
    if (RootCityObjects.Num() <= 0) {
        GetAllRootCityObjects();
    }

    if (OutsideParent.IsEmpty()) {
        FVector2d UV;
        FindCollisionUV(HitResult, UV);
        UV.Y = -1;
        return GetCityObjectByUV(UV);
    }

    // 親を探す
    USceneComponent* ParentIterator = GetAttachParent();
    while (ParentIterator != nullptr) {
        if (const auto& Parent = Cast<UPLATEAUCityObjectGroup>(ParentIterator); Parent->GetName().Contains(OutsideParent)) {
            return Parent->GetCityObjectByID(OutsideParent);
        }
        ParentIterator = ParentIterator->GetAttachParent();
    }

    UE_LOG(LogTemp, Error, TEXT("There is no %s."), *OutsideParent);
    return FPLATEAUCityObject();
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetAtomicCityObjectByRaycast(const FHitResult& HitResult) {
    FVector2d UV;
    FindCollisionUV(HitResult, UV);
    return GetCityObjectByUV(UV);
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetCityObjectByUV(const FVector2d& UV) {
    return GetCityObjectByIndex(FPLATEAUCityObjectIndex(static_cast<int>(UV.X), static_cast<int>(UV.Y)));
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetCityObjectByIndex(FPLATEAUCityObjectIndex Index) {
    if (RootCityObjects.Num() <= 0) {
        GetAllRootCityObjects();
    }

    for (const auto& RootCityObject : RootCityObjects) {
        if (RootCityObject.CityObjectIndex == Index) {
            return RootCityObject;
        }

        for (const auto& ChildCityObject : RootCityObject.Children) {
            if (ChildCityObject.CityObjectIndex == Index) {
                return ChildCityObject;
            }
        }
    }

    UE_LOG(LogTemp, Error, TEXT("There is no index (%d, %d)."), Index.PrimaryIndex, Index.AtomicIndex);
    return FPLATEAUCityObject();
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

    OutsideParent = JsonRootObject->GetStringField(plateau::CityObject::OutsideParentFieldName);

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
