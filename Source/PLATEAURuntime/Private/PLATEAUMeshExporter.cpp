// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUMeshExporter.h"
#include "plateau/mesh_writer/gltf_writer.h"
#include "plateau/mesh_writer/obj_writer.h"
#include "plateau/mesh_writer/fbx_writer.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUEditor/Private/SPLATEAUExportPanel.h"
#include "plateau/polygon_mesh/model.h"
#include "plateau/polygon_mesh/node.h"
#include "plateau/polygon_mesh/mesh.h"
#include "plateau/polygon_mesh/sub_mesh.h"
#include "MeshDescription/Public/MeshDescription.h"
#include "Runtime/StaticMeshDescription/Public/StaticMeshAttributes.h"
#include "MaterialTypes.h"
#include "Engine/Classes/Engine/Texture.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/MaterialInstance.h"
#include "Engine/Classes/Engine/StaticMesh.h"
#include "StaticMeshResources.h"
#include "UObject/UObjectBaseUtility.h"
#include "StaticMeshResources.h"
#include "HAL/FileManager.h"
#include "filesystem"
#include "EditorFramework/AssetImportData.h"
#include "HAL/FileManager.h"

#if WITH_EDITOR

void FPLATEAUMeshExporter::Export(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    ModelNames.Empty();
    TargetActor = ModelActor;
    switch (Option.FileFormat) {
    case EMeshFileFormat::OBJ:
        ExportAsOBJ(ExportPath, ModelActor, Option);
        break;
    case EMeshFileFormat::FBX:
        ExportAsFBX(ExportPath, ModelActor, Option);
        break;
    case EMeshFileFormat::GLTF:
        ExportAsGLTF(ExportPath, ModelActor, Option);
        break;
    default:
        break;
    }
}

void FPLATEAUMeshExporter::ExportAsOBJ(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    UE_LOG(LogTemp, Log, TEXT("Export as OBJ"));
    plateau::meshWriter::ObjWriter Writer;
    if (Option.TransformType == EMeshTransformType::PlaneRect) {
        ReferencePoint = ModelActor->GeoReference.ReferencePoint;
    } else {
        ReferencePoint = FVector::ZeroVector;
    }
    const auto ModelDataArray = CreateModelFromActor(ModelActor, Option);
    for (int i = 0; i < ModelDataArray.Num(); i++) {
        if (ModelDataArray[i]->getRootNodeCount() != 0) {
            const FString ExportPathWithName = ExportPath + "/" + ModelNames[i] + ".obj";
            Writer.write(TCHAR_TO_UTF8(*ExportPathWithName), *ModelDataArray[i]);
        }
    }
}

void FPLATEAUMeshExporter::ExportAsFBX(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    UE_LOG(LogTemp, Log, TEXT("Export as FBX"));
    plateau::meshWriter::FbxWriter Writer;
    if (Option.TransformType == EMeshTransformType::PlaneRect) {
        ReferencePoint = ModelActor->GeoReference.ReferencePoint;
    } else {
        ReferencePoint = FVector::ZeroVector;
    }
    const auto ModelDataArray = CreateModelFromActor(ModelActor, Option);
    for (int i = 0; i < ModelDataArray.Num(); i++) {
        if (ModelDataArray[i]->getRootNodeCount() != 0) {
            const FString ExportPathWithName = ExportPath + "/" + ModelNames[i] + ".fbx";
            Writer.write(TCHAR_TO_UTF8(*ExportPathWithName), *ModelDataArray[i], Option.FbxWriteOptions);
        }
    }
}

void FPLATEAUMeshExporter::ExportAsGLTF(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    UE_LOG(LogTemp, Log, TEXT("Export as GLTF"));
    plateau::meshWriter::GltfWriter Writer;
    const auto ModelDataArray = CreateModelFromActor(ModelActor, Option);
    for (int i = 0; i < ModelDataArray.Num(); i++) {
        if (ModelDataArray[i]->getRootNodeCount() != 0) {
            const FString ExportPathWithName = ExportPath + "/" + ModelNames[i] + "/" + ModelNames[i] + ".gltf";
            const FString ExportPathWithFolder = ExportPath + "/" + ModelNames[i];

            std::filesystem::create_directory(TCHAR_TO_UTF8(*ExportPathWithFolder));
            Writer.write(TCHAR_TO_UTF8(*ExportPathWithName), *ModelDataArray[i], Option.GltfWriteOptions);
        }
    }
}

