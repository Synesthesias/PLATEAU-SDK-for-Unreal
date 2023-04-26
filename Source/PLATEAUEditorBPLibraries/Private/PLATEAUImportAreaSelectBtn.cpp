// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUImportAreaSelectBtn.h"
#include <plateau/geometry/geo_reference.h>
#include "PLATEAUImportSettings.h"
#include "PLATEAUEditor/Public/PLATEAUEditor.h"
#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentEditor.h"
#include "Widgets/PLATEAUSDKEditorUtilityWidget.h"

/**
 * @brief エリア選択ウィンドウ表示
 * @param ZoneID ゾーンID
 * @param SourcePath データソースパス（ローカルではデータディレクトリへのパス、サーバではデータセットID）
 * @param bImportFromServer サーバ起動か？
 */
void UPLATEAUImportAreaSelectBtn::OpenAreaWindow(const int ZoneID, const FString& SourcePath, const bool bImportFromServer) {
    const auto& ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();
    ExtentEditor->SetImportFromServer(bImportFromServer);
    if (bImportFromServer) {
        // サーバーデータが受信されている時のみ範囲選択ボタンが表示されるようになっている
        ExtentEditor->SetClientPtr(ExtentEditor->GetPLATEAUSDKEditorUtilityWidget().Get()->GetClientPtr());
        ExtentEditor->SetServerDatasetID(TCHAR_TO_UTF8(*SourcePath));
    } else {
        ExtentEditor->SetSourcePath(SourcePath);
    }

    // ビューポートの操作性向上のため100分の1スケールで設定
    const plateau::geometry::GeoReference RawGeoReference(ZoneID, {}, 1, plateau::geometry::CoordinateSystem::ESU);
    ExtentEditor->SetGeoReference(RawGeoReference);
    
    const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
    GlobalTabManager->TryInvokeTab(FPLATEAUExtentEditor::TabId);
}

/**
 * @brief パッケージの種類を表すビット一覧を取得
 * @return ビット演算用の数値配列
 */
TArray<int64> UPLATEAUImportAreaSelectBtn::GetAllPackages() {
    TArray<int64> Packages;
    for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
        Packages.Add(static_cast<int64>(Package));
    }
    return Packages;
}

/**
 * @brief モデル結合単位の名称が格納された一覧を取得
 * @return モデル結合単位名称配列
 */
TArray<FText> UPLATEAUImportAreaSelectBtn::GetGranularityTexts() {
    TArray<FText> GranularityTexts;
    const auto Items = UPLATEAUImportSettings::GetGranularityTexts();
    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
        GranularityTexts.Add(static_cast<FText>(ItemIter->Value));
    }
    return GranularityTexts;
}

/**
 * @brief パッケージがどのカテゴリーを表すのかを示すための名前一覧を取得
 * @return カテゴリー名マップ
 */
TMap<int64, FText> UPLATEAUImportAreaSelectBtn::GetCategoryNames() {
    return UPLATEAUImportSettings::GetCategoryNames();
}

/**
 * @brief パッケージ情報取得
 * @param Package パッケージを表す数値
 * @return パッケージの情報
 */
FPackageInfo UPLATEAUImportAreaSelectBtn::GetPackageInfo(const int64 Package) {
    const auto PackageInfo = plateau::dataset::CityModelPackageInfo::getPredefined(static_cast<plateau::dataset::PredefinedCityModelPackage>(Package));
    const FPackageInfo PackageInfoData(PackageInfo.hasAppearance(), PackageInfo.minLOD(), PackageInfo.maxLOD());
    return PackageInfoData;
}
