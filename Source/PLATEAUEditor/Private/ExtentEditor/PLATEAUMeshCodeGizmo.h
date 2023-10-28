// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include <plateau/dataset/mesh_code.h>

namespace plateau {
    namespace geometry {
        class GeoReference;
    }
}

/**
 * @brief 各地域メッシュのメッシュコードのギズモを表します。
 */
class PLATEAUEDITOR_API FPLATEAUMeshCodeGizmo {
public:
    FPLATEAUMeshCodeGizmo();

    void ResetSelectedArea();
    void DrawExtent(const FSceneView* View, FPrimitiveDrawInterface* PDI) const;
    void DrawRegionMeshID(const FViewport& InViewport, const FSceneView& View, FCanvas& Canvas, const FString& RegionMeshID, double CameraDistance, int IconCount) const;

    /**
     * @brief 内部状態から範囲の最小値を取得します。
     */
    FVector2D GetMin() const;

    /**
     * @brief 内部状態から範囲の最大値を取得します。
     */
    FVector2D GetMax() const;

    /**
     * @brief 内部状態から範囲のサイズを取得します。 
     */
    FVector2D GetSize() const;

    /**
     * @brief メッシュコード取得
     */
    plateau::dataset::MeshCode GetMeshCode() const;
    
    /**
     * @brief メッシュID取得
     */
    FString GetRegionMeshID() const;

    /**
     * @brief 選択状態取得 
     */
    TArray<bool> GetbSelectedArray() const;

    /**
     * @brief 範囲が選択されているか否か 
     */
    bool bSelectedArea() const;

    /**
    * @brief 選択状態設定 
    */
    void SetbSelectedArray(const TArray<bool>& InbSelectedArray);
    
    /**
     * @brief インスタンスを初期化します。
     */
    void Init(const plateau::dataset::MeshCode& InMeshCode, const plateau::geometry::GeoReference& InGeoReference);

    /**
     * @brief マウス座標がエリア内であれば選択状態をトグル
     * @param X マウス座標X
     * @param Y マウス座標Y
     */
    void ToggleSelectArea(const double X, const double Y);

    /**
     * @brief ドラッグ矩形がギズモ範囲と交差していた時に選択されたとみなす
     * @param InMin ドラッグ矩形の最小座標
     * @param InMax ドラッグ矩形の最大座標
     * @param bSelect エリアを指定の選択状態に設定
     */
    void SetSelectArea(const FVector2d InMin, const FVector2d InMax, const bool bSelect);

    /**
     * @brief マウス座標がエリア内であれば指定の選択状態に設定
     * @param X マウス座標X
     * @param Y マウス座標Y
     * @param bSelect エリアを指定の選択状態に設定
     */
    void SetSelectArea(const double X, const double Y, const bool bSelect);

    /**
     * @brief 選択されているメッシュID配列を取得
     */
    TArray<FString> GetSelectedMeshIds();
    
    /**
     * @brief エリア内の描画有効化状態を設定
     * @param bValue 有効化するか？
     */
    static void SetShowLevel5Mesh(const bool bValue);

private:
    int MeshCodeLevel;
    inline static bool bShowLevel5Mesh = false;

    plateau::dataset::MeshCode MeshCode;
    FString MeshCodeString;
    double Width;
    double Height;
    double MinX;
    double MinY;
    double MaxX;
    double MaxY;
    float LineThickness;
    TArray<bool> bSelectedArray;
    TObjectPtr<UMaterialInstanceDynamic> AreaSelectedMaterial;
    TObjectPtr<UMaterialInstanceDynamic> AreaUnSelectedMaterial;

    bool IsThirdMeshCode() const;
};
