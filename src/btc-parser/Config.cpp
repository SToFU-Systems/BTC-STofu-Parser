// Precompiled headers
#include "pch.h"

// Project headers
#include "Config.h"
#include "Logger.hpp"
#include "Utils.h"


//================================================================================
// Method: init
// Description: Initializes the Config object. Handles command-line arguments,
//              loads JSON configuration, and extracts data from it. Returns 
//              AppErrorCode indicating success or the first error encountered.
//================================================================================
AppErrorCode Config::init(IN int argc, IN wchar_t* argv[])
{
    // Handle command arguments
    const AppErrorCode kHandlerResult = handleCommandArguments(argc, argv);
    RETURN_IF_FAILED(kHandlerResult);

    // Json
    std::ifstream jsonFile(argv[1]);
    nlohmann::json jsonData = nlohmann::json::parse(jsonFile);
    
    // Get Block Directory
    const AppErrorCode kBlockParseResult = parseBlockDirectory(jsonData);
    RETURN_IF_FAILED(kBlockParseResult);

    // Get Xor Directory
    const AppErrorCode kXorParseResult = parseXorDirectory(jsonData);
    RETURN_IF_FAILED(kXorParseResult);

    // Get Output Directory
    const AppErrorCode kOutputParseResult = parseOutputDirectory(jsonData);
    RETURN_IF_FAILED(kOutputParseResult);

    LOG_INFO("Successfully initialized config data.");
    return AppErrorCode::Success;
}

//================================================================================
// Method: getBlockPath
// Description: Returns the configured block directory path as wide string.
//================================================================================
std::wstring Config::getBlockPath() const noexcept
{
    return m_blockDirectoryPath;
}

//================================================================================
// Method: getXorPath
// Description: Returns the configured XOR directory path as wide string.
//================================================================================
std::wstring Config::getXorPath() const noexcept
{
    return m_xorDirectoryPath;
}

//================================================================================
// Method: getOutputPath
// Description: Returns the configured output directory path as wide string.
//================================================================================
std::wstring Config::getOutputPath() const noexcept
{
    return m_outputDirectoryPath;
}

//================================================================================
// Method: setBlockPath
// Description: Checks and sets the block directory path to the provided value.
//================================================================================
AppErrorCode Config::setBlockPath(IN const std::wstring& newPath)
{
    if (!std::filesystem::exists(newPath))
    {
        LOG_ERROR("Failed to set {} as block directory path", newPath);
        return AppErrorCode::DirectoryNotFound;
    }

    m_blockDirectoryPath = newPath;
    return AppErrorCode::Success;
}

//================================================================================
// Method: setXorPath
// Description: Checks and sets the XOR path to the provided value.
//================================================================================
AppErrorCode Config::setXorPath(IN const std::wstring& newPath)
{
    if (!std::filesystem::exists(newPath))
    {
        LOG_ERROR("Failed to set {} as XOR directory path", newPath);
        return AppErrorCode::DirectoryNotFound;
    }

    m_xorDirectoryPath = newPath;
    return AppErrorCode::Success;
}

//================================================================================
// Method: setOutputPath
// Description: Sets the output path to the provided value.
//================================================================================
AppErrorCode Config::setOutputPath(IN const std::wstring& newPath)
{
    if (!std::filesystem::exists(newPath))
    {
        LOG_ERROR("Failed to set {} as output directory path", newPath);
        return AppErrorCode::DirectoryNotFound;
    }

    m_outputDirectoryPath = newPath;
    return AppErrorCode::Success;
}

//================================================================================
// Method: handleCommandArguments
// Description: Validates command-line arguments. Ensures JSON file path is passed
//              and it exists on disk. Returns Success on valid input or an error 
//              code otherwise.
//================================================================================
AppErrorCode Config::handleCommandArguments(IN int argc, IN wchar_t* argv[]) const
{
    constexpr size_t kExpectedArgumentsNum = 2;

    if (argc != kExpectedArgumentsNum)
    {
        LOG_ERROR("Incorrect number of command arguments. Expected: {}, but received: {}", kExpectedArgumentsNum, argc);
        return AppErrorCode::CommandArgumentsError;
    }

    if (!std::filesystem::exists(argv[1]))
    {
        LOG_ERROR("Config file not found.");
        return AppErrorCode::FileNotFound;
    }

    return AppErrorCode::Success;
}

//================================================================================
// Method: parseBlockDirectory
// Description: Reads the Block Directory key from the given JSON, validates that
//              a non-empty wide string was parsed, and applies it via setBlockPath.
//              Returns Success if applied or the specific error from parsing or
//              validation if unsuccessful.
//================================================================================
AppErrorCode Config::parseBlockDirectory(IN const nlohmann::json& json)
{
    constexpr const char* kBlockDirectoryJsonKey = "BlockDirectory";

    const auto kParsedBlockDirectory = parseWstringFromJson(json, kBlockDirectoryJsonKey);
    RETURN_ERROR_CODE_IF_UNEXPECTED(kParsedBlockDirectory);

    return setBlockPath(kParsedBlockDirectory.value());
}

//================================================================================
// Method: parseXorDirectory
// Description: Reads the Xor Directory key from the given JSON, validates that
//              a non-empty wide string was parsed, and applies it via setXorPath.
//              Returns Success if applied or the specific error from parsing or
//              validation if unsuccessful.
//================================================================================
AppErrorCode Config::parseXorDirectory(IN const nlohmann::json& json)
{
    constexpr const char* kXorDirectoryJsonKey = "XorDirectory";

    const auto kParsedXorDirectory = parseWstringFromJson(json, kXorDirectoryJsonKey);
    RETURN_ERROR_CODE_IF_UNEXPECTED(kParsedXorDirectory);

    return setXorPath(kParsedXorDirectory.value());
}

//================================================================================
// Method: parseOutputDirectory
// Description: Reads the Output Directory key from the given JSON, validates that
//              a non-empty wide string was parsed, and applies it via setOutputPath.
//              Returns false if the key is missing/invalid or if applying the path
//              fails; otherwise returns true.
//================================================================================
AppErrorCode Config::parseOutputDirectory(IN const nlohmann::json& json)
{
    constexpr const char* kOutputDirectoryJsonKey = "OutputDirectory";

    const auto kParsedOutputDirectory = parseWstringFromJson(json, kOutputDirectoryJsonKey);
    RETURN_ERROR_CODE_IF_UNEXPECTED(kParsedOutputDirectory);

    return setOutputPath(kParsedOutputDirectory.value());
}

//================================================================================
// Method: parseWstringFromJson
// Description: Attempts to parse a wide string from a JSON object by key.
//              Returns the converted value on success or AppErrorCode on failure.
//================================================================================
std::expected<std::wstring, AppErrorCode> Config::parseWstringFromJson(
    IN const nlohmann::json& json, 
    IN const std::string& jsonKey
) const
{
    const bool kIsKeyPresent = isJsonKeyPresent(json, jsonKey);
    if (!kIsKeyPresent)
        return std::unexpected(AppErrorCode::JsonKeyNotFound);

    std::string kParsedString = json[jsonKey].get<std::string>();
    return stringUtf8ToWide(kParsedString);
}

//================================================================================
// Method: isJsonKeyPresent
// Description: Checks if the given key exists in the JSON object. Returns true 
//              if present, false otherwise.
//================================================================================
bool Config::isJsonKeyPresent(IN const nlohmann::json& json, IN const std::string& jsonKey) const
{
    if (!json.contains(jsonKey))
    {
        LOG_ERROR("Couldn't find {} key in JSON", jsonKey);
        return false;
    }

    return true;
}
