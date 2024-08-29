// Copyright 2023 Ministry of Land, Infrastructure and Transport
#include "Component/PLATEAUCityObjectGroup.h"
#include "PLATEAUMeshExporter.h"
#include "PLATEAUCityModelLoader.h"
#include "CityGML/PLATEAUCityObject.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include <citygml/cityobject.h>
#include "Util/PLATEAUGmlUtil.h"

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

void UPLATEAUCityObjectGroup::SerializeCityObject(const FPLATEAUCityObject& InCityObject, const FString InOutsideParent, const TArray<FString> InOutsideChildren) {
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InCityObject, InOutsideParent, InOutsideChildren);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject, const plateau::granularityConvert::ConvertGranularity& Granularity) {
    SetConvertGranularity(Granularity);
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InNode, InCityObject);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity) {
    SetMeshGranularity(Granularity);
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InNode, InCityObject);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const FPLATEAUCityObject& InCityObject) {
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InNode, InCityObject);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const FString& InNodeName, const plateau::polygonMesh::Mesh& InMesh, 
    const plateau::granularityConvert::ConvertGranularity& Granularity, TMap<FString, FPLATEAUCityObject> CityObjMap) {
    SetConvertGranularity(Granularity);
    const plateau::polygonMesh::MeshGranularity MeshGranularity = (const plateau::polygonMesh::MeshGranularity)Granularity;
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InNodeName, InMesh, MeshGranularity, CityObjMap);
}
void UPLATEAUCityObjectGroup::SerializeCityObject(const FString& InNodeName, const plateau::polygonMesh::Mesh& InMesh,
    const plateau::polygonMesh::MeshGranularity& Granularity, TMap<FString, FPLATEAUCityObject> CityObjMap) {
    SetMeshGranularity(Granularity);
    SerializedCityObjects = PlateauSerializer.SerializeCityObject(InNodeName, InMesh, Granularity, CityObjMap);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const std::string& InNodeName, const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& InLoadInputData, const std::shared_ptr<const citygml::CityModel> InCityModel) {
    const plateau::polygonMesh::MeshGranularity& Granularity = InLoadInputData.ExtractOptions.mesh_granularity;
    SetMeshGranularity(Granularity);
    SerializedCityObjects = CityModelSerializer.SerializeCityObject(InNodeName, InMesh, Granularity, InCityModel);
}

void UPLATEAUCityObjectGroup::SerializeCityObject(const plateau::polygonMesh::Node& InNode, const citygml::CityObject* InCityObject, const plateau::polygonMesh::MeshGranularity& Granularity) {
    SetMeshGranularity(Granularity);
    SerializedCityObjects = CityModelSerializer.SerializeCityObject(InNode, InCityObject, Granularity);
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
    return GetCityObjectByIndex(FPLATEAUCityObjectIndex(static_cast<int32>(UV.X), static_cast<int32>(UV.Y)));
}

FPLATEAUCityObject UPLATEAUCityObjectGroup::GetCityObjectByIndex(const FPLATEAUCityObjectIndex Index) {
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

    Deserializer.DeserializeCityObjects(SerializedCityObjects, GetAttachChildren(), RootCityObjects, OutsideParent);
    return RootCityObjects;
}

const plateau::granularityConvert::ConvertGranularity UPLATEAUCityObjectGroup::GetConvertGranularity() {
    return static_cast<plateau::granularityConvert::ConvertGranularity>(MeshGranularityIntValue);
}

void UPLATEAUCityObjectGroup::SetConvertGranularity(const plateau::granularityConvert::ConvertGranularity Granularity) {
    MeshGranularityIntValue = (int)Granularity;
}

void UPLATEAUCityObjectGroup::SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity) {
    MeshGranularityIntValue = (int)Granularity;
}