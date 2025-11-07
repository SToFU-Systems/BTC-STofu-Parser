// Precompiled headers
#include "pch.h"

// Project headers
#include "AppErrorCode.hpp"
#include "BitcoinReader.h"
#include "BlockFileParser.h"
#include "BlockPrinter.h"
#include "Common.h"
#include "Config.h"
#include "DataTypes.h"
#include "Logger.hpp"
#include "SingleBlockParser.h"
#include "Utils.h"
#include "CsvParser.h"

Config config;

//================================================================================
// Function: failMessage
// Description: Prints an error message to stderr using std::format. The message
//              includes the error code value, its string description, and a
//              custom error message. Returns the numeric error code.
//================================================================================
static int failMessage(const AppErrorCode code, const std::string& msg)
{
    std::error_code ec = code;
    std::cerr << std::format("[ERROR] [Code {}] {} | {}\n", ec.value(), ec.message(), msg);
    return ec.value();
}

//================================================================================
// Function: failMessageLog
// Description: Logs an error message to the configured logger. The log includes 
//              the error code value, its string description, and a custom error 
//              message. Returns the numeric error code.
//================================================================================
static int failMessageLog(const AppErrorCode code, const std::string& msg)
{
    std::error_code ec = code;
    LOG_ERROR("[Code {}] {} | {}", ec.value(), ec.message(), msg);
    return ec.value();
}

//================================================================================
// Function: setLogger
// Description: Initializes the logger with the specified log directory path. 
//              The log level is set to LOG_VERBOSE in debug builds, and LOG_INFO
//              in release builds. Returns true if logger initialization is 
//              successful, false otherwise.
//================================================================================
bool setLogger(const std::string& logPath)
{
    LOG_TYPE level = LOG_TYPE::LOG_INFO;

#ifdef _DEBUG
    level = LOG_TYPE::LOG_VERBOSE;
#endif // _DEBUG

    return Logger::Init(logPath, level);;
}

//================================================================================
// Function: wmain
// Description: Application entry point. Initializes logger, loads configuration
//              from the provided path, parses Bitcoin block files, and outputs 
//              results into JSON file.
//              Command line usage: <path to config.json>
//================================================================================
//int wmain(int argc, wchar_t* argv[])
//{
//    // Set logger
//    constexpr const char* kLogDir = "../../log/";
//    if(!setLogger(kLogDir))
//        return failMessage(AppErrorCode::LoggerInitFailure,
//            "Failed to initialize logger");
//
//    // Config
//    const AppErrorCode kConfigInitResult = config.init(argc, argv);
//    if (kConfigInitResult != AppErrorCode::Success)
//        return failMessageLog(kConfigInitResult, "Received incorrect command arguments");
//
//    const std::wstring kBlockDirectory = config.getBlockPath();
//    const std::wstring kXorPath = config.getXorPath();
//    const std::wstring kOutputPath = config.getOutputPath();
//
//    try
//    {
//        // Get list of btc files
//        LOG_VERBOSE("Extracting list of block files in \'{}\'...", kBlockDirectory);
//        std::vector<std::wstring> blocksPath = getBlockFilesInDirectory(kBlockDirectory);
//        LOG_VERBOSE("Found {} block files.", blocksPath.size());
//
//        for (const std::wstring& kBlockPath : blocksPath)
//        {
//            LOG_INFO("Started parsing block file: \'{}\'...", kBlockPath);
//
//            // Parse btc file
//            BitcoinReader reader(kBlockPath, kXorPath);
//            BlockFileParser parser(reader);
//
//            auto kParseResult = parser.parse();
//            if (!kParseResult.has_value())
//                return failMessageLog(kParseResult.error(), "main()");
//            
//            std::vector<Block> blocks = std::move(kParseResult).value();
//            LOG_VERBOSE("Successfully parsed block file."); 
//
//            // Output parsing result to file
//            const std::wstring kFileName = std::filesystem::path(kBlockPath).stem();
//            const std::wstring kFilePath = kOutputPath + kFileName + L".json";
//            std::ofstream outFile(kFilePath);
//
//            LOG_VERBOSE("Printing results to file...", kBlockPath);
//            for (const Block& block : blocks)
//            {
//                BlockPrinter printer(block, outFile, true);
//                printer.printBlock();
//            }
//
//            LOG_INFO("Parsing is finished successfully. Output results printed in: {}", kOutputPath);
//        }
//    }
//    catch (const std::runtime_error& e)
//    {
//        return failMessageLog(AppErrorCode::RuntimeException, e.what());
//    }
//    catch(const std::exception& e)
//    {
//        return failMessageLog(AppErrorCode::UnknownException, e.what());
//    }
//    catch (...)
//    {
//        return failMessageLog(AppErrorCode::UnknownException, "Unknown exception in main()");
//    }
//
//    return static_cast<int>(AppErrorCode::Success);
//}

int main()
{
    std::cout << "Downloading...\n";
    if (!DownloadKaggleCsv()) {
        std::cerr << "Download failed\n";
        return 1;
    }
    std::cout << "Saved: " << outPath << "\n";

    std::cout << "Parsing CSV...\n";
    std::map<std::string, double> priceMap = LoadCsvToMap(outPath);

    return 0;
}