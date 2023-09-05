
#pragma once

#include <plateau/texture/texture_image_base.h>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

namespace plateau::texture {
    class PngTextureImage : public TextureImageBase {
    public:
        explicit PngTextureImage(const std::string& file_path) :
                file_path_(file_path),
                load_succeed(init(file_path)) {
            if (!load_succeed) throw std::runtime_error("png load failed.");
        };

        size_t getWidth() const override {
            return image_width_;
        }

        size_t getHeight() const override {
            return image_height_;
        }

        bool save(const std::string& file_path) override{
            throw std::runtime_error("Outputting png file is not supported.");
        }

        void packTo(TextureImageBase* dest, size_t x_delta, size_t y_delta) override;

        const std::string& getFilePath() const override {
            return file_path_;
        }

        virtual bool loadSucceed() const override {
            return load_succeed;
        }

        const std::vector<std::vector<uint8_t>>& getBitmapData() const;

    private:
        bool init(const std::string& file_name);
        std::vector<std::vector<uint8_t>> bitmap_data_;
        unsigned int image_width_ = 0;
        unsigned int image_height_ = 0;
        unsigned int image_channels_ = 0;
        bool load_succeed;
        std::string file_path_;
    };
}
