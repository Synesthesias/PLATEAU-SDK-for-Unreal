
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "png_texture_image.h"

struct jpeg_error_mgr;

namespace plateau::texture {
    class JpegTextureImage {
    public:
        explicit JpegTextureImage() {
        }

        ~JpegTextureImage() = default;

        /**
         * \brief 指定されたファイルから画像を読み込み、テクスチャ画像を作成します。
         * \param file_name 画像ファイルのパス
         * \param height_limit 画像の高さがこの値を超える場合画像データは読み込まれません。
         * \return 読み込みに成功した場合true、それ以外はfalse
         */
        bool init(const std::string& file_name, const size_t height_limit);
        void init(size_t width, size_t height, size_t color);

        size_t getWidth() const {
            return image_width_;
        }

        size_t getHeight() const {
            return image_height;
        }

        std::vector<uint8_t>& getBitmapData() {
            return bitmap_data_;
        }

        bool save(const std::string& file_name);
        void pack(size_t x_delta, size_t y_delta, const JpegTextureImage& image);
        void packPng(const size_t x_delta, const size_t y_delta, PngTextureImage& image);

    private:
        std::shared_ptr<::jpeg_error_mgr> jpegErrorManager;
        std::vector<uint8_t> bitmap_data_;
        std::string                       filePath;
        size_t                            image_width_;
        size_t                            image_height;
        size_t                            image_channels_;
        int                               colourSpace;
    };

} // namespace plateau::texture
