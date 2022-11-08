#include "PLATEAUMeshExporter.h"
#include "plateau/mesh_writer/gltf_writer.h"
#include "plateau/mesh_writer/obj_writer.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUEditor/Private/SPLATEAUExportPanel.h"
#include "plateau/polygon_mesh/model.h"
#include "plateau/polygon_mesh/node.h"
#include "plateau/polygon_mesh/mesh.h"
#include "plateau/polygon_mesh/sub_mesh.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "UObject/UObjectBaseUtility.h"

void FPLATEAUMeshExporter::Export(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
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
    const auto ModelDataArray = CreateModelFromActor(ModelActor, Option);
    for (int i = 0; i < ModelDataArray.Num(); i++) {
        if (ModelDataArray[i].getRootNodeCount() != 0) {
            //writeでコケる
            //Writer.write(TCHAR_TO_UTF8(*ExportPath), ModelDataArray[i]);
        }
    }
}

void FPLATEAUMeshExporter::ExportAsFBX(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    //TODO : FBX対応
    UE_LOG(LogTemp, Warning, TEXT("Export as FBX is temporarily unavailable"));
}

void FPLATEAUMeshExporter::ExportAsGLTF(const FString ExportPath, APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
}

TArray<plateau::polygonMesh::Model> FPLATEAUMeshExporter::CreateModelFromActor(APLATEAUInstancedCityModel* ModelActor, const MeshExportOptions Option) {
    TArray<plateau::polygonMesh::Model> ModelArray;
    const auto RootComponent = ModelActor->GetRootComponent();
    const auto Components = RootComponent->GetAttachChildren();
    UE_LOG(LogTemp, Log, TEXT("Children Num : %d"), Components.Num());
    for (int i = 0; i < Components.Num(); i++) {
        UE_LOG(LogTemp, Log, TEXT("Children Name : %s"), *Components[i]->GetName());

        //BillboardComponentなるコンポーネントがついていることがあるので無視
        if (!Components[i]->GetName().Contains("BillboardComponent")) {
            ModelArray.Add(CreateModel(Components[i], Option));
        }
    }
    return ModelArray;
}

plateau::polygonMesh::Model FPLATEAUMeshExporter::CreateModel(USceneComponent* ModelRootComponent, const MeshExportOptions Option) {
    plateau::polygonMesh::Model ModelData;
    const auto Components = ModelRootComponent->GetAttachChildren();
    UE_LOG(LogTemp, Log, TEXT("Children Num : %d"), Components.Num());
    for (int i = 0; i < Components.Num(); i++) {
        ModelData.addNode(CreateNode(Components[i], Option));
    }
    return ModelData;
}

plateau::polygonMesh::Node FPLATEAUMeshExporter::CreateNode(USceneComponent* NodeRootComponent, const MeshExportOptions Option) {
    plateau::polygonMesh::Node NodeData(TCHAR_TO_UTF8(*RemoveSuffix(NodeRootComponent->GetName())));
    UE_LOG(LogTemp, Log, TEXT("Node Name : %s"), *FString(RemoveSuffix(NodeRootComponent->GetName())));
    const auto Components = NodeRootComponent->GetAttachChildren();
    for (int i = 0; i < Components.Num(); i++) {
        if (!Option.bExportHiddenObjects) {
            if (!Components[i]->IsVisible()) {
                continue;
            }
        }
        NodeData.setMesh(CreateMesh(Components[i], Option));
    }
    return NodeData;
}

