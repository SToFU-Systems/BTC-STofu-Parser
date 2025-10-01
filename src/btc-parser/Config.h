#pragma once

// Standard Library headers
#include <string>

// Project headers
#include "Common.h"


class Config
{
public:
    //================================================================================
    // Method: init
    // Description: Initializes the Config object. Handles command-line arguments,
    //              loads JSON configuration, and extracts data from it.
    //================================================================================
    bool init(IN int argc, IN wchar_t* argv[]);


    //================================================================================
    // Method: getBlockPath
    // Description: Returns the configured block directory path.
    //================================================================================
    std::wstring getBlockPath() const noexcept;


    //================================================================================
    // Method: getXorPath
    // Description: Returns the configured XOR directory path.
    //================================================================================
    std::wstring getXorPath() const noexcept;


private:
    //================================================================================
    // Method: setBlockPath
    // Description: Sets the block directory path to the provided value.
    //================================================================================
    [[nodiscard]] bool setBlockPath(IN const std::wstring& newPath);


    //================================================================================
    // Method: setXorPath
    // Description: Sets the XOR path to the provided value.
    //================================================================================
    [[nodiscard]] bool setXorPath(IN const std::wstring& newPath);


    //================================================================================
    // Method: handleCommandArguments
    // Description: Validates command-line arguments. Ensures JSON file path is passed
    //              and it exists on disk.
    //================================================================================
    [[nodiscard]] bool handleCommandArguments(IN int argc, IN wchar_t* argv[]) const;


    //================================================================================
    // Method: parseBlockDirectory
    // Description: Reads the Block Directory key from the given JSON, validates that
    //              a non-empty wide string was parsed, and applies it via setBlockPath.
    //              Returns false if the key is missing/invalid or if applying the path
    //              fails; otherwise returns true.
    //================================================================================
    [[nodiscard]] bool parseBlockDirectory(IN const nlohmann::json& json);


    //================================================================================
    // Method: parseXorDirectory
    // Description: Reads the Xor Directory key from the given JSON, validates that
    //              a non-empty wide string was parsed, and applies it via setXorPath.
    //              Returns false if the key is missing/invalid or if applying the path
    //              fails; otherwise returns true.
    //================================================================================
    [[nodiscard]] bool parseXorDirectory(IN const nlohmann::json& json);


    //================================================================================
    // Method: parseWstringFromJson
    // Description: Attempts to parse a wide string from a JSON object by key.
    //              Returns nullopt if the key is missing.
    //================================================================================
    std::optional<std::wstring> parseWstringFromJson(IN const nlohmann::json& json, IN const std::string& jsonKey) const;


    //================================================================================
    // Method: isJsonKeyPresent
    // Description: Checks if the given key exists in the JSON object. Logs an error
    //              message if not found.
    //================================================================================
    bool isJsonKeyPresent(IN const nlohmann::json& json, IN const std::string& jsonKey) const;


private:
    std::wstring m_blockDirectoryPath;
    std::wstring m_xorDirectoryPath;
};
