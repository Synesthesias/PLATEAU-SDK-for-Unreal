#pragma once

namespace plateau::texture{
    class AtlasInfo {
    public:

        explicit AtlasInfo(const bool valid, const size_t left, const size_t top,
                           const size_t width, const size_t height,
                           double u_pos, double v_pos, double u_factor, double v_factor, std::string src_texture_path) :
                valid_(valid), left_(left), top_(top), width_(width), height_(height),
                u_pos_(u_pos), v_pos_(v_pos), u_factor_(u_factor), v_factor_(v_factor), src_texture_path(std::move(src_texture_path)) {
        }

        static AtlasInfo empty() {
            return AtlasInfo(false, 0, 0, 0, 0, 0, 0, 0, 0, "");
        }

        size_t getLeft() const {
            return left_;
        }
        size_t getTop() const {
            return top_;
        }
        size_t getWidth() const {
            return width_;
        }
        size_t getHeight() const {
            return height_;
        }
        double getUPos() const {
            return u_pos_;
        }
        double getVPos() const {
            return v_pos_;
        }
        double getUFactor() const {
            return u_factor_;
        }
        double getVFactor() const {
            return v_factor_;
        }

        bool getValid() const;

        const std::string& getSrcTexturePath() const {
            return src_texture_path;
        };

    private:
        bool valid_;     // パッキングが成功したかどうか
        size_t left_;    // パッキングされた画像の左上のX座標
        size_t top_;     // パッキングされた画像の左上のY座標
        size_t width_;   // パッキングされた画像の幅
        size_t height_;  // パッキングされた画像の高さ
        double u_pos_;    // uv座標の左上u座標
        double v_pos_;    // uv座標の左上v座標
        double u_factor_; // u座標の倍率
        double v_factor_; // v座標の倍率
        std::string src_texture_path;
    };
}
