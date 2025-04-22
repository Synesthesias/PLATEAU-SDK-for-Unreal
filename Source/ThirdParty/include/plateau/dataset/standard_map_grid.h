#pragma once

#include <string>

#include <libplateau_api.h>
#include "plateau/geometry/geo_coordinate.h"
#include "plateau/dataset/grid_code.h"

namespace plateau::dataset {
    /**
     * \brief 国土基本図図郭を表します。
     * 
     * 国土基本図の図郭コードを扱い、緯度経度範囲の取得などの機能を提供します。
     */
    class LIBPLATEAU_EXPORT StandardMapGrid : public GridCode {
    public:
        explicit StandardMapGrid(std::string  code);
        StandardMapGrid() = default;

        /**
         * \brief 図郭コードを文字列として取得します。
         */
        std::string get() const override;

        /**
         * \brief 図郭の緯度経度範囲を取得します。
         */
        geometry::Extent getExtent() const override;

        /**
         * \brief 図郭コードが適切な値かどうかを返します。
         */
        bool isValid() const override;

        /**
         * \brief １段階上のレベルのグリッドコードに変換します。
         */
        std::shared_ptr<GridCode> upper() const override;
        GridCode* upperRaw() const override;

        /**
         * \brief コードのレベル（詳細度）を取得します。
         */
        int getLevel() const override;

        /**
         * \brief コードのレベル（詳細度）が、PLATEAUの仕様上考えられる中でもっとも大きいものであるときにtrueを返します。
         */
        bool isLargestLevel() const override;
        bool isSmallerThanNormalGml() const override;
        bool isNormalGmlLevel() const override;

        bool operator==(const StandardMapGrid& other) const;
        bool operator<(const StandardMapGrid& other) const;

    private:
        std::string code_;  // 図郭コード
        bool is_valid_ = false;     // コードが有効かどうか
    };
} 