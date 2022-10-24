#pragma once

#include <libplateau_api.h>
#include <plateau/io/mesh_convert_options.h>
#include <citygml/citymodel.h>

class LIBPLATEAU_EXPORT MeshConvertOptionsFactory {
public:
    static void setValidReferencePoint(MeshConvertOptions& options, const citygml::CityModel& city_model);
};
