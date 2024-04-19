#pragma once

#include <string>

#include <citygml/citygml.h>
#include <libplateau_api.h>
#include <stack>

#include <plateau/polygon_mesh/mesh_extractor.h>
#include <plateau/polygon_mesh/transform.h>

namespace plateau::meshWriter {

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
