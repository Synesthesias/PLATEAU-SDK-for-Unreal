// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAURuntimeUtil.h"
#include "PLATEAUImportSettings.h"

/**
 * @brief パッケージの種類を表すビット一覧を取得
 * @return ビット演算用の数値配列
 */
TArray<int64> UPLATEAURuntimeUtil::GetAllPackages() {
    TArray<int64> Packages;
    for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
        Packages.Add(static_cast<int64>(Package));
    }
    return Packages;
}
