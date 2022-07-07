#pragma once

#include <string>
#include <vector>
#include <fstream>

#include <citygml/citymodel.h>
#include <libplateau_api.h>
#include <plateau_dll_logger.h>
#include <plateau/io/mesh_convert_options.h>

/**
 * \brief GMLファイルをメッシュファイルに変換する機能を提供します。
 */
class LIBPLATEAU_EXPORT MeshConverter {
public:
    MeshConverter() = default;

    /**
     * \brief GMLファイルをメッシュファイルに変換します。
     *
     * 変換後のメッシュファイル(OBJもしくはglTF)は出力先ディレクトリに<em>LOD{LODの値}_{gmlファイル名}.{拡張子}</em>という名前で格納され、
     * .gmlから参照されるテクスチャファイル一式は出力先ディレクトリにコピーされます。
     *
     * 引数として指定するcity_modelは以下を満たしている必要があります。
     * - tessellateオプションがtrueでパースされていること
     * - 入力GMLファイルがパースされた都市モデルであること
     *
     * \param destination_directory 出力先ディレクトリ
     * \param gml_file_path 入力GMLファイル
     * \param city_model 入力GMLファイルをパースした都市モデル。nullptrを入力した場合は内部でパースされます。
     */
    void convert(const std::string& destination_directory, const std::string& gml_file_path, std::shared_ptr<const citygml::CityModel> city_model = nullptr, std::shared_ptr<PlateauDllLogger> logger = nullptr) const;

    /**
     * \brief GMLファイルをメッシュファイルに変換します。
     *
     * 変換後のメッシュファイル(OBJもしくはglTF)は出力先ディレクトリに<em>LOD{LODの値}_{gmlファイル名}.{拡張子}</em>という名前で格納され、
     * .gmlから参照されるテクスチャファイル一式は出力先ディレクトリにコピーされます。
     *
     * 引数として指定するcity_modelは以下を満たしている必要があります。
     * - tessellateオプションがtrueでパースされていること
     * - 入力GMLファイルがパースされた都市モデルであること
     *
     * \param [in] destination_directory 出力先ディレクトリ
     * \param [in] gml_file_path 入力GMLファイル
     * \param [out] converted_files 変換後のメッシュファイル
     * \param [in] city_model 入力GMLファイルをパースした都市モデル。nullptrを入力した場合は内部でパースされます。
     */
    void convert(const std::string& destination_directory, const std::string& gml_file_path,
        std::vector<std::string>& converted_files, std::shared_ptr<const citygml::CityModel> city_model = nullptr, std::shared_ptr<PlateauDllLogger> logger = nullptr) const;

    /**
     * \brief メッシュ変換オプションを取得します。
     * \return コピーされたメッシュ変換オプション
     */
    [[nodiscard]] MeshConvertOptions getOptions() const;

    /**
     * \brief メッシュ変換オプションを設定します。
     * \param options メッシュ変換オプション
     */
    void setOptions(const MeshConvertOptions& options);

private:
    MeshConvertOptions options_;
};
