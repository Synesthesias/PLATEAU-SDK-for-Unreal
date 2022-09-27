#pragma once

#include <string>
#include <fstream>

#include <citygml/citygml.h>
#include <libplateau_api.h>
#include <plateau_dll_logger.h>
#include <plateau/io/mesh_convert_options.h>

class LIBPLATEAU_EXPORT ObjWriter {
public:
    ObjWriter() :
            v_offset_(0), uv_offset_(0) {
    }

    bool write(
            const std::string& obj_file_path,
            const std::string& gml_file_path,
            const citygml::CityModel& city_model,
            MeshConvertOptions options, unsigned lod,
            std::shared_ptr<PlateauDllLogger> logger = nullptr);

    static TVec3d
    convertPosition(const TVec3d& position, const TVec3d& reference_point, const CoordinateSystem axes, float unit_scale);

private:
    // OBJ書き出し
    void writeObj(const std::string& obj_file_path, const citygml::CityModel& city_model, unsigned lod);
    void writeCityObjectRecursive(std::ofstream& ofs, const citygml::CityObject& target_object, unsigned lod);
    void writeCityObject(std::ofstream& ofs, const citygml::CityObject& target_object, unsigned lod);
    void writeGeometry(std::ofstream& ofs, const citygml::Geometry& target_geometry);
    void writeVertices(std::ofstream& ofs, const std::vector<TVec3d>& vertices);
    void writeIndices(std::ofstream& ofs, const std::vector<unsigned int>& indices);
    void writeIndicesWithUV(std::ofstream& ofs, const std::vector<unsigned int>& indices);
    void writeUVs(std::ofstream& ofs, const std::vector<TVec2f>& uvs);
    void writeMaterialReference(std::ofstream& ofs, const std::shared_ptr<const citygml::Texture>& texture);

    // MTL書き出し
    void writeMtl(const std::string& obj_file_path);

    MeshConvertOptions options_;

    std::map<std::string, std::shared_ptr<const citygml::Texture>> required_materials_;
    unsigned v_offset_, uv_offset_;

    std::weak_ptr<PlateauDllLogger> dll_logger_;
};
