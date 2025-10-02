#pragma once

// Standard Library headers
#include <expected>
#include <string>

// Project headers
#include "AppErrorCode.hpp"
#include "Common.h"


class Config
{
public:
    //================================================================================
    // Method: init
    // Description: Initializes the Config object. Handles command-line arguments,
    //              loads JSON configuration, and extracts data from it. Returns 
    //              AppErrorCode indicating success or the first error encountered.
    //================================================================================
    [[nodiscard]] AppErrorCode init(IN int argc, IN wchar_t* argv[]);


    //================================================================================
    // Method: getBlockPath
    // Description: Returns the configured block directory path as wide string.
    //================================================================================
    [[nodiscard]] std::wstring getBlockPath() const noexcept;


    //================================================================================
    // Method: getXorPath
    // Description: Returns the configured XOR directory path as wide string.
    //================================================================================
    [[nodiscard]] std::wstring getXorPath() const noexcept;


    //================================================================================
    // Method: getOutputPath
    // Description: Returns the configured output directory path as wide string.
    //================================================================================
    [[nodiscard]] std::wstring getOutputPath() const noexcept;
private:
    //================================================================================
    // Method: setBlockPath
    // Description: Sets the block directory path to the provided value.
    //================================================================================
    [[nodiscard]] AppErrorCode setBlockPath(IN const std::wstring& newPath);


    //================================================================================
    // Method: setXorPath
    // Description: Sets the XOR path to the provided value.
    //================================================================================
    [[nodiscard]] AppErrorCode setXorPath(IN const std::wstring& newPath);


    //================================================================================
    // Method: setOutputPath
    // Description: Sets the output path to the provided value.
    //================================================================================
    [[nodiscard]] AppErrorCode setOutputPath(IN const std::wstring& newPath);


    //================================================================================
    // Method: handleCommandArguments
    // Description: Validates command-line arguments. Ensures JSON file path is passed
    //              and it exists on disk. Returns Success on valid input or an error 
    //              code otherwise.
    //================================================================================
    [[nodiscard]] AppErrorCode handleCommandArguments(IN int argc, IN wchar_t* argv[]) const;


    //================================================================================
    // Method: parseBlockDirectory
    // Description: Reads the Block Directory key from the given JSON, validates that
    //              a non-empty wide string was parsed, and applies it via setBlockPath.
    //              Returns Success if applied or the specific error from parsing or
    //              validation if unsuccessful.
    //================================================================================
    [[nodiscard]] AppErrorCode parseBlockDirectory(IN const nlohmann::json& json);


    //================================================================================
    // Method: parseXorDirectory
    // Description: Reads the Xor Directory key from the given JSON, validates that
    //              a non-empty wide string was parsed, and applies it via setXorPath.
    //              Returns false if the key is missing/invalid or if applying the path
    //              fails; otherwise returns true.
    //================================================================================
    [[nodiscard]] AppErrorCode parseXorDirectory(IN const nlohmann::json& json);


    //================================================================================
    // Method: parseOutputDirectory
    // Description: Reads the Output Directory key from the given JSON, validates that
    //              a non-empty wide string was parsed, and applies it via setOutputPath.
    //              Returns false if the key is missing/invalid or if applying the path
    //              fails; otherwise returns true.
    //================================================================================
    [[nodiscard]] AppErrorCode parseOutputDirectory(IN const nlohmann::json& json);


    //================================================================================
    // Method: parseWstringFromJson
    // Description: Attempts to parse a wide string from a JSON object by key.
    //              Returns the converted value on success or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<std::wstring, AppErrorCode> parseWstringFromJson(
        IN const nlohmann::json& json, 
        IN const std::string& jsonKey
    ) const;


    //================================================================================
    // Method: isJsonKeyPresent
    // Description: Checks if the given key exists in the JSON object. Returns true 
    //              if present, false otherwise.
    //================================================================================
    [[nodiscard]] bool isJsonKeyPresent(
        IN const nlohmann::json& json, 
        IN const std::string& jsonKey
    ) const;


private:
    std::wstring m_blockDirectoryPath;
    std::wstring m_xorDirectoryPath;
    std::wstring m_outputDirectoryPath;
};
