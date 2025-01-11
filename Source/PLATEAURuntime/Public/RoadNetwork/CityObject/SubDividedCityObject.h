#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <optional>
#include <plateau/polygon_mesh/model.h>

#include "CityGML/PLATEAUCityObject.h"
#include "RoadNetwork/RGraph/RGraphDef.h"
class UPLATEAUCityObjectGroup;
class RnLineString;
class RnPoint;
struct FAttributeDataHelper;
class FSubDividedCityObject
{
public:
    class FSubMesh {
    public:
        TSharedPtr<TArray<int32>> Triangles;

        TArray<FSubMesh> Separate() const;
        TArray<TArray<int32>> CreateOutlineIndices() const;
        FSubMesh DeepCopy() const;
    };

    class FMesh {
    public:
        TSharedPtr<TArray<FVector>> Vertices;
        TSharedPtr<TArray<FSubMesh>> SubMeshes;

        void VertexReduction();
        void Separate();
        FMesh DeepCopy() const;
    };

    FString Name;
    FString SerializedCityObjects;
    bool bVisible;
    FPLATEAUCityObject CityObject;
    TArray<FMesh> Meshes;
    TArray<FSubDividedCityObject> Children;
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
    ERRoadTypeMask SelfRoadType;
    ERRoadTypeMask ParentRoadType;

    ERRoadTypeMask GetRoadType(bool ContainsParent) const;
    FSubDividedCityObject(const plateau::polygonMesh::Model& PlateauModel, TMap<FString, FPLATEAUCityObject>& CityObj );
    TArray<const FSubDividedCityObject*> GetAllChildren() const;
    void SetCityObjectGroup(UPLATEAUCityObjectGroup* Group);
    TSharedPtr<FSubDividedCityObject> DeepCopy();

public:
    FSubDividedCityObject(){}
    FSubDividedCityObject(const plateau::polygonMesh::Node& PlateauNode, TMap<FString, FPLATEAUCityObject>& CityObj, ERRoadTypeMask ParentTypeMask);
};
