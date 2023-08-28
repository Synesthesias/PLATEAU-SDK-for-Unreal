
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct jpeg_error_mgr;

namespace plateau::texture {
    class TiffTextureImage {
    public:

        typedef enum {
            NONE_COMPRESSION, LZW_COMPRESSION
        } COMPRESSION_TYPE_t;

        explicit TiffTextureImage();

        bool init(const std::string& fileName);
        ~TiffTextureImage() {
        }

        size_t getWidth() const {
            return image_width;
        }
        size_t getHeight() const {
            return image_height;
        }
        std::vector<std::vector<uint8_t>>& getBitmapData();

    private:
        std::vector<std::vector<uint8_t>> bitmapData;
        unsigned int image_width;
        unsigned int image_height;
        unsigned int image_channels;
    };
} // namespace tiff


