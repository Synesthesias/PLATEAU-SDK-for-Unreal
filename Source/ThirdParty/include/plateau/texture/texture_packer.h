
#pragma once

#include <plateau/polygon_mesh/model.h>
#include <plateau/texture/texture_image_base.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>
#include "texture_atlas_canvas.h"

namespace plateau::texture {

    /// テクスチャのアトラス化をします。
    /// 実装上の注意：
    /// TexturePacker にAPIを増やすとき、変更が必要なのは texture_packer.cpp に加えて texture_packer_dummy.cpp もであることに注意してください。
    class LIBPLATEAU_EXPORT TexturePacker {
    public:

        explicit TexturePacker(size_t width, size_t height, const int internal_canvas_count = 8);

        void process(plateau::polygonMesh::Model& model);
        void processNodeRecursive(const plateau::polygonMesh::Node& node);
        void processMesh(plateau::polygonMesh::Mesh* mesh);

    private:
        bool isTexturePacked(const std::string& src_file_path, TextureAtlasCanvas*& out_contained_canvas_ptr, AtlasInfo& out_atlas_info);
        std::vector<std::shared_ptr<TextureAtlasCanvas>> canvases_;
        size_t canvas_width_;
        size_t canvas_height_;

    };
} // namespace plateau::texture
