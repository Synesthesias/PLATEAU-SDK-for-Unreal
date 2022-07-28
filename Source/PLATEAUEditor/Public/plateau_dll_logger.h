#pragma once
#include <libplateau_api.h>
#include <citygml/citygmllogger.h>

using namespace citygml;

typedef CityGMLLogger::LOGLEVEL DllLogLevel;
typedef void(*LogCallbackFuncPtr)(const char*);

/**
 * @brief DLL内のログをDLLの利用者に渡すことを目的として CityGMLLogger を拡張したものです。
 * log()のコール時に、コールバックによってDLL利用者に文字列を渡します。
 */
class LIBPLATEAU_EXPORT PlateauDllLogger : public CityGMLLogger {
public:
    /// Error, Warn, Info それぞれのコールバックを指定し、
    /// どのログレベル以上を表示するかを指定します。
    explicit PlateauDllLogger(
            LOGLEVEL level = DllLogLevel::LL_INFO,
            LogCallbackFuncPtr log_error_callback = nullptr,
            LogCallbackFuncPtr log_warn_callback = nullptr,
            LogCallbackFuncPtr log_info_callback = nullptr
    ) : CityGMLLogger(level)
        , log_error_callback_(log_error_callback)
        , log_warn_callback_(log_warn_callback)
        , log_info_callback_(log_info_callback) {
    };

    void log(DllLogLevel level, const std::string& message, const char* file = nullptr, int line = -1) const override;
    void setLogCallbacks(LogCallbackFuncPtr error_callback, LogCallbackFuncPtr warn_callback, LogCallbackFuncPtr info_callback);

    /// エラーレベルの log() をコールした後に例外を出します。
    void throwException(const std::string& message);

private:
    LogCallbackFuncPtr log_error_callback_;
    LogCallbackFuncPtr log_warn_callback_;
    LogCallbackFuncPtr log_info_callback_;
};