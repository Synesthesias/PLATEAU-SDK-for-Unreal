#pragma once
#include <plateau/polygon_mesh/model.h>

namespace plateau::granularityConvert {
    class IModelConverter {
    public:
        virtual plateau::polygonMesh::Model convert(const plateau::polygonMesh::Model* src) const = 0;
        virtual ~IModelConverter() = default;
    };
}