TArray<std::shared_ptr<plateau::polygonMesh::Model>> FPLATEAUMeshExporter::CreateModelFromActor(APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    TArray<std::shared_ptr<plateau::polygonMesh::Model>> ModelArray;
    const auto RootComponent = ModelActor->GetRootComponent();
    const auto Components = RootComponent->GetAttachChildren();
    for (int i = 0; i < Components.Num(); i++) {
        //BillboardComponentなるコンポーネントがついていることがあるので無視
        if (Components[i]->GetName().Contains("BillboardComponent")) continue;

        ModelArray.Add(CreateModel(Components[i], Option));
        ModelNames.Add(RemoveSuffix(Components[i]->GetName()));
    }
    return ModelArray;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUMeshExporter::CreateModel(USceneComponent* ModelRootComponent, const MeshExportOptions Option) {
    auto OutModel = plateau::polygonMesh::Model::createModel();
    const auto Components = ModelRootComponent->GetAttachChildren();
    for (int i = 0; i < Components.Num(); i++) {
        auto& Node = OutModel->addEmptyNode(TCHAR_TO_UTF8(*RemoveSuffix(Components[i]->GetName())));
        CreateNode(Node, Components[i], Option);
    }
    return OutModel;
}

void FPLATEAUMeshExporter::CreateNode(plateau::polygonMesh::Node& OutNode, USceneComponent* NodeRootComponent, const MeshExportOptions Option) {
    for (const auto& Component : NodeRootComponent->GetAttachChildren()) {
        if (!Option.bExportHiddenObjects && !Component->IsVisible())
            continue;

        auto& Node = OutNode.addEmptyChildNode(TCHAR_TO_UTF8(*Component->GetName()));
        auto Mesh = plateau::polygonMesh::Mesh();
        CreateMesh(Mesh, Component, Option);
        auto MeshPtr = std::make_unique<plateau::polygonMesh::Mesh>(Mesh);
        Node.setMesh(std::move(MeshPtr));
    }
}

void FPLATEAUMeshExporter::CreateMesh(plateau::polygonMesh::Mesh& OutMesh, USceneComponent* MeshComponent, const MeshExportOptions Option) {
    const auto StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);

    if (StaticMeshComponent == nullptr || StaticMeshComponent->GetStaticMesh() == nullptr)
        return;

    //渡すためのデータ各種
    std::vector<TVec3d> Vertices;
    std::vector<unsigned int> OutIndices;
    plateau::polygonMesh::UV UV1;
    plateau::polygonMesh::UV UV2;
    plateau::polygonMesh::UV UV3;
    plateau::polygonMesh::UV UV4;

    const auto& RenderMesh = StaticMeshComponent->GetStaticMesh()->GetLODForExport(0);

    auto& InVertices = RenderMesh.VertexBuffers.StaticMeshVertexBuffer;
    for (uint32 i = 0; i < InVertices.GetNumVertices(); ++i) {
        const FVector2f& UV = InVertices.GetVertexUV(i, 0);
        UV1.push_back(TVec2f(UV.X, 1.0f - UV.Y));
    }

    //UV4
    for (uint32 i = 0; i < InVertices.GetNumVertices(); ++i) {
        const FVector2f& UV = InVertices.GetVertexUV(i, 3);
        UV4.push_back(TVec2f(UV.X, 1.0f - UV.Y));
    }

    auto GeoRef = TargetActor->GeoReference.GetData();
    for (uint32 i = 0; i < RenderMesh.VertexBuffers.PositionVertexBuffer.GetNumVertices(); i++) {
        const auto VertexPosition = RenderMesh.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
        TVec3d Vertex;
        if (Option.TransformType == EMeshTransformType::PlaneRect)
            Vertex = TVec3d(VertexPosition.X + ReferencePoint.X, VertexPosition.Y + ReferencePoint.Y, VertexPosition.Z + ReferencePoint.Z);
        else
            Vertex = TVec3d(VertexPosition.X, VertexPosition.Y, VertexPosition.Z);
        Vertex = GeoRef.convertAxisToENU(plateau::geometry::CoordinateSystem::ESU, Vertex);
        Vertex = GeoRef.convertAxisFromENUTo(StaticCast<plateau::geometry::CoordinateSystem>(Option.CoordinateSystem), Vertex);
        Vertices.push_back(Vertex);
    }

    for (int32 TriangleIndex = 0; TriangleIndex < RenderMesh.IndexBuffer.GetNumIndices() / 3; ++TriangleIndex) {
        OutIndices.push_back(RenderMesh.IndexBuffer.GetIndex(TriangleIndex * 3));
        OutIndices.push_back(RenderMesh.IndexBuffer.GetIndex(TriangleIndex * 3 + 1));
        OutIndices.push_back(RenderMesh.IndexBuffer.GetIndex(TriangleIndex * 3 + 2));
    }

    for (int k = 0; k < RenderMesh.Sections.Num(); k++) {
        const auto& Section = RenderMesh.Sections[k];
        if (Section.NumTriangles <= 0) continue;

        //サブメッシュの開始・終了インデックス計算
        const int FirstIndex = Section.FirstIndex;
        const int EndIndex = Section.FirstIndex + Section.NumTriangles * 3 - 1;
        ensureAlwaysMsgf((EndIndex - FirstIndex + 1) % 3 == 0, TEXT("SubMesh indices size should be multiple of 3."));

        //マテリアルがテクスチャを持っているようなら取得、設定によってはスキップ
        FString TextureFilePath = FString("");

        if (Option.bExportTexture) {
            const auto  MaterialInstance = (UMaterialInstance*)StaticMeshComponent->GetMaterial(k);
            if (MaterialInstance->TextureParameterValues.Num() > 0) {
                FMaterialParameterMetadata MetaData;
                MaterialInstance->TextureParameterValues[0].GetValue(MetaData);
                if (const auto Texture = MetaData.Value.Texture; Texture != nullptr) {
                    const auto TextureSourceFiles = Texture->AssetImportData->GetSourceData().SourceFiles;
                    if (TextureSourceFiles.Num() == 0) {
                        UE_LOG(LogTemp, Error, TEXT("SourceFilePath is missing in AssetImportData: %s"), *Texture->GetName());
                        OutMesh.addSubMesh("",nullptr, FirstIndex, EndIndex);

                        // TODO マテリアル対応、下のnullptrをマテリアルに置き換える
                        OutMesh.addSubMesh("", nullptr, FirstIndex, EndIndex);
                        continue;
                    }

                    const auto BaseDir = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*(FPaths::ProjectContentDir() + "PLATEAU/"));
                    const auto AssetBasePath = FPaths::GetPath(Texture->GetPackage()->GetLoadedPath().GetLocalFullPath());
                    const auto TextureFileRelativePath = TextureSourceFiles[0].RelativeFilename;
                    TextureFilePath = AssetBasePath / TextureFileRelativePath;

                    //TextureFilePath = TextureFileRelativePath.Contains(":")? TextureFileRelativePath :  AssetBasePath / TextureFileRelativePath;
                    
                    UE_LOG(LogTemp, Log, TEXT("TexPath: %s : %s : %s : %s"), *BaseDir , *AssetBasePath, *TextureFileRelativePath, *TextureFilePath);
                }
            }
        }
        std::string TextureFilePathStr = TCHAR_TO_UTF8(*TextureFilePath);

        // TODO マテリアル対応、下のnullptrをマテリアルに置き換える
        OutMesh.addSubMesh(TextureFilePathStr, nullptr, FirstIndex, EndIndex);
    }

    OutMesh.addVerticesList(Vertices);

    OutMesh.addIndicesList(OutIndices, 0, false);
    OutMesh.addUV1(UV1, Vertices.size());
    OutMesh.addUV4(UV4, Vertices.size());
    ensureAlwaysMsgf(OutMesh.getIndices().size() % 3 == 0, TEXT("Indice size should be multiple of 3."));
    ensureAlwaysMsgf(OutMesh.getVertices().size() == OutMesh.getUV1().size(), TEXT("Size of vertices and uv1 should be same."));
}

