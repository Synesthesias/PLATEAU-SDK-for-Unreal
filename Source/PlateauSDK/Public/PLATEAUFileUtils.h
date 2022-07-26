#pragma once

#include "CoreMinimal.h"

#include "PLATEAUCityModelLoader.h"

#include "plateau/mesh/mesh_convert_options.h"

/*
 * ファイル操作のためのユーティリティを提供します。
 */
class PLATEAUFileUtils {
public:
    /*
     * メッシュファイルを全て出力先のパスにインポートします。
     *
     * @param GmlFiles インポートするGMLファイル
     * @param DestinationPath 出力先のコンテンツパス(/Game/...)
     * @return 
     */
    static void ImportFbx(const TArray<FString>& GmlFiles, const FString& DestinationPath, const TMap<ECityModelPackage, MeshConvertOptions>& MeshConvertOptionsMap);
};
