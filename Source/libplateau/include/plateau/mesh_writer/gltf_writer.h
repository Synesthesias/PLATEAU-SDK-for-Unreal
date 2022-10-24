#pragma once
#include <string>
#include <fstream>

#include <citygml/citygml.h>
#include <libplateau_api.h>
#include <plateau_dll_logger.h>

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/BufferBuilder.h>
#include <GLTFSDK/GLTFResourceWriter.h>
#include <GLTFSDK/GLBResourceWriter.h>
#include <GLTFSDK/IStreamWriter.h>
#include <GLTFSDK/Serialize.h>

#include <plateau/polygon_mesh/mesh_extractor.h>
#include <plateau/polygon_mesh/polygon_mesh_utils.h>


namespace plateau::meshWriter {
    /**
     * @enum MeshFileFormat
     *
     * 出力ファイルフォーマット
     */
    enum class MeshFileFormat {
        GLB,
        GLTF
    };
    /**
     * \brief glTF出力の設定です。
     */
    struct GltfWriteOptions {

        /**
         * \brief 出力ファイルフォーマットを指定します。
         */
        MeshFileFormat mesh_file_format;

        /**
         * \brief テクスチャファイル用のディレクトリをgltfファイルからの相対パスで指定します。
         * NULLの場合はgltfファイルと同じ階層にテクスチャファイルを保存します。
         */
        std::string texture_directory_path;

        GltfWriteOptions() :
            mesh_file_format(MeshFileFormat::GLB), texture_directory_path("") {
        }
    };

    class LIBPLATEAU_EXPORT GltfWriter {
    public:
        GltfWriter() :
            image_id_num_(0), texture_id_num_(0), node_name_(""), scene_(), mesh_(),
            material_ids_(), current_material_id_(), default_material_id_(""), required_materials_(), options_() {
        }

        bool write(const std::string& destination, const plateau::polygonMesh::Model& model, GltfWriteOptions options);

    private:
        void precessNodeRecursive(const plateau::polygonMesh::Node& node, Microsoft::glTF::Document& document, Microsoft::glTF::BufferBuilder& bufferBuilder);
        std::string writeMaterialReference(std::string texUrl, Microsoft::glTF::Document& document);
        void writeNode(Microsoft::glTF::Document& document);
        void writeMesh(std::string accessorIdPositions, std::string accessorIdIndices, std::string accessorIdTexCoords, Microsoft::glTF::BufferBuilder& bufferBuilder);

        std::map<std::string, std::string> required_materials_;
        Microsoft::glTF::Scene scene_;
        Microsoft::glTF::Mesh mesh_;
        std::string node_name_;
        int image_id_num_, texture_id_num_;
        std::map<std::string, std::string> material_ids_;
        std::string default_material_id_, current_material_id_;
        GltfWriteOptions options_;
    };
}
