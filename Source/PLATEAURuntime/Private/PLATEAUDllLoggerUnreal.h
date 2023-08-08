// Copyright 2023 Ministry of Land, Infrastructure and Transport
#pragma once
#include <plateau_dll_logger.h>

/**
 * @brief PLATEAU共通ライブラリ内で発生したログをUnreal Engineに渡すロガーです。
 */
class PLATEAUDllLoggerUnreal
{
public:
    explicit PLATEAUDllLoggerUnreal(citygml::CityGMLLogger::LOGLEVEL LogLevel);
    
    /**
     * @brief PLATEAU共通ライブラリに渡すべきロガーを返します。 
     */
    std::shared_ptr<PlateauDllLogger> GetLogger();
private:
    std::shared_ptr<PlateauDllLogger> DllLogger;
    static void LogInfo(const char* Char);
    static void LogWarning(const char* Char);
    static void LogError(const char* Char);
};
