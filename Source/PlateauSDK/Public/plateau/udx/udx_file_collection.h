#pragma once

#include <map>

#include <libplateau_api.h>
#include <plateau/udx/gml_file_info.h>
#include <plateau/udx/city_model_package_info.h>

/**
 * \brief PLATEAUの3D都市モデルのフォルダ階層でudxの直下にあるフォルダを表します。
 */
class UdxSubFolder {
public:
    explicit UdxSubFolder(std::string name, PredefinedCityModelPackage package)
        : name_(std::move(name)), package_(package) {
    }

    explicit UdxSubFolder(std::string name)
        : name_(std::move(name)) {
        if (*this == bldg())
            package_ = bldg().package_;
    }

    //! 建築物、建築物部分、建築物付属物及びこれらの境界面
    static UdxSubFolder bldg() {
        return UdxSubFolder("bldg", PredefinedCityModelPackage::Building);
    }

    //! 道路
    static UdxSubFolder tran() {
        return UdxSubFolder("tran", PredefinedCityModelPackage::Road);
    }

    //! 都市計画決定情報
    static UdxSubFolder urf() {
        return UdxSubFolder("urf", PredefinedCityModelPackage::UrbanPlanningDecision);
    }

    //! 土地利用
    static UdxSubFolder luse() {
        return UdxSubFolder("luse", PredefinedCityModelPackage::LandUse);
    }

    //! 洪水浸水想定区域
    static UdxSubFolder fld() {
        return UdxSubFolder("fld", PredefinedCityModelPackage::DisasterRisk);
    }

    //! 津波浸水想定
    static UdxSubFolder tnm() {
        return UdxSubFolder("tnm", PredefinedCityModelPackage::DisasterRisk);
    }

    //! 土砂災害警戒区域
    static UdxSubFolder lsld() {
        return UdxSubFolder("lsld", PredefinedCityModelPackage::DisasterRisk);
    }

    //! 高潮浸水想定区域
    static UdxSubFolder htd() {
        return UdxSubFolder("htd", PredefinedCityModelPackage::DisasterRisk);
    }

    //! 内水浸水想定区域
    static UdxSubFolder ifld() {
        return UdxSubFolder("ifld", PredefinedCityModelPackage::DisasterRisk);
    }
    //! 都市設備
    static UdxSubFolder frn() {
        return UdxSubFolder("frn", PredefinedCityModelPackage::UrbanFacility);
    }

    //! 植生
    static UdxSubFolder veg() {
        return UdxSubFolder("veg", PredefinedCityModelPackage::Vegetation);
    }

    //! 起伏
    static UdxSubFolder dem() {
        return UdxSubFolder("dem", PredefinedCityModelPackage::Relief);
    }

    const std::string& name() const {
        return name_;
    }

    PredefinedCityModelPackage package() const {
        return package_;
    }

    CityModelPackageInfo getPackageInfo() const {
        return CityModelPackageInfo::getPredefined(package_);
    }

    bool operator==(const UdxSubFolder& rhs) const {
        return name_ == rhs.name_;
    }

private:
    std::string name_;
    PredefinedCityModelPackage package_;
};

class LIBPLATEAU_EXPORT UdxFileCollection {
public:
    static UdxFileCollection find(const std::string& udx_path);
    static UdxFileCollection filter(const UdxFileCollection& collection, const std::vector<MeshCode>& mesh_codes);
    void copyFiles(const std::string& destination_root_path);
    void copyFiles(const std::string& destination_root_path, const UdxSubFolder& sub_folder);
    void copyCodelistFiles(const std::string& destination_root_path);

    std::vector<GmlFileInfo>& getGmlFiles(const UdxSubFolder& sub_folder);
    std::vector<GmlFileInfo> getAllGmlFiles();
    std::vector<UdxSubFolder> getSubFolders();
    std::vector<MeshCode> getMeshCodes();

    std::string getRelativePath(const std::string& path) const;

private:
    std::string udx_path_;
    std::map<std::string, std::vector<GmlFileInfo>> files_;
};
