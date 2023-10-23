#pragma once

#include <libplateau_api.h>
#include <map>
#include <string>
#include <cmath>
#include <set>

namespace plateau::polygonMesh {

    struct LIBPLATEAU_EXPORT CityObjectIndex {
        int primary_index;
        int atomic_index;

        CityObjectIndex()
            : primary_index(0)
            , atomic_index(0) {
        }

        CityObjectIndex(const int primary_index, const int atomic_index)
            : primary_index(primary_index)
            , atomic_index(atomic_index) {
        }

        static CityObjectIndex fromUV(const TVec2f& uv) {
            return {
                static_cast<int>(std::lround(uv.x)),
                static_cast<int>(std::lround(uv.y))
            };
        }

        TVec2f toUV() const {
            return {
                static_cast<float>(primary_index),
                static_cast<float>(atomic_index)
            };
        }

        static int invalidIndex() {
            return -1;
        }

        static CityObjectIndex first() {
            return { 0, 0 };
        }

        CityObjectIndex nextAtomic() const {
            return {
                primary_index,
                atomic_index + 1
            };
        }

        CityObjectIndex getPrimary() const {
            return {
                primary_index,
                invalidIndex()
            };
        }

        bool operator<(const CityObjectIndex& other) const {
            return primary_index < other.primary_index
                ? true
                : primary_index > other.primary_index
                ? false
                : atomic_index < other.atomic_index;
        }

        bool operator==(const CityObjectIndex& other) const {
            return primary_index == other.primary_index && atomic_index == other.atomic_index;
        };

        std::string toString() const {
            return std::to_string(primary_index) + "," + std::to_string(atomic_index);
        };
    };


    /// CityObjectIndex の複数形です。
    class CityObjectIndexSet {
    public:
        CityObjectIndexSet() : indices_(){};

        void insert(const CityObjectIndex& city_obj_index) {
            indices_.insert(city_obj_index);
        }

        size_t sizeOfPrimary() const {
            size_t ret = 0;
            for (const auto& index: indices_) {
                if (index.atomic_index == CityObjectIndex::invalidIndex()) ret++;
            }
            return ret;
        }

        size_t sizeOfAtomic() const {
            return indices_.size() - sizeOfPrimary();
        }

        /// セットに含まれるPrimaryIndexを列挙します。
        std::set<int> listPrimaryIndices() const {
            std::set<int> primaries;
            for(auto& index : indices_) {
                primaries.insert(index.primary_index);
            }
            return primaries;
        }

        const std::set<CityObjectIndex>& get() const {
            return indices_;
        }

    private:
        std::set<CityObjectIndex> indices_;
    };


    /**
     * @brief CityObjectListは、地物インデックスと地物IDの対応関係を保持するために、Modelに含まれる地物のリストを保持する目的で設計されています。
     *
     * plateau::polygonMesh::Meshに次のメンバー変数を追加します。
     * 上記のUVに記録したID ( CityObjectIndex ) と、 gml:id を対応付けるデータ構造  CityObjectListを作成します。
     * CityObjectListは、 std::map<CityObjectIndex, std::string> city_object_index_to_gml_idから構成されます。
     * なお、std::map のキーに自作の構造体をとるには、比較演算子< を定義する必要があることに注意してください。
     * 参考: 逆引き！ C++でMapのキーとして、自分で定義した構造体を利用する
     * 「https://programming-tips.info/use_struct_as_key_of_map/cpp/index.html」
     *
     * mesh_extractor の処理において、上記の city_object_indexが構築されるようにし、ゲームエンジンから読めるようにします。
     */
    class LIBPLATEAU_EXPORT CityObjectList {
        using TIdMap = std::map<CityObjectIndex, std::string>;

    public:
        CityObjectList() = default;
        CityObjectList(const std::vector<std::tuple<CityObjectIndex, std::string>>& initial_val);

        const std::string& getAtomicGmlID(const CityObjectIndex& city_object_index) const;
        const std::string& getPrimaryGmlID(int index) const;
        bool tryGetPrimaryGmlID(int index, std::string& out_gml_id) const;
        bool tryGetAtomicGmlID(const CityObjectIndex& city_obj_index, std::string& out_gml_id) const;
        bool containsCityObjectIndex(const CityObjectIndex& city_obj_index) const;

        void getAllKeys(std::vector<CityObjectIndex>& keys) const;

        std::shared_ptr<std::vector<CityObjectIndex>> getAllKeys() const;
        std::vector<CityObjectIndex> getAllPrimaryIndices() const;
        std::vector<CityObjectIndex> getAllAtomicIndices() const;

        CityObjectIndex getCityObjectIndex(const std::string& gml_id) const;

        void add(const CityObjectIndex& key, const std::string& value);

        size_t size() const;

        bool operator==(const CityObjectList& other) const;
        decltype(TIdMap().begin()) begin() {return city_object_index_to_gml_id_.begin();};
        decltype(TIdMap().end()) end(){return city_object_index_to_gml_id_.end();};
        TIdMap& getIdMap() { return city_object_index_to_gml_id_; };
        bool containsPrimaryGmlId(const std::string& primary_gml_id) const;


    private:
        TIdMap city_object_index_to_gml_id_;
    };

}
