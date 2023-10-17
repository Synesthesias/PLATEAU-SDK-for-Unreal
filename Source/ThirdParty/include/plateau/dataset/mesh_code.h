#pragma once

#include <string>
#include <vector>

#include <libplateau_api.h>
#include "plateau/geometry/geo_coordinate.h"

namespace plateau::dataset {
    /**
     * \brief 地域メッシュコードを表します。
     * 
     * 2~5次メッシュの緯度経度範囲の取得、緯度経度範囲を内包する3次メッシュの取得を行う機能を提供しています。
     */
    class LIBPLATEAU_EXPORT MeshCode {
    public:
        explicit MeshCode(const std::string& code);
        MeshCode() = default;

        /**
         * \brief メッシュコードを文字列として取得します。
         */
        std::string get() const;

        /**
         * \brief メッシュコードの次数を取得します。
         */
        int getLevel() const;

        /**
         * \brief メッシュコードの緯度経度範囲を取得します。
         */
        geometry::Extent getExtent() const;

        /**
         * \brief 座標点を含む3次メッシュを取得します。
         */
        static MeshCode getThirdMesh(const geometry::GeoCoordinate& coordinate);

        /**
         * \brief 座標範囲を含む3次メッシュを全て取得します。
         */
        static void getThirdMeshes(const geometry::Extent& extent, std::vector<MeshCode>& mesh_codes);

        /**
         * \brief 座標範囲を含む3次メッシュを全て取得します。
         */
        static std::shared_ptr<std::vector<MeshCode>> getThirdMeshes(const geometry::Extent& extent);

        /**
         * \brief 地域メッシュが内包されるかどうかを計算します。
         */
        bool isWithin(const MeshCode& other) const;

        /**
         * \brief 地域メッシュを2次メッシュとして取得します。
        */
        MeshCode asSecond() const;

        /**
         * \brief レベル2以上の範囲で１段階上のレベルの地域メッシュに変換します。
        */
        MeshCode& upper();

        /**
         * \brief メッシュコードが適切な値かどうかを返します。
        */
        bool isValid() const;

        bool operator==(const MeshCode& other) const;

        //! setに入れるために演算子オーバーロードします。
        bool operator<(MeshCode& other) const;
        bool operator<(const MeshCode& other) const;

    private:
        int first_row_;
        int first_col_;
        int second_row_;
        int second_col_;
        int third_row_;
        int third_col_;
        int fourth_row_;
        int fourth_col_;
        int fifth_row_;
        int fifth_col_;
        int level_;
        bool is_valid_;

        static void nextRow(MeshCode& mesh_code);
        static void nextCol(MeshCode& mesh_code);
        static int compareCol(const MeshCode& lhs, const MeshCode& rhs);
        static int compareRow(const MeshCode& lhs, const MeshCode& rhs);
    };
}
