#pragma once

#include <list>
#include <citygml/vecs.hpp>

namespace citygml{
    class CityModel;
    class CityObject;
    class Polygon;
}

namespace plateau::polygonMesh {

    class PolygonMeshUtils {
    public:
        /// 仕様上存在しうる最大LODです。 LODは0から始まるので、LODのパターン数は (この数 +1)です。
        static constexpr int max_lod_in_specification_ = 3;

        /**
         * city_model の中心点を返します。
         * また GMLファイルから city_model の Envelope を読み取れない場合は、
         * 中心点が分からないので原点座標を返します。
         */
        static TVec3d getCenterPoint(const citygml::CityModel& city_model, int coordinate_zone_id);

        /**
         * city_obj の子を再帰的に検索して返します。
         * ただし引数のcityObj自身は含めません。
         */
        static std::list<const citygml::CityObject*> getChildCityObjectsRecursive(const citygml::CityObject& city_obj);

        /**
         * cityObjの位置を表現するにふさわしい1点の座標を返します。
         * 注意 :
         * 位置が分からない場合、例外 std::invalid_argument を投げます。
         * ポリゴンがない CityObject の場合は位置不明になるので例外への対応をお願いします。
         */
        static TVec3d cityObjPos(const citygml::CityObject& city_obj);

        /**
         * cityObjのポリゴンであり、頂点数が1以上であるものを検索します。
         * 最初に見つかったポリゴンを返します。なければ nullptr を返します。
         */
        static const citygml::Polygon* findFirstPolygon(const citygml::CityObject* city_obj, unsigned int lod);
    };
}
