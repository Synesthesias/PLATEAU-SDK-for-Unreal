// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Export/PLATEAUExportModelAPI.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"
#include "Kismet/KismetSystemLibrary.h"
#define LOCTEXT_NAMESPACE "PLATEAUExportModelAPI"

/**
 * @brief モデル出力
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param ExportPath モデル出力先フォルダパス
 * @param FPLATEAUMeshExportOptions 出力オプション
 */
void UPLATEAUExportModelAPI::ExportModel(APLATEAUInstancedCityModel* TargetCityModel, const FString& ExportPath, const FPLATEAUMeshExportOptions& Options) {

    const auto& FoundFileArray = plateau::Export::GetFoundFiles(Options.FileFormat, ExportPath);
    if (0 < FoundFileArray.Num()) {
        const FText Title = LOCTEXT("ConfirmOverwriteTitle", "ファイル出力確認");
        FFormatOrderedArguments Args;
        Args.Add(FText::FromString(plateau::Export::MeshFileFormatToStr(Options.FileFormat)));
        const FText DialogText = FText::Format(LOCTEXT("ConfirmOverwriteDesc", "出力先に{0}ファイルが既に存在しています。\n上書きしますか？"), Args);
        if (FMessageDialog::Open(EAppMsgType::YesNo, DialogText, Title) == EAppReturnType::No) {
            return;
        }
    }

    FPLATEAUMeshExporter MeshExporter;
    if (!FPaths::DirectoryExists(ExportPath)) {
        FFileManagerGeneric::Get().MakeDirectory(*ExportPath, true);
    }

    if (MeshExporter.Export(ExportPath, TargetCityModel, Options)) {
        const auto OpenDirectlyPath = FString::Format(TEXT("file://{0}"), { ExportPath });
        if (UKismetSystemLibrary::CanLaunchURL(OpenDirectlyPath)) {
            UKismetSystemLibrary::LaunchURL(OpenDirectlyPath);
        }
        else {
            const FText Title = LOCTEXT("OpenDirectlyFailureResultTitle", "エラー");
            const FText DialogText = LOCTEXT("OpenDirectlyFailureResultDesc", "出力先のディレクトリが開けませんでした。");
            FMessageDialog::Open(EAppMsgType::Ok, DialogText, Title);
        }
    }
    else {
        const FText Title = LOCTEXT("FileExportFailureResultTitle", "エラー");
        const FText DialogText = LOCTEXT("FileExportFailureResultDesc", "ファイル出力に失敗しました。");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText, Title);
    }

}

#undef LOCTEXT_NAMESPACE