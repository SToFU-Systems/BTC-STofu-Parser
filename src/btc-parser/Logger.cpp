//Precompiled Headers
#include "pch.h"

// 3rd-party libraries
#include <spdlog/sinks/stdout_color_sinks.h>

// Project headers
#include "Logger.hpp"


constexpr uint32_t kMaxLogTypes = static_cast<uint32_t>(LOG_TYPE::LOG_TYPE_MAX);

//================================================================================
// Method: Init
// Description: Fully initialises the m_logger: sets directory, log level, console
//              sink and daily file sink. Must be called once at program start.
//================================================================================
bool Logger::Init(IN const std::string& pathDir, IN LOG_TYPE level)
{
    try
    {
        auto& instance = Instance();
        instance.setPathDir(pathDir);
        instance.setLogLevel(static_cast<uint32_t>(level));

        spdlog::level::level_enum logLevel = static_cast<spdlog::level::level_enum>(level);

        // Dist sink fans-out records to multiple sinks (console + file)
        instance.m_distSink = std::make_shared<spdlog::sinks::dist_sink_mt>();

        const std::string fileName = instance.makeFileName("");

        // Daily rotation at 00:00, keep up to kMaxLogFiles
        instance.m_fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            fileName, 0, 0, false, kMaxLogFiles
        );

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        consoleSink->set_level(logLevel);
        consoleSink->set_pattern(kSpdLogPattern);

        instance.m_fileSink->set_level(logLevel);
        instance.m_fileSink->set_pattern(kSpdLogPattern);

        instance.m_distSink->add_sink(consoleSink);
        instance.m_distSink->add_sink(instance.m_fileSink);

        instance.m_logger = std::make_shared<spdlog::logger>("app", instance.m_distSink);
        instance.m_logger->set_level(logLevel);
        instance.m_logger->set_pattern(kSpdLogPattern);

        // Make this logger globally accessible for spdlog macros
        spdlog::register_logger(instance.m_logger);
        spdlog::set_default_logger(instance.m_logger);

        // Auto-flush on error-or-higher messages
        spdlog::flush_on(spdlog::level::err);
        
        return true;
    }
    catch (const spdlog::spdlog_ex& ex) 
    {
        std::cerr << "[Logger Init Error] " << ex.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) 
    {
        std::cerr << "[Logger Init Exception] " << e.what() << std::endl;
        return false;
    }
    catch (...) 
    {
        std::cerr << "[Logger Init Unknown Error]" << std::endl;
        return false;
    }
}

//================================================================================
// Method: Logger::Instance
// Description: Provides access to the singleton m_logger instance.
//================================================================================
Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

//================================================================================
// Method: getLogLevel
// Description: Returns log level value from registry.
//================================================================================
uint32_t Logger::getLogLevel()
{
    uint32_t logLevel;
    DWORD bufferSize = sizeof(logLevel);

    LONG valueResult = RegGetValueW(
        HKEY_LOCAL_MACHINE,
        kRegSubKeyPath,
        kRegLogName,
        RRF_RT_DWORD,
        nullptr,
        &logLevel,
        &bufferSize
    );

    if (!isRegistryResultValid(valueResult, "GetValue"))
    {
        // Registry read failed → use default
        logLevel = kDefaultLogLevel;
    }

    if (!isLogLevelValid(logLevel))
    {
        // Out-of-range value → use default
        logLevel = kDefaultLogLevel;
    }

    m_logLevel = logLevel;// cache
    return m_logLevel;
}

//================================================================================
// Method: setLogLevel
// Description: Sets log level value in registry.
//================================================================================
bool Logger::setLogLevel(IN uint32_t logLevel)
{
    // Clamp to default if value is out of range
    if (!isLogLevelValid(logLevel))
    {
        logLevel = kDefaultLogLevel;
        std::cout << "Used default value for LogLevel.\n";
    }

    // Open target registry key for writing
    HKEY key;
    LONG openResult = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        kRegSubKeyPath,
        0,
        KEY_WRITE,
        &key
    );

    if (!isRegistryResultValid(openResult, "OpenKey"))
        return false;

    // Persist the log level as REG_DWORD
    LONG valueResult = RegSetValueExW(
        key,
        kRegLogName,
        0,
        REG_DWORD,
        reinterpret_cast<LPBYTE>(&logLevel),
        sizeof(logLevel)
    );

    RegCloseKey(key);

    // Indicate whether the write succeeded
    return isRegistryResultValid(valueResult, "SetValue");
}

//================================================================================
// Method: Logger::getLogger
// Description: Returns a shared pointer to the underlying spdlog m_logger.
//================================================================================
std::shared_ptr<spdlog::logger> Logger::getLogger()
{
    return m_logger;
}

//================================================================================
// Method: Logger::setPathDir
// Description: Sets directory where log files will be stored.
//================================================================================
void Logger::setPathDir(IN const std::string& pathDir)
{
    m_logPathDir = pathDir;
}

//================================================================================
// Method: Logger::setCompany
// Description: Switches log output to a company?specific daily file.
//================================================================================
void Logger::setCompany(IN const std::string& company)
{
    if (m_fileSink)
    {
        m_fileSink->flush();// ensure last records are written
        m_distSink->remove_sink(m_fileSink);// detach old file sink
        m_fileSink.reset();
    }

    const std::string fileName = makeFileName(company);

    m_fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileName, 0, 0, false, 5);

    m_distSink->add_sink(m_fileSink);

    m_fileSink->set_pattern(kSpdLogPattern);
    m_fileSink->set_level(m_logger->level());
}

//================================================================================
// Method: Logger::makeFileName
// Description: Builds full path "<dir>/<company> <YYYY-MM-DD>.txt".
//================================================================================
std::string Logger::makeFileName(IN const std::string& company) const
{
    return std::format("{} {}.txt", m_logPathDir, company);
}

//================================================================================
// Method: isLogLevelValid
// Description: Checks if the input log level value is within allowed range.
//              If out of range, logs a warning and returns false.
//================================================================================
bool Logger::isLogLevelValid(IN uint32_t value) const noexcept
{
    if (value > kMaxLogTypes)
    {
        std::cout << "Warning: LogLevel is higher than allowed.\n";
        return false;
    }

    return true;
}

//================================================================================
// Method: isRegistryResultValid
// Description: Checks if a Windows registry operation succeeded. Returns true 
//              if result equals ERROR_SUCCESS, otherwise false.
//================================================================================
bool Logger::isRegistryResultValid(IN LSTATUS result, IN std::string_view operation) const noexcept
{
    if (result != ERROR_SUCCESS)
        return false;

    return true;
}
