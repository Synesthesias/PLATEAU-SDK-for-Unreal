#pragma once

#include <libplateau_api.h>
#include <plateau/dataset/gml_file.h>
#include <plateau/dataset/city_model_package.h>

namespace plateau::geometry {
    class GeoReference;
}

namespace plateau::dataset {
    /**
     * \brief PLATEAUの3D都市モデルのフォルダ階層でudxの直下にあるフォルダを表します。
     */
    class LIBPLATEAU_EXPORT UdxSubFolder {
    public:
        UdxSubFolder(std::string name)
            : name_(std::move(name)) {
        }

        static PredefinedCityModelPackage getPackage(const std::string& folder_name);
        static CityModelPackageInfo getPackageInfo(const std::string& folder_name);

        const std::string& getName() const {
            return name_;
        }

        operator std::string& () {
            return name_;
        }

        operator std::string() const {
            return name_;
        }

    private:
        std::string name_;

        //! 建築物、建築物部分、建築物付属物及びこれらの境界面
        static const std::string bldg;
        //! 道路
        static const std::string tran;
        //! 都市計画決定情報
        static const std::string urf;
        //! 土地利用
        static const std::string luse;
        //! 都市設備
        static const std::string frn;
        //! 植生
        static const std::string veg;
        //! 起伏
        static const std::string dem;
        //! 洪水浸水想定区域
        static const std::string fld;
        //! 津波浸水想定
        static const std::string tnm;
        //! 土砂災害警戒区域
        static const std::string lsld;
        //! 高潮浸水想定区域
        static const std::string htd;
        //! 内水浸水想定区域
        static const std::string ifld;
    };

    class LIBPLATEAU_EXPORT IDatasetAccessor {
    public:
        /**
         * \brief GMLファイル群のうち、範囲が extent の内部であり、パッケージ種が package であるものを vector で返します。
         * なお、 package はフラグの集合と見なされるので、複数のビットを立てることで複数の指定が可能です。
         */
         virtual std::shared_ptr<std::vector<GmlFile>> getGmlFiles(const PredefinedCityModelPackage package) = 0;

         virtual void getGmlFiles(const PredefinedCityModelPackage package,
                                  std::vector<GmlFile>& out_vector) = 0;

         /**
          * \brief 座標範囲で都市モデルデータをフィルタリングします。
          * \param extent 座標範囲
          * \param collection フィルタリングされた都市モデルデータの格納先
          */
        virtual void filter(const geometry::Extent& extent, IDatasetAccessor& collection) const = 0;

        /**
         * \brief 座標範囲で都市モデルデータをフィルタリングします。
         * \param extent 座標範囲
         * \return フィルタリングされた都市モデルデータ
         */
        virtual std::shared_ptr<IDatasetAccessor> filter(const geometry::Extent& extent) const = 0;

        /**
         * \brief メッシュコードで都市モデルデータをフィルタリングします。
         * \param mesh_codes 欲しい地域IDのvector
         * \param collection フィルタリングされた都市モデルデータの格納先
         */
        virtual void filterByMeshCodes(const std::vector<MeshCode>& mesh_codes, IDatasetAccessor& collection) const = 0;

        /**
         * \brief メッシュコードで都市モデルデータをフィルタリングします。
         * \param mesh_codes 欲しい地域IDのvector
         * \return フィルタリングされた都市モデルデータ
         */
        virtual std::shared_ptr<IDatasetAccessor> filterByMeshCodes(const std::vector<MeshCode>& mesh_codes) const = 0;

        /**
         * \brief 都市モデルデータが存在する地域メッシュのリストを取得します。
         */
        virtual std::set<MeshCode>& getMeshCodes() = 0;

        virtual TVec3d calculateCenterPoint(const plateau::geometry::GeoReference& geo_reference) = 0;

        /// 含まれるパッケージ種をフラグで返します。
        virtual PredefinedCityModelPackage getPackages() = 0;

        /// 仮想コンストラクタのイディオムです。
        virtual IDatasetAccessor* create() const = 0;
        virtual IDatasetAccessor* clone() const = 0;

        virtual ~IDatasetAccessor() = default;
    };
}
