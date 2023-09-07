#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <libplateau_api.h>
#include <vector>

namespace plateau::texture {
    /**
     * 画像機能の基底クラスです。
     */
    class LIBPLATEAU_EXPORT TextureImageBase {
    public:

        // 実装上の注意:
        // すべての子クラスがこのクラスの抽象関数をすべて実装しているわけではないことに注意してください。

        /**
         * ファイルパスから画像を読み込みます。
         * 拡張子による場合分けで、JpegTextureImage, PngTextureImage, TiffTextureImageのいずれかを返します。
         */
        static std::unique_ptr<TextureImageBase> tryCreateFromFile(const std::string& file_path, size_t height_limit, bool& out_result);
        /**
         * 指定サイズのJpegTextureImageをメモリ上に作ります。
         */
        static std::unique_ptr<TextureImageBase> createNewTexture(size_t width, size_t height);

        virtual size_t getWidth() const = 0;
        virtual size_t getHeight() const = 0;
        virtual bool save(const std::string& file_path) = 0;
        virtual void packTo(TextureImageBase* dest, size_t x_delta, size_t y_delta) = 0;
        virtual const std::string& getFilePath() const = 0;
        virtual bool loadSucceed() const = 0;
        virtual std::vector<uint8_t>& getBitmapData() = 0;
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

        virtual std::vector<uint8_t>& getBitmapData() override {
            throw std::runtime_error("not supported");
        }

        const std::string empty_file_path_ = "";
    };
}
