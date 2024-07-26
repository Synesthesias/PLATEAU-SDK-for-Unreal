#pragma once
#include <libplateau_api.h>
#include <citygml/vecs.hpp>

namespace plateau::heightMapGenerator {

    class LIBPLATEAU_EXPORT HeightmapOperation {
    public:
        static double getPositionFromPercent(double percent, double min, double max);
        static TVec2d getPositionFromPercent(const TVec2d& percent, const TVec2d& min, const TVec2d& max);

    };
}