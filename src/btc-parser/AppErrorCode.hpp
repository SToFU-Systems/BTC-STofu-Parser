#pragma once

// Standard Library headers
#include <string>
#include <system_error>


//================================================================================
// Enum: AppErrorCode
// Description: Represents all possible program exit statuses for error handling
//================================================================================
enum class AppErrorCode
{
    Success = 0,                    // Operation completed successfully

    CommandArgumentsError = 1,      // Invalid or missing command-line parameters
    JsonKeyNotFound = 2,            // Missing or invalid key in JSON configuration

    FileError = 3,                  // General file-related error (open, seek, etc.)
    FileReadingFailure = 4,         // Failed while reading file contents
    FileNotFound = 5,               // File could not be found
    DirectoryNotFound = 6,          // Directory could not be found

    BlockParsingFailure = 7,        // Unable to parse Bitcoin block or related data
    BlockWrongMagic = 8,            // Unrecognized block magic value
    EmptyBlockSize = 9,             // Parsed block size is zero
    TooLargeBlockSize = 10,         // Parsed block size exceeds allowed limit

    IntegerConversionFailure = 11,  // Error converting between integer types
    XorObfuscationFailure = 12,     // Failure during XOR de-obfuscation

    RuntimeException = 13,          // Standard runtime exception was thrown
    UnknownException = 14,          // Non-standard or unknown exception occurred

    LoggerInitFailure = 15,         // Logger failed to initialize
};


//================================================================================
// Integrate with std::error_code
//================================================================================
namespace std
{
    template <>
    struct is_error_code_enum<AppErrorCode> : true_type {};
}

class AppErrorCategory : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "Error";
    }


    std::string message(int value) const override
    {
        switch (static_cast<AppErrorCode>(value))
        {
        case AppErrorCode::Success:
            return "Operation completed successfully";

        case AppErrorCode::CommandArgumentsError:
            return "Invalid or missing command-line parameters";
        case AppErrorCode::JsonKeyNotFound:
            return "Missing or invalid key in JSON configuration";

        case AppErrorCode::FileError:
            return "General file-related error";
        case AppErrorCode::FileReadingFailure:
            return "Failed while reading file contents";
        case AppErrorCode::FileNotFound:
            return "File not found";
        case AppErrorCode::DirectoryNotFound:
            return "Directory not found";

        case AppErrorCode::BlockParsingFailure:
            return "Failed to parse Bitcoin block data";
        case AppErrorCode::BlockWrongMagic:
            return "Unrecognized block magic number";
        case AppErrorCode::EmptyBlockSize:
            return "Block size is zero";
        case AppErrorCode::TooLargeBlockSize:
            return "Block size exceeds maximum allowed limit";

        case AppErrorCode::IntegerConversionFailure:
            return "Integer conversion failed";
        case AppErrorCode::XorObfuscationFailure:
            return "XOR de-obfuscation failed";

        case AppErrorCode::RuntimeException:
            return "A std::runtime_error was thrown";
        case AppErrorCode::UnknownException:
            return "An unknown exception was thrown";

        case AppErrorCode::LoggerInitFailure:
            return "Logger failed to initialize";

        default:
            return "Unrecognized error code";
        }
    }
};


//================================================================================
// Function: appErrorCategory
// Description: Returns a singleton instance of AppErrorCategory
//================================================================================
inline const AppErrorCategory& errorCategory()
{
    static AppErrorCategory instance;
    return instance;
}


//================================================================================
// Function: make_error_code
// Description: Factory for std::error_code from AppErrorCode
//================================================================================
inline std::error_code make_error_code(AppErrorCode e) noexcept
{
    return { static_cast<int>(e), errorCategory() };
}
