// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUExportModelBtn.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"
#include "Kismet/KismetSystemLibrary.h"
#define LOCTEXT_NAMESPACE "UPLATEAUExportModelBtn"

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
    const auto& FoundFileArray = plateau::Export::GetFoundFiles(static_cast<EMeshFileFormat>(FileFormat), ExportPath);
    if (0 < FoundFileArray.Num()) {
        const FText Title = LOCTEXT("ConfirmOverwriteTitle", "ファイル出力確認");
        FFormatOrderedArguments Args;
        Args.Add(FText::FromString(plateau::Export::MeshFileFormatToStr(static_cast<EMeshFileFormat>(FileFormat))));
        const FText DialogText = FText::Format(LOCTEXT("ConfirmOverwriteDesc", "出力先に{0}ファイルが既に存在しています。\n上書きしますか？"), Args);
        if (FMessageDialog::Open(EAppMsgType::YesNo, DialogText, Title) == EAppReturnType::No) {
            return;
        }
    }
    
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
    if (MeshExporter.Export(ExportPath, TargetCityModel, Options)) {
        const auto OpenDirectlyPath = FString::Format(TEXT("file://{0}"), {ExportPath});
        if (UKismetSystemLibrary::CanLaunchURL(OpenDirectlyPath)) {
            UKismetSystemLibrary::LaunchURL(OpenDirectlyPath);
        } else {
            const FText Title = LOCTEXT("OpenDirectlyFailureResultTitle", "エラー");
            const FText DialogText = LOCTEXT("OpenDirectlyFailureResultDesc", "出力先のディレクトリが開けませんでした。");
            FMessageDialog::Open(EAppMsgType::Ok, DialogText, Title);
        }
    } else {
        const FText Title = LOCTEXT("FileExportFailureResultTitle", "エラー");
        const FText DialogText = LOCTEXT("FileExportFailureResultDesc", "ファイル出力に失敗しました。");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText, Title);
    }
}

#undef LOCTEXT_NAMESPACE