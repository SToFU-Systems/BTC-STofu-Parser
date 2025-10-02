#pragma once

// Standard Library headers
#include <format>
#include <string>
#include <string_view>

// 3rd-party libraries
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>

// Project headers
#include "Common.h"
#include "LoggerUtils.h"

//================================================================================
// Enum: LOG_TYPE
// Description: Defines supported logging levels from verbose to critical. The
//              LOG_TYPE_MAX value indicates the total number of levels.
//================================================================================
enum class LOG_TYPE
{
    LOG_VERBOSE = spdlog::level::trace,       // Detailed debug information.
    LOG_INFO = spdlog::level::info,           // Application is working as expected.
    LOG_WARNING = spdlog::level::warn,        // Indications of potential issues or unexpected events that don't stop execution.
    LOG_ERROR = spdlog::level::err,           // Error events that cause some functionality to fail but do not crash the application.
    LOG_CRITICAL = spdlog::level::critical,   // Severe error events that will lead the application to abort.
    LOG_TYPE_MAX = LOG_CRITICAL + 1           // Value to represent the total number of log types (not a log level).
};


//=========================================================================================
// CONSTANS
//=========================================================================================
static constexpr const char* kLogTypes[] = { "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL" };
inline constexpr size_t kMaxLogFiles = 6;
inline constexpr uint32_t kDefaultLogLevel = static_cast<uint32_t>(LOG_TYPE::LOG_INFO);


//=========================================================================================
// MACROSES
//=========================================================================================
#define LOG_FMT(level, ...) \
        Logger::Instance().getLogger()->log( \
        level, \
        std::format("[{:<8}] {} | {} | {} | {}", \
            kLogTypes[static_cast<int>(level)], __func__, __FILE__, __LINE__, format_utf8(__VA_ARGS__)) \
    );\
           Logger::Instance().getLogger()->flush();

#define LOG_VERBOSE(...)  LOG_FMT(spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)    LOG_FMT(spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)     LOG_FMT(spdlog::level::info, __VA_ARGS__)
#define LOG_WARNING(...)  LOG_FMT(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)    LOG_FMT(spdlog::level::err, __VA_ARGS__)
#define LOG_CRITICAL(...) LOG_FMT(spdlog::level::critical, __VA_ARGS__)


//================================================================================
// Class: Logger
// Description: Provides logging functionality. Accumulates messages via << operator
//              and writes them to a UTF-8 encoded log file with timestamps.
//================================================================================
class Logger
{
public:

    //================================================================================
    // Function: Init
    // Description: Initializes the m_logger instance with both daily file sink and
    //              colored console sink. Applies the specified log level
    //================================================================================
    static bool Init(IN const std::string& pathDir, IN LOG_TYPE level = LOG_TYPE::LOG_VERBOSE);


    //================================================================================
    // Method: Logger::Instance
    // Description: Provides access to the singleton m_logger instance.
    //================================================================================
    static Logger& Instance();


    //================================================================================
    // Method: getLogLevel
    // Description: Returns log level value from registry.
    //================================================================================
    uint32_t getLogLevel();


    //================================================================================
    // Method: setLogLevel
    // Description: Sets log level value in registry.
    //================================================================================
    [[nodiscard]] bool setLogLevel(IN uint32_t logLevel);


    //================================================================================
    // Method: Logger::getLogger
    // Description: Returns a shared pointer to the underlying spdlog m_logger.
    //================================================================================
    std::shared_ptr<spdlog::logger> getLogger();


    //================================================================================
    // Method: Logger::setPathDir
    // Description: Sets directory where log files will be stored.
    //================================================================================
    void setPathDir(IN const std::string& pathDir);


    //================================================================================
    // Method: Logger::setCompany
    // Description: Switches log output to a company?specific daily file.
    //================================================================================
    void setCompany(IN const std::string& company);

private:

    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;


    //================================================================================
    // Method: Logger::makeFileName
    // Description: Builds full path "<dir>/<company> <YYYY-MM-DD>.txt".
    //================================================================================
    std::string makeFileName(IN const std::string& company) const;


    //================================================================================
    // Method: isLogLevelValid
    // Description: Checks if the input log level value is within allowed range.
    //              If out of range, logs a warning and returns false.
    //================================================================================
    [[nodiscard]] bool isLogLevelValid(IN uint32_t value) const noexcept;


    //================================================================================
    // Method: isRegistryResultValid
    // Description: Checks if a Windows registry operation succeeded. Returns true 
    //              if result equals ERROR_SUCCESS, otherwise false.
    //================================================================================
    [[nodiscard]] bool isRegistryResultValid(IN LSTATUS result, IN std::string_view operation) const noexcept;

private:
    std::string m_logPathDir;

    static constexpr const wchar_t* kRegSubKeyPath = L"SOFTWARE\\E-Chat\\BitcoinParser";
    static constexpr const wchar_t* kRegLogName = L"LogLevel";
    static constexpr char kSpdLogPattern[] = "[%H:%M:%S.%f]: %v";

    std::shared_ptr<spdlog::sinks::dist_sink_mt> m_distSink;
    std::shared_ptr<spdlog::sinks::daily_file_sink_mt> m_fileSink;

    uint32_t m_logLevel;
    std::shared_ptr<spdlog::logger> m_logger;

};