FString FPLATEAUMeshExporter::RemoveSuffix(const FString ComponentName) {
    int Index = 0;
    if (ComponentName.FindLastChar('_', Index)) {
        if (ComponentName.RightChop(Index + 1).IsNumeric()) {
            return ComponentName.LeftChop(ComponentName.Len() - Index);
        } else {
            return ComponentName;
        }
    } else
        return ComponentName;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUMeshExporter::CreateModelFromLODComponents(APLATEAUInstancedCityModel* ModelActor, const TArray<USceneComponent*> ModelComponents, const MeshExportOptions Option)     {

    TargetActor = ModelActor;
    auto OutModel = plateau::polygonMesh::Model::createModel();
    TMap<FString, plateau::polygonMesh::Node*> ParentNodeMap;

    for (const auto comp : ModelComponents) {
    
        FString ParentName = comp->GetAttachParent()->GetName();
        auto ParentRef = ParentNodeMap.Find(ParentName);
        plateau::polygonMesh::Node* Parent;

        if (ParentRef == nullptr) {
            Parent = &OutModel->addEmptyNode(TCHAR_TO_UTF8(*ParentName));
            ParentNodeMap.Add(ParentName, Parent);
        }
        else {
            Parent = *ParentRef;
        }
            
        auto& Node = Parent->addEmptyChildNode(TCHAR_TO_UTF8(*comp->GetName()));
        auto Mesh = plateau::polygonMesh::Mesh();
        CreateMesh(Mesh, comp, Option);
        auto MeshPtr = std::make_unique<plateau::polygonMesh::Mesh>(Mesh);
        Node.setMesh(std::move(MeshPtr));
    }

    return OutModel;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUMeshExporter::CreateModelFromFeatureComponents(APLATEAUInstancedCityModel* ModelActor, const TArray<USceneComponent*> ModelComponents, const MeshExportOptions Option) {

    TargetActor = ModelActor;
    auto OutModel = plateau::polygonMesh::Model::createModel();
    TMap<FString, TArray<plateau::polygonMesh::Node*>> RootLodMap;

    for (const auto comp : ModelComponents) {

        plateau::polygonMesh::Node* Root;
        plateau::polygonMesh::Node* Parent;
        TArray<plateau::polygonMesh::Node*> lodArray;

        FString RootName = comp->GetAttachParent()->GetAttachParent()->GetName();   //Root
        FString ParentName = comp->GetAttachParent()->GetName();                    //LOD

        auto RootRef = RootLodMap.Find(RootName);
        if (RootRef == nullptr) {
            Root = &OutModel->addEmptyNode(TCHAR_TO_UTF8(*RootName));
            lodArray.Add(Root);     //先頭にRootを入れる
            RootLodMap.Add(RootName, lodArray);
        }
        else {
            lodArray = *RootRef;
            Root = lodArray[0];
        }

        auto ParentRef = lodArray.FindByPredicate([&ParentName](const auto& lodNode) {
            return lodNode->getName() == TCHAR_TO_UTF8(*ParentName);
            });      
        if (ParentRef == nullptr) {
            Parent = &Root->addEmptyChildNode(TCHAR_TO_UTF8(*ParentName));
            lodArray.Add(Parent);
        }
        else {
            Parent = *ParentRef;
        }

        auto& Node = Parent->addEmptyChildNode(TCHAR_TO_UTF8(*comp->GetName()));
        auto Mesh = plateau::polygonMesh::Mesh();
        CreateMesh(Mesh, comp, Option);
        auto MeshPtr = std::make_unique<plateau::polygonMesh::Mesh>(Mesh);
        Node.setMesh(std::move(MeshPtr));
    }

    return OutModel;
}


#endif
