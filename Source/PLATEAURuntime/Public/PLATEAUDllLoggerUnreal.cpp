#include "PLATEAUDllLoggerUnreal.h"

PLATEAUDllLoggerUnreal::PLATEAUDllLoggerUnreal(citygml::CityGMLLogger::LOGLEVEL LogLevel)
{
    DllLogger = std::make_shared<PlateauDllLogger>(LogLevel, &LogError, &LogWarning, &LogInfo);
}

std::shared_ptr<PlateauDllLogger> PLATEAUDllLoggerUnreal::GetLogger()
{
    return DllLogger;
}

void PLATEAUDllLoggerUnreal::LogInfo(const char* Char)
{
    const FString Str = FString(UTF8_TO_TCHAR(Char));
    UE_LOG(LogTemp, Log, TEXT("libplateau: %s"), *Str);
}

void PLATEAUDllLoggerUnreal::LogWarning(const char* Char)
{
    const FString Str = FString(UTF8_TO_TCHAR(Char));
    UE_LOG(LogTemp, Warning, TEXT("libplateau: %s"), *Str);
}

void PLATEAUDllLoggerUnreal::LogError(const char* Char)
{
    const FString Str = FString(UTF8_TO_TCHAR(Char));
    UE_LOG(LogTemp, Error, TEXT("libplateau: %s"), *Str);
}
