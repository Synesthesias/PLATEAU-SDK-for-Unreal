
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "texture_image_base.h"

struct jpeg_error_mgr;

namespace plateau::texture {
    class TiffTextureImage : public TextureImageBase {
    public:

        typedef enum {
            NONE_COMPRESSION, LZW_COMPRESSION
        } COMPRESSION_TYPE_t;

        explicit TiffTextureImage(const std::string& file_path) :
                file_path_(file_path),
                load_succeed_(init(file_path))
        {
        };

        size_t getWidth() const override {
            return image_width;
        };

        size_t getHeight() const override {
            return image_height;
        };

        bool save(const std::string& file_path) override{
            throw std::runtime_error("Outputting tiff file is not supported.");
            return false;
        };

        void packTo(TextureImageBase* dest, size_t x_delta, size_t y_delta) override;

        const std::string& getFilePath() const override {
            return file_path_;
        };

        virtual bool loadSucceed() const override{
            return load_succeed_;
        };

        std::vector<std::vector<uint8_t>>& getBitmapData();

    private:
        bool init(const std::string& fileName);
        std::vector<std::vector<uint8_t>> bitmapData;
        unsigned int image_width = 0;
        unsigned int image_height = 0;
        uint16_t image_channels = 0;
        bool load_succeed_;
        std::string file_path_;
    };
} // namespace tiff


