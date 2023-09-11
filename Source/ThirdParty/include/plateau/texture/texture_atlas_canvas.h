#pragma once

#include "texture_image_base.h"
#include "atlas_info.h"
#include "atlas_container.h"

namespace plateau::texture{
    class TextureAtlasCanvas {
    public:

        explicit TextureAtlasCanvas(size_t width, size_t height) :
                vertical_range_(0), capacity_(0), coverage_(0),
                canvas_(TextureImageBase::createNewTexture(width, height)),
                canvas_width_(width), canvas_height_(height) {
        }


        void setSaveFilePathIfEmpty(const std::string& original_file_path);
        const std::string& getSaveFilePath() const;

        TextureImageBase& getCanvas() {
            return *canvas_;
        }

        void flush();

        /**
         * \brief テクスチャ全体に対しての既にパックされた画像の占有率（100%）
         */
        double getCoverage() const {
            return coverage_;
        }

        void update(const size_t width, const size_t height, const bool is_new_container, const AtlasInfo& packed_texture_info); // 画像のパッキング成功時の処理、第3引数（TRUE:新規コンテナを作成、FALSE:既存コンテナに追加）
        AtlasInfo insert(const size_t width, const size_t height, const std::string& src_texture_path); // 指定された画像領域（width x height）の領域が確保できるか検証、戻り値AtrasInfoの「valid」ブール値（true:成功、false:失敗）で判定可能
        bool isTexturePacked(const std::string& src_file_path, AtlasInfo& out_atlas_info);

    private:
        std::vector<AtlasContainer> container_list_;
        size_t canvas_width_;
        size_t canvas_height_;
        size_t vertical_range_;
        size_t capacity_;
        double coverage_;
        std::unique_ptr<TextureImageBase> canvas_;
        std::string save_file_path_;
        std::vector<AtlasInfo> packed_textures_info;
    };
}
