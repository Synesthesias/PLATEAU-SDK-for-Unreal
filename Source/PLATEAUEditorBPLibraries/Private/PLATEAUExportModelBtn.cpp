// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUExportModelBtn.h"
#include "PLATEAUMeshExporter.h"

/**
 * @brief モデル出力
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param ExportPath モデル出力先フォルダパス
 * @param FileFormat 出力モデルのファイルフォーマット
 * @param bExportAsBinary バイナリとして出力するか？
 * @param bExportHiddenModel 非表示モデルを出力するか？
 * @param bExportTexture テクスチャを出力するか？
 * @param TransformType 座標設定
 * @param CoordinateSystem 座標系設定
 */
void UPLATEAUExportModelBtn::ExportModel(APLATEAUInstancedCityModel* TargetCityModel, const FString& ExportPath, const uint8 FileFormat, const bool bExportAsBinary, const bool bExportHiddenModel, const bool bExportTexture, const uint8 TransformType, const uint8 CoordinateSystem) {
    MeshExportOptions Options;
    Options.FileFormat = static_cast<EMeshFileFormat>(FileFormat);
    plateau::meshWriter::GltfWriteOptions GltfOptions;
    GltfOptions.mesh_file_format = bExportAsBinary ? plateau::meshWriter::GltfFileFormat::GLTF : plateau::meshWriter::GltfFileFormat::GLB;
    plateau::meshWriter::FbxWriteOptions FbxOptions;
    FbxOptions.file_format = bExportAsBinary ? plateau::meshWriter::FbxFileFormat::Binary : plateau::meshWriter::FbxFileFormat::ASCII;
    Options.GltfWriteOptions = GltfOptions;
    Options.FbxWriteOptions = FbxOptions;
    Options.bExportHiddenObjects = bExportHiddenModel;
    Options.bExportTexture = bExportTexture;
    Options.TransformType = static_cast<EMeshTransformType>(TransformType);
    Options.CoordinateSystem = static_cast<ECoordinateSystem>(CoordinateSystem);
    FPLATEAUMeshExporter MeshExporter;
    MeshExporter.Export(ExportPath, TargetCityModel, Options);
}
