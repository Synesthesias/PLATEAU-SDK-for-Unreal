// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUExportModelTest.h"
#include "PLATEAUExportModelBtn.h"

/**
 * @brief モデル出力
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param ExportPath モデル出力先フォルダパス
 * @param FileFormat 出力モデルのファイルフォーマット
 * @param bExportAsBinary バイナリとして出力するか？
 * @param bExportHiddenModel 非表示モデルを出力するか？
 * @param bExportTexture テクスチャを出力するか？
 * @param CoordinateSystem 座標設定
 * @param TransformType 座標系の設定
 */
void UPLATEAUExportModelTest::ExportModel(APLATEAUInstancedCityModel* TargetCityModel, const FString& ExportPath, const uint8 FileFormat, const bool bExportAsBinary, const bool bExportHiddenModel, const bool bExportTexture, const uint8 CoordinateSystem, const uint8 TransformType) {
    UPLATEAUExportModelBtn::ExportModel(TargetCityModel, ExportPath, FileFormat, bExportAsBinary, bExportHiddenModel, bExportTexture, CoordinateSystem, TransformType);
}
