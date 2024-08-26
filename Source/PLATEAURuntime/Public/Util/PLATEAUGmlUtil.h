// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include <PLATEAUImportSettings.h>
#include <CityGML/PLATEAUCityObject.h>
#include "plateau/polygon_mesh/node.h"

class PLATEAURUNTIME_API FPLATEAUGmlUtil {
public:
    /**
    * @brief 3D都市モデル内のCityGMLファイルに相当するコンポーネントを入力として、CityGMLファイル名を返します。
    * @return CityGMLファイル名
    */
    static FString GetGmlFileName(const USceneComponent* const InGmlComponent);

    /**
     * @brief Gmlコンポーネントのパッケージ情報を取得します。
     */
    static plateau::dataset::PredefinedCityModelPackage GetCityModelPackage(const USceneComponent* const InGmlComponent);

    //ComponentからNode Path String取得
    static FString GetNodePathString(const USceneComponent* Component);
    //plateau::polygonMesh::NodeからNode Path String取得
    static FString GetNodePathString(const plateau::polygonMesh::Node& InNode);
    //String ArrayからNode Path String取得
    static FString GetNodePathString(TArray<FString> NodePath);

    /**
     * @brief FPLATEAUCityObjectの子階層のGml Idを取得します。
     */
    static TSet<FString> GetChildrenGmlIds(const FPLATEAUCityObject CityObj);

    //CityModelからTMap<FString, FPLATEAUCityObject>を生成します
    static TMap<FString, FPLATEAUCityObject> CreateMapFromCityModel(const std::shared_ptr<const citygml::CityModel> InCityModel);

    //citygml::CityObjectからFPLATEAUCityObjectを生成します　(CityObjectIndex, Childrenは無視）
    static void ConvertCityObject(const citygml::CityObject* InCityObject, FPLATEAUCityObject& OutCityObject);
};
