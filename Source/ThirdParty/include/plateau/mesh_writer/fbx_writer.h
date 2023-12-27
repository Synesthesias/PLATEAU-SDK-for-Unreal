#pragma once
#include <string>

#include <libplateau_api.h>
#include <plateau/polygon_mesh/mesh_extractor.h>

namespace plateau::meshWriter {
    enum class FbxFileFormat : uint32_t
    {
        Binary,
        ASCII
    };

    /**
     * \brief FBX出力の設定です。
     */
    struct FbxWriteOptions {
        FbxFileFormat file_format;
        geometry::CoordinateSystem coordinate_system;
    };

    class LIBPLATEAU_EXPORT FbxWriter {
    public:
        bool write(const std::string& fbx_file_path, const plateau::polygonMesh::Model& model, const FbxWriteOptions& options);
    };
}
