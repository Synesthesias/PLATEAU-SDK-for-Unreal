#pragma once

#include <filesystem>
#include <libplateau_api.h>

namespace plateau::dataset {

    struct LIBPLATEAU_EXPORT LodFlag;

    /**
     * \brief GMLファイルに含まれるLOD番号を検索します。
     * ファイルの中身を文字列検索し、":lod(番号)" にヒットした番号をフラグ形式で返します。
     */
    class LIBPLATEAU_EXPORT LodSearcher {
    public:
        static plateau::dataset::LodFlag searchLodsInFile(const std::filesystem::path& file_path);
        static plateau::dataset::LodFlag searchLodsInIstream(std::istream& ifs);
    };

    /// どのLODが含まれるかをフラグ(unsigned)で表現します。
    struct LIBPLATEAU_EXPORT LodFlag {
    public:
        /// 下から n ビット目を1にします。
        void setFlag(unsigned digit);
        /// 下から n ビット目を0にします。
        void unsetFlag(unsigned digit);
        unsigned getFlag() const;
        /// 立っているフラグのうちもっとも上位のものが何ビット目かを返します。
        /// フラグがどれも0なら-1を返します。
        int getMax() const;
        /// searchLodsInFile の実装の都合上、LODは1桁とします。
        static const int max_lod_ = 9;

    private:
        /// lod n が含まれるとき、flags の下から n ビット目が立ちます。
        unsigned flags_ = 0;
    };
}
