#pragma once
#include "i_model_converter.h"

namespace plateau::granularityConvert {
    class ConvertFromAtomicToPrimary : public IModelConverter {
    public:

        /// 最小地物単位のモデルを受け取り、それを主要地物単位に変換したモデルを返します。
        plateau::polygonMesh::Model convert(const plateau::polygonMesh::Model* src) const override;
    };
}