plateau::polygonMesh::Mesh FPLATEAUMeshExporter::CreateMesh(USceneComponent* MeshComponent, const MeshExportOptions Option) {
    plateau::polygonMesh::Mesh MeshData(TCHAR_TO_UTF8(*RemoveSuffix(MeshComponent->GetName())));
    UE_LOG(LogTemp, Log, TEXT("Mesh Name : %s"), *FString(RemoveSuffix(MeshComponent->GetName())));

    //StaticMeshComponentにキャスト
    UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent);
    FStaticMeshAttributes Attributes(*StaticMeshComponent->GetStaticMesh()->GetMeshDescription(0));

    //渡すためのデータ各種
    std::vector<TVec3d> Vertices;
    std::vector<unsigned int> OutIndices;
    plateau::polygonMesh::UV UV1;
    plateau::polygonMesh::UV UV2;
    plateau::polygonMesh::UV UV3;
    TArray<plateau::polygonMesh::SubMesh> SubMeshArray;
    const auto VertexPositions = Attributes.GetVertexPositions();
    const auto UVs = Attributes.GetVertexInstanceUVs();
    const auto Indices = Attributes.GetVertexInstanceVertexIndices();
    const auto SubMeshCount = StaticMeshComponent->GetStaticMesh()->GetMeshDescription(0)->PolygonGroups().Num();

    for (int i = 0; i < VertexPositions.GetNumElements(); i++) {
        const auto VertexPosition = VertexPositions.Get(i, 0);
        const TVec3d Vertex(VertexPosition.X, VertexPosition.Y, VertexPosition.Z);
        Vertices.push_back(Vertex);
        OutIndices.push_back(i);
    }
    for (int j = 0; j < UVs.GetNumElements(); j++) {
        //TODO: UV2、UV3対応
        UV1.push_back(TVec2f(UVs.Get(j, 0).X, UVs.Get(j, 0).Y));
    }

    int SubMeshIndex = 0;
    for (int k = 0; k < SubMeshCount; k++) {
        //サブメッシュの開始・終了インデックス計算
        const auto SubMeshPolygonNum = StaticMeshComponent->GetStaticMesh()->GetMeshDescription(0)->GetPolygonGroupPolygons(FPolygonGroupID(k)).Num();
        const int SubMeshEndIndex = FMath::Min(SubMeshIndex - 1 + ((int)(SubMeshPolygonNum * 3)), (int)Vertices.size() - 1);

        //マテリアルがテクスチャを持っているようなら取得、設定によってはスキップ
        if (Option.bExportTexture) {
            FString PathName;
            const auto  MaterialInstance = (UMaterialInstance*)StaticMeshComponent->GetMaterial(k);
            if (MaterialInstance->TextureParameterValues.Num() > 0) {
                FMaterialParameterMetadata MetaData;
                MaterialInstance->TextureParameterValues[0].GetValue(MetaData);
                PathName = (FPaths::ProjectContentDir() + "PLATEAU/" + MetaData.Value.Texture->GetName()).Replace(TEXT("/"), TEXT("\\"));
                UE_LOG(LogTemp, Log, TEXT("Texture Name : %s"), *PathName);
            }
        }

        //SubMeshDataにテクスチャパスを渡すとコケる
        //plateau::polygonMesh::SubMesh SubMeshData(SubMeshIndex, SubMeshEndIndex, TCHAR_TO_UTF8(*PathName));
        UE_LOG(LogTemp, Log, TEXT("SubMesh Start : %d, SubMesh End : %d"), SubMeshIndex, SubMeshEndIndex);

        SubMeshIndex = SubMeshEndIndex + 1;
    }

    MeshData.addVerticesList(Vertices);

    //addIndicesListで止まる
    //MeshData.addIndicesList(OutIndices, 0, false);
    MeshData.addUV1(UV1, 0);
    UE_LOG(LogTemp, Log, TEXT("Vertices Num : %d, Indices Num : %d, UVs Num : %d"), Vertices.size(), OutIndices.size(), UV1.size());

    return MeshData;
}

FString FPLATEAUMeshExporter::RemoveSuffix(const FString ComponentName) {
    int Index = 0;
    if (ComponentName.FindLastChar('_', Index)) {
        if (ComponentName.RightChop(Index).Contains("-")) {
            return ComponentName;
        }
        else {
            return ComponentName.LeftChop(ComponentName.Len() - Index);
        }
    }
    else
        return ComponentName;
}