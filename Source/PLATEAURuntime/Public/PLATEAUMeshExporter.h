#pragma once

#include "CoreMinimal.h"

#include "plateau/mesh_writer/gltf_writer.h"

enum class EMeshTransformType : uint8_t;
enum class EMeshFileFormat : uint8_t;

class APLATEAUInstancedCityModel;

struct MeshExportOptions {
    EMeshTransformType TransformType;
    bool bExportHiddenObjects;
    bool bExportTexture;
    EMeshFileFormat FileFormat;
    plateau::meshWriter::GltfWriteOptions GltfWriteOptions;
};

namespace plateau {
    namespace polygonMesh {
        class Model;
    }
}

class PLATEAURUNTIME_API FPLATEAUMeshExporter {
public:
    void Export(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option);

private:
    void ExportAsOBJ(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option);
    void ExportAsFBX(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option);
    void ExportAsGLTF(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option);
    TArray<plateau::polygonMesh::Model> CreateModelFromActor(APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option);
    plateau::polygonMesh::Model CreateModel(USceneComponent* ModelRootComponent, const MeshExportOptions Option);
    plateau::polygonMesh::Node CreateNode(USceneComponent* NodeRootComponent, const MeshExportOptions Option);
    plateau::polygonMesh::Mesh CreateMesh(USceneComponent* MeshComponent, const MeshExportOptions Option);
    FString RemoveSuffix(const FString ComponentName);
};
