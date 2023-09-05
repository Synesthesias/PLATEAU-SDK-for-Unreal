
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>

#include "png_texture_image.h"
#include <plateau/texture/texture_image_base.h>

struct jpeg_error_mgr;

namespace plateau::texture {
    class JpegTextureImage : public TextureImageBase {
    public:

        /**
         * jpegファイルをロードします。
         */
        explicit JpegTextureImage(const std::string& file_name, size_t height_limit) :
            load_succeed_(init(file_name, height_limit))
        {
        };

        /**
         * 新規の画像をメモリ上に作ります。
         */
        explicit JpegTextureImage(size_t width, size_t height, uint8_t color){
            init(width, height, color);
        };

        size_t getWidth() const override {
            return image_width_;
        };

        size_t getHeight() const override {
            return image_height;
        };

        const std::string& getFilePath() const override{
            return filePath;
        };

        std::vector<uint8_t>& getBitmapData() {
            return bitmap_data_;
        };

        virtual bool loadSucceed() const override {
            return load_succeed_;
        };

        bool save(const std::string& file_path) override;

        void packTo(TextureImageBase* dest, const size_t x_delta, const size_t y_delta) override;


    private:
        /**
         * \brief 指定されたファイルから画像を読み込み、テクスチャ画像を作成します。
         * \param file_name 画像ファイルのパス
         * \param height_limit 画像の高さがこの値を超える場合画像データは読み込まれません。
         * \return 読み込みに成功した場合true、それ以外はfalse
         */
        bool init(const std::string& file_name, size_t height_limit);
        void init(size_t width, size_t height, uint8_t color);

        std::shared_ptr<::jpeg_error_mgr> jpegErrorManager;
        std::vector<uint8_t> bitmap_data_;
        std::string                       filePath;
        size_t                            image_width_ = 0;
        size_t                            image_height = 0;
        int                            image_channels_ = 0;
        bool load_succeed_;
    };

} // namespace plateau::texture
