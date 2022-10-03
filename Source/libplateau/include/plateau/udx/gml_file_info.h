#pragma once

#include <libplateau_api.h>
#include <plateau/udx/mesh_code.h>

namespace plateau::udx {

    class LIBPLATEAU_EXPORT GmlFileInfo {
    public:
        explicit GmlFileInfo(const std::string& path);

        const std::string& getPath() const;
        MeshCode getMeshCode() const;
        const std::string& getFeatureType() const;
        std::string getAppearanceDirectoryPath() const;

    private:
        std::string path_;
        std::string code_;
        std::string feature_type_;
    };
}
