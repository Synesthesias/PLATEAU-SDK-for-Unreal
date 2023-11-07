#pragma once
#include <plateau/granularity_convert/i_model_converter.h>

namespace plateau::granularityConvert {


    class ConvertFromAtomicToArea : public IModelConverter {
    public:
        /// 最小地物単位のモデルを受け取り、それを地域単位に変換したモデルを返します。
        plateau::polygonMesh::Model convert(const plateau::polygonMesh::Model* src) const override;
    };
}
