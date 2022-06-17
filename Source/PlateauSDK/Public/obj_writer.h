#pragma once
#include <string>
#include <fstream>

#include <citygml/citygml.h>
#include <libplateau_api.h>

enum class AxesConversion {
    WNU,
    RUF
};

class LIBPLATEAU_EXPORT ObjWriter {
public:
    ObjWriter() : ofs_() {
    }

    void write(const std::string& obj_file_path, const citygml::CityModel& city_model, const std::string& gml_file_path);
    void setMergeMeshFlg(bool value);
    bool getMergeMeshFlg() const;
    void setDestAxes(AxesConversion value);
    AxesConversion getDestAxes() const;
    void setValidReferencePoint(const citygml::CityModel& city_model);
    void getReferencePoint(double xyz[]) const;
    void setReferencePoint(const double xyz[]);

private:
    unsigned int writeVertices(const std::vector<TVec3d>& vertices);
    void writeIndices(const std::vector<unsigned int>& indices, unsigned int ix_offset, unsigned int tx_offset, bool tex_flg);
    unsigned int writeUVs(const std::vector<TVec2f>& uvs);
    void writeMaterial(const std::string& tex_path);
    void processChildCityObject(const citygml::CityObject& target_object, unsigned int& v_offset, unsigned int& t_offset);
    void writeCityObject(const citygml::CityObject& target_object, unsigned int& v_offset, unsigned int& t_offset, bool recursive_flg);
    void writeGeometry(const citygml::Geometry& target_geometry, unsigned int& v_offset, unsigned int& t_offset, bool recursive_flg);

    std::ofstream ofs_;
    std::ofstream ofs_mat_;
    std::string gml_file_path_, obj_file_path_;
    std::vector<std::string> mat_list_;
    bool merge_mesh_flg_ = false;
    AxesConversion axes_ = AxesConversion::WNU;
    double ref_point_[3] = {0,0,0};
};
