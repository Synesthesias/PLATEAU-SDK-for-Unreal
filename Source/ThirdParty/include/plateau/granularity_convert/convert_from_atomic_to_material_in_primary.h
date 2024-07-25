#pragma once
#include <plateau/granularity_convert/i_model_converter.h>

namespace plateau::granularityConvert {
    class ConvertFromAtomicToMaterialInPrimary : public IModelConverter {
    public:
        plateau::polygonMesh::Model convert(const plateau::polygonMesh::Model* src) const override;
    };
}