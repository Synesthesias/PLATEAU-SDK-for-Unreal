// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUEditorUtil.h"
#include "Interfaces/IMainFrameModule.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "PLATEAUImportSettings.h"

static constexpr TCHAR OpenDirectoryDialogTitle[] = TEXT("フォルダ選択");

/**
 * @brief OSのウィンドウハンドルを他のAPIのためにvoidポインタの形で取得する
 * @return OSのウィンドウハンドル
 */
void* UPLATEAUEditorUtil::GetWindowHandle() {
    const IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
    const TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

    if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
        return MainWindow->GetNativeWindow()->GetOSWindowHandle();
    }

    return nullptr;
}

/**
 * @brief フォルダ選択ダイアログ表示
 * @param SourcePath 選択された入力フォルダパス
 * @return 入力フォルダが選択されたか？
 */
bool UPLATEAUEditorUtil::OpenDirectoryDialog(FString& SourcePath) {
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (FString OutFolderName; DesktopPlatform->OpenDirectoryDialog(GetWindowHandle(), OpenDirectoryDialogTitle, "", OutFolderName)) {
        try {
            SourcePath = OutFolderName;
            return true;
        } catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *OutFolderName);
            return false;
        }
    }
    return false;
}
