// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUImportLocalFolderBtn.h"
#include <plateau/dataset/dataset_source.h>
#include "Interfaces/IMainFrameModule.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

static constexpr TCHAR DialogTitle[] = TEXT("Select folder.");

/**
 * @brief OSのウィンドウハンドルを他のAPIのためにvoidポインタの形で取得する
 * @return OSのウィンドウハンドル
 */
void* UPLATEAUImportLocalFolderBtn::GetWindowHandle() {
    const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
    const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

    if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
        return MainWindow->GetNativeWindow()->GetOSWindowHandle();
    }

    return nullptr;
}

/**
 * @brief 入力フォルダ選択ダイアログ表示
 * @param IsDatasetValid 選択された入力フォルダ内のデータセットが妥当か？
 * @param SourcePath 選択された入力フォルダパス
 * @return 入力フォルダが選択されたか？
 */
bool UPLATEAUImportLocalFolderBtn::OpenDirectoryDialog(bool& IsDatasetValid, FString& SourcePath) {
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (FString OutFolderName; DesktopPlatform->OpenDirectoryDialog(GetWindowHandle(), DialogTitle, "", OutFolderName)) {
        try {
            const auto InDatasetSource = plateau::dataset::DatasetSource::createLocal(TCHAR_TO_UTF8(*OutFolderName));
            const auto LocalDatasetAccessor = InDatasetSource.getAccessor();
            IsDatasetValid = LocalDatasetAccessor != nullptr && LocalDatasetAccessor->getPackages() != plateau::dataset::PredefinedCityModelPackage::None;
            SourcePath = OutFolderName;
            return true;
        } catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *OutFolderName);
            return false;
        }
    }
    return false;
}
