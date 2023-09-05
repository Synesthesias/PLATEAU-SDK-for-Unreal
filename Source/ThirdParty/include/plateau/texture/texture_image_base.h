#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <libplateau_api.h>

namespace plateau::texture {
    class LIBPLATEAU_EXPORT TextureImageBase {
    public:
        /**
         * ファイルパスから画像を読み込みます。
         */
        static std::unique_ptr<TextureImageBase> tryCreateFromFile(const std::string& file_path, size_t height_limit, bool& out_result);
        static std::unique_ptr<TextureImageBase> createNewTexture(size_t width, size_t height);

        virtual size_t getWidth() const = 0;
        virtual size_t getHeight() const = 0;
        virtual bool save(const std::string& file_path) = 0;
        virtual void packTo(TextureImageBase* dest, size_t x_delta, size_t y_delta) = 0;
        virtual const std::string& getFilePath() const = 0;
        virtual bool loadSucceed() const = 0;
        virtual ~TextureImageBase() = default;

    private:
    };

    class EmptyTexture : public TextureImageBase {
    public:
        size_t getWidth() const override {
            return 0;
        };

        size_t getHeight() const override {
            return 0;
        };

        virtual bool save(const std::string& file_path) override{
            throw std::runtime_error("Called save on EmptyTexture.");
        };

        virtual void packTo(TextureImageBase* dest, const size_t x_delta, const size_t y_delta) override{
            throw std::runtime_error("Called packTo on EmptyTexture.");
        }

        virtual const std::string& getFilePath() const override {
            return empty_file_path_;
        }

        virtual bool loadSucceed() const override {
            return false;
        }

        const std::string empty_file_path_ = "";
    };
}
