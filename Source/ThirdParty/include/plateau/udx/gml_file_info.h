#pragma once

#include <libplateau_api.h>
#include <plateau/udx/mesh_code.h>

namespace plateau::udx {

    class LIBPLATEAU_EXPORT GmlFileInfo {
    public:
        explicit GmlFileInfo(const std::string& path);

        const std::string& getPath() const;
        void setPath(const std::string& path);
        MeshCode getMeshCode() const;
        const std::string& getFeatureType() const;
        std::string getAppearanceDirectoryPath() const;
        bool isValid() const;

    private:
        std::string path_;
        std::string code_;
        std::string feature_type_;
        bool is_valid_;

        void applyPath();
    };
}
