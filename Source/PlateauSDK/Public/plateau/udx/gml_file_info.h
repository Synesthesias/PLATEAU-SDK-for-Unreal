#pragma once

#include <libplateau_api.h>
#include <plateau/udx/mesh_code.h>

class LIBPLATEAU_EXPORT GmlFileInfo {
public:
    explicit GmlFileInfo(const std::string& path);

    const std::string& getPath() const;
    const MeshCode& getMeshCode() const;
    const std::string& getFeatureType() const;
    std::string getAppearanceDirectoryPath() const;

private:
    std::string path_;
    MeshCode code_;
    std::string feature_type_;
};
