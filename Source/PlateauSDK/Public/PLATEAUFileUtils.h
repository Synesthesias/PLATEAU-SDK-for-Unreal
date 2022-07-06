#pragma once

#include "CoreMinimal.h"

/*
 * ファイル操作のためのユーティリティを提供します。
 */
class PLATEAUFileUtils {
public:
    /*
     * メッシュファイルを全て出力先のパスにインポートします。
     *
     * @param files インポートするメッシュファイル
     * @param destinationPath 出力先のコンテンツパス(/Game/...)
     * @return 
     */
    static void ImportFbx(const TArray<FString>& files, const FString& destinationPath);
};
