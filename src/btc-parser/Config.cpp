// Precompiled headers
#include "pch.h"

// Project headers
#include "Config.h"
#include "Utils.h"


//================================================================================
// Method: init
// Description: Initializes the Config object. Handles command-line arguments,
//              loads JSON configuration, and extracts data from it.
//================================================================================
bool Config::init(IN int argc, IN wchar_t* argv[])
{
    // Handle command arguments
    const bool kHandlerResult = handleCommandArguments(argc, argv);
    if (!kHandlerResult)
        return false;

    // Json
    std::ifstream jsonFile(argv[1]);
    nlohmann::json jsonData = nlohmann::json::parse(jsonFile);
    
    // Get Block Directory
    const bool kBlockParseResult = parseBlockDirectory(jsonData);
    if (!kBlockParseResult)
        return false;

    // Get Xor Directory
    const bool kXorParseResult = parseXorDirectory(jsonData);
    if (!kXorParseResult)
        return false;

    return true;
}

//================================================================================
// Method: getBlockPath
// Description: Returns the configured block directory path.
//================================================================================
std::wstring Config::getBlockPath() const noexcept
{
    return m_blockDirectoryPath;
}

//================================================================================
// Method: getXorPath
// Description: Returns the configured XOR directory path.
//================================================================================
std::wstring Config::getXorPath() const noexcept
{
    return m_xorDirectoryPath;
}

//================================================================================
// Method: setBlockPath
// Description: Checks and sets the block directory path to the provided value.
//================================================================================
bool Config::setBlockPath(IN const std::wstring& newPath)
{
    if (!std::filesystem::exists(newPath))
    {
        std::cout << "Couldn't set Block path. Directory is not found.\n";
        return false;
    }

    m_blockDirectoryPath = newPath;
    return true;
}

//================================================================================
// Method: setXorPath
// Description: Checks and sets the XOR path to the provided value.
//================================================================================
bool Config::setXorPath(IN const std::wstring& newPath)
{
    if (!std::filesystem::exists(newPath))
    {
        std::cout << "Couldn't set Xor path. Directory is not found.\n";
        return false;
    }

    m_xorDirectoryPath = newPath;
    return true;
}

//================================================================================
// Method: handleCommandArguments
// Description: Validates command-line arguments. Ensures JSON file path is passed
//              and it exists on disk.
//================================================================================
bool Config::handleCommandArguments(IN int argc, IN wchar_t* argv[]) const
{
    if (argc != 2)
    {
        std::cout << "Incorrect command arguments. Usage: <json_path>\n";
        return false;
    }

    if (!std::filesystem::exists(argv[1]))
    {
        std::cout << "Json not found\n";
        return false;
    }

    return true;
}

//================================================================================
// Method: parseBlockDirectory
// Description: Reads the Block Directory key from the given JSON, validates that
//              a non-empty wide string was parsed, and applies it via setBlockPath.
//              Returns false if the key is missing/invalid or if applying the path
//              fails; otherwise returns true.
//================================================================================
bool Config::parseBlockDirectory(IN const nlohmann::json& json)
{
    constexpr const char* kBlockDirectoryJsonKey = "BlockDirectory";

    const auto kParsedBlockDirectory = parseWstringFromJson(json, kBlockDirectoryJsonKey);
    if (!kParsedBlockDirectory.has_value())
        return false;

    const bool kSetBlockResult = setBlockPath(kParsedBlockDirectory.value());
    if (!kSetBlockResult)
        return false;

    return true;
}

//================================================================================
// Method: parseXorDirectory
// Description: Reads the Xor Directory key from the given JSON, validates that
//              a non-empty wide string was parsed, and applies it via setXorPath.
//              Returns false if the key is missing/invalid or if applying the path
//              fails; otherwise returns true.
//================================================================================
bool Config::parseXorDirectory(IN const nlohmann::json& json)
{
    constexpr const char* kXorDirectoryJsonKey = "XorDirectory";

    const auto kParsedXorDirectory = parseWstringFromJson(json, kXorDirectoryJsonKey);
    if (!kParsedXorDirectory.has_value())
        return false;

    const bool kSetXorResult = setXorPath(kParsedXorDirectory.value());
    if (!kSetXorResult)
        return false;

    return true;
}

//================================================================================
// Method: parseWstringFromJson
// Description: Attempts to parse a wide string from a JSON object by key.
//              Returns nullopt if the key is missing.
//================================================================================
std::optional<std::wstring> Config::parseWstringFromJson(IN const nlohmann::json& json, IN const std::string& jsonKey) const
{
    const bool kIsKeyPresent = isJsonKeyPresent(json, jsonKey);
    if (!kIsKeyPresent)
        return std::nullopt;

    std::string kParsedString = json[jsonKey].get<std::string>();
    return stringUtf8ToWide(kParsedString);
}

//================================================================================
// Method: isJsonKeyPresent
// Description: Checks if the given key exists in the JSON object. Logs an error
//              message if not found.
//================================================================================
bool Config::isJsonKeyPresent(IN const nlohmann::json& json, IN const std::string& jsonKey) const
{
    if (!json.contains(jsonKey))
    {
        std::cout << "Couldn't find " << jsonKey << " key in JSON.\n";
        return false;
    }

    return true;
}
