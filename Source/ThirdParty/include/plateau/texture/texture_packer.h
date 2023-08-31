
#pragma once

#include <plateau/polygon_mesh/model.h>
#include <plateau/texture/texture_image.h>

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace plateau::texture {

    class AtlasInfo {
    public:

        explicit AtlasInfo() : valid_(false), left_(0), top_(0), width_(0), height_(0), u_pos_(0), v_pos_(0), u_factor_(0), v_factor_(0) {
        }
        ~AtlasInfo() {
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
        void clear();
        void setAtlasInfo(
            const bool valid, const size_t left, const size_t top,
            const size_t width, const size_t height,
            double u_pos, double v_pos, double u_factor, double v_factor);

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
    };

    class AtlasContainer {
    public:

        explicit AtlasContainer(const size_t _gap, const size_t _horizontal_range, const size_t _vertical_range);
        ~AtlasContainer() {
        }

        size_t getGap() const {
            return gap;
        }
        size_t getRootHorizontalRange() const {
            return root_horizontal_range;
        }
        size_t getHorizontalRange() const {
            return horizontal_range;
        }
        size_t getVerticalRange() const {
            return vertical_range;
        }
        void add(const size_t _length);

    private:
        size_t gap;                     // 画像を格納するコンテナの高さ
        size_t root_horizontal_range;   // コンテナ内で未使用の領域のX座標
        size_t horizontal_range;        // 最後の画像をパッキングするための領域のX座標
        size_t vertical_range;          // パッキングの対象となる親の画像のコンテナが配置されている左上のY座標
    };

    class TextureAtlasCanvas {
    public:
        const unsigned char gray = 80;

        explicit TextureAtlasCanvas() : canvas_width_(0), canvas_height_(0), vertical_range_(0), capacity_(0), coverage_(0) {
        }

        explicit TextureAtlasCanvas(size_t width, size_t height) : vertical_range_(0), capacity_(0), coverage_(0) {
            canvas_width_ = width;
            canvas_height_ = height;
            canvas_.init(width, height, gray);
        }

        ~TextureAtlasCanvas() {
        }

        void setSaveFilePathIfEmpty(const std::string& original_file_path);
        const std::string& getSaveFilePath() const;

        TextureImage& getCanvas() {
            return canvas_;
        }

        void init(size_t width, size_t height);
        void clear();
        void flush();

        /**
         * \brief テクスチャ全体に対しての既にパックされた画像の占有率（100%）
         */
        double getCoverage() const {
            return coverage_;
        }

        void update(const size_t width, const size_t height, const bool is_new_container); // 画像のパッキング成功時の処理、第3引数（TRUE:新規コンテナを作成、FALSE:既存コンテナに追加）
        AtlasInfo insert(const size_t width, const size_t height); // 指定された画像領域（width x height）の領域が確保できるか検証、戻り値AtrasInfoの「valid」ブール値（true:成功、false:失敗）で判定可能

    private:
        std::vector<AtlasContainer> container_list_;
        size_t canvas_width_;
        size_t canvas_height_;
        size_t vertical_range_;
        size_t capacity_;
        double coverage_;
        TextureImage canvas_;
        std::string save_file_path_;
    };

    /// テクスチャのアトラス化をします。
    /// 実装上の注意：
    /// TexturePacker にAPIを増やすとき、変更が必要なのは texture_packer.cpp に加えて texture_packer_dummy.cpp もであることに注意してください。
    class LIBPLATEAU_EXPORT TexturePacker {
    public:
        const int default_resolution = 2048;

        explicit TexturePacker(size_t width, size_t height, const int internal_canvas_count = 8);

        ~TexturePacker();

        void process(plateau::polygonMesh::Model& model);
        void processNodeRecursive(const plateau::polygonMesh::Node& node);
        void processMesh(plateau::polygonMesh::Mesh* mesh);

    private:
        std::vector<TextureAtlasCanvas> canvases_;
        size_t canvas_width_;
        size_t canvas_height_;
    };
} // namespace plateau::texture
