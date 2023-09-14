// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUImportAreaSelectBtn.h"
#include <plateau/geometry/geo_reference.h>
#include "PLATEAUImportSettings.h"
#include "PLATEAUWindow.h"
#include "PLATEAUEditor/Public/PLATEAUEditor.h"
#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentEditor.h"
#include "Widgets/PLATEAUSDKEditorUtilityWidget.h"
#include "Factories/MaterialImportHelpers.h"

#define LOCTEXT_NAMESPACE "UPLATEAUImportAreaSelectBtn"

/**
 * @brief エリア選択ウィンドウ表示
 * @param ZoneID ゾーンID
 * @param SourcePath データソースパス（ローカルではデータディレクトリへのパス、サーバではデータセットID）
 * @param bImportFromServer サーバーからインポートするかどうか
 */
void UPLATEAUImportAreaSelectBtn::OpenAreaWindow(const int ZoneID, const FString& SourcePath, const bool bImportFromServer) {
    const auto& Window = IPLATEAUEditorModule::Get().GetWindow();
    const auto& ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();
    ExtentEditor->SetImportFromServer(bImportFromServer);
    if (bImportFromServer) {
        // サーバーデータが受信されている時のみ範囲選択ボタンが表示されるようになっている
        const auto& EditorUtilityWidget = dynamic_cast<UPLATEAUSDKEditorUtilityWidget*>(Window->GetEditorUtilityWidget());
        if (EditorUtilityWidget != nullptr) {
            ExtentEditor->SetClientPtr(EditorUtilityWidget->GetClientPtr());
            ExtentEditor->SetServerDatasetID(TCHAR_TO_UTF8(*SourcePath));
        } else {
            const FText Title = LOCTEXT("Warning", "警告");
            const FText DialogText = LOCTEXT("WidgetError", "PLATEAU SDKに問題が発生しました。PLATEAU SDKを再起動して下さい。");
            FMessageDialog::Open(EAppMsgType::Ok, DialogText, &Title);
            return;
        }
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
 * @return カテゴリー情報のマップ
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

/**
 * @brief Fallback Material取得
 * @param Package パッケージを表す数値
 * @return Fallback Materialの情報
 */
UMaterialInterface* UPLATEAUImportAreaSelectBtn::GetDefaultFallbackMaterial(const int64 Package) {
    const FString Name = UPLATEAUImportSettings::GetDefaultFallbackMaterialName(Package);
    const FString FallbackPath = "/PLATEAU-SDK-for-Unreal/Materials/Fallback";
    FText Error;
    UMaterialInterface* result = UMaterialImportHelpers::FindExistingMaterial(FallbackPath, Name, false, Error);
    if(result == nullptr)
        UE_LOG(LogTemp, Warning, TEXT("Fallback Material Not Found: %s %s"), *Name, *FString(Error.ToString()));
    return result;
}

#undef LOCTEXT_NAMESPACE