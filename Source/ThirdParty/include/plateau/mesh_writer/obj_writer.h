#pragma once

#include <string>

#include <citygml/citygml.h>
#include <libplateau_api.h>
#include <stack>

#include <plateau/polygon_mesh/mesh_extractor.h>
#include <plateau/polygon_mesh/transform.h>

namespace plateau::meshWriter {

    /// ゲームオブジェクトの位置のスタックです。
    /// Nodeの親から子へと処理が進むたびpushし、子から親へ処理が戻るたびpopします。
    /// こうすることで、親から子まで累積的にTransformを適用することができます。
    class TransformStack {
    public:
        void push(const polygonMesh::Transform& transform) {
            stack_.push_back(transform);
        }

        polygonMesh::Transform pop() {
            if (stack_.empty()) {
                throw std::runtime_error("TransformStack is empty.");
            }
            auto transform = stack_.at(stack_.size() - 1);
            stack_.pop_back();
            return transform;
        }

        /// スタック内の全Transformを掛け合わせて積を計算します。
        polygonMesh::Transform CalcProduct() {
            auto current = glm::mat4(1.0f);
            for(int i=stack_.size() - 1; i>=0; i--) {
                current = stack_.at(i).toGlmMatrix() * current;
            }
            return polygonMesh::Transform::fromGlmMatrix(current);
        }


    private:
        std::vector<polygonMesh::Transform> stack_;
    };

    /// ModelをもとにOBJファイルを書き出すクラスです。
    class LIBPLATEAU_EXPORT ObjWriter {
    public:
        ObjWriter() :
            v_offset_(0), uv_offset_(0), required_materials_() {
        }

        bool write(const std::string& obj_file_path, const plateau::polygonMesh::Model& model);

    private:
        // OBJ書き出し
        void writeObj(const std::string& obj_file_path, const plateau::polygonMesh::Node& node,
                      TransformStack& transform_stack);
        void writeCityObjectRecursive(std::ofstream& ofs, const plateau::polygonMesh::Node& node, TransformStack& transform_stack);
        void writeCityObject(std::ofstream& ofs, const plateau::polygonMesh::Node& node, TransformStack& transform_stack);
        static void writeVertices(std::ofstream& ofs, const std::vector<TVec3d>& vertices, TransformStack& transform_stack);
        void writeIndicesWithUV(std::ofstream& ofs, const std::vector<unsigned int>& indices) const;
        static void writeUVs(std::ofstream& ofs, const std::vector<TVec2f>& uvs);
        void writeMaterialReference(std::ofstream& ofs, const std::string& texUrl);

        // MTL書き出し
        void writeMtl(const std::string& obj_file_path);


        unsigned v_offset_, uv_offset_;
        std::map<std::string, std::string> required_materials_;

    };
}
