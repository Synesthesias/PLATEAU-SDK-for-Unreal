
#pragma once

#include <plateau/texture/png_texture_image.h>

#include <cstdint>
#include <string>
#include <vector>

namespace plateau::texture {
    class PngTextureImage {
    public:
        explicit PngTextureImage();

        ~PngTextureImage() {
        }

        bool init(const std::string& file_name);

        size_t getWidth() const {
            return image_width_;
        }

        size_t getHeight() const {
            return image_height_;
        }

        std::vector<std::vector<uint8_t>>& getBitmapData();

    private:
        std::vector<std::vector<uint8_t>> bitmap_data_;
        unsigned int image_width_;
        unsigned int image_height_;
        unsigned int image_channels_;
    };
}
