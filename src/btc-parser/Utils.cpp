// Precompiled headers
#include "pch.h"

// Project headers
#include "Utils.h"


namespace
{
    //================================================================================
    // Function: HandleConvertionInput
    // Description: Validates the provided input data buffer before processing.
    //================================================================================
    [[nodiscard]] bool handleConvertionInput(IN const std::vector<uint8_t>& inputData, IN const size_t inputSize)
    {
        if (inputData.size() != inputSize)
            return false;

        return true;
    }
}

//================================================================================
// Function: LeUint8ToUint16
// Description: Converts uint8_t little-endian encoded array into uint16_t value
//================================================================================
bool leUint8ToUint16(IN const std::vector<uint8_t>& in, OUT uint16_t& out)
{
    constexpr size_t kOutBytes = sizeof(uint16_t);
    const bool kInputHandler = handleConvertionInput(in, kOutBytes);
    if (!kInputHandler)
        return false;

    out = 0;
    for (size_t i = 0; i < kOutBytes; ++i)
    {
        const size_t kShift = i * 8;
        const uint16_t kNewByte = static_cast<uint16_t>(in[i]) << kShift;

        out |= kNewByte;
    }

    return true;
}

//================================================================================
// Function: LeUint8ToUint32
// Description: Converts uint8_t little-endian encoded array into uint32_t value
//================================================================================
bool leUint8ToUint32(IN const std::vector<uint8_t>& in, OUT uint32_t& out)
{
    constexpr size_t kOutBytes = sizeof(uint32_t);
    const bool kInputHandler = handleConvertionInput(in, kOutBytes);
    if (!kInputHandler)
        return false;

    out = 0;
    for (size_t i = 0; i < kOutBytes; ++i)
    {
        const size_t kShift = i * 8;
        const uint32_t kNewByte = static_cast<uint32_t>(in[i]) << kShift;

        out |= kNewByte;
    }

    return true;
}

//================================================================================
// Function: LeUint8ToUint64
// Description: Converts uint8_t little-endian encoded array into uint64_t value
//================================================================================
bool leUint8ToUint64(IN const std::vector<uint8_t>& in, OUT uint64_t& out)
{
    constexpr size_t kOutBytes = sizeof(uint64_t);
    const bool kInputHandler = handleConvertionInput(in, kOutBytes);
    if (!kInputHandler)
        return false;

    out = 0;
    for (size_t i = 0; i < kOutBytes; ++i)
    {
        const size_t kShift = i * 8;
        const uint64_t kNewByte = static_cast<uint64_t>(in[i]) << kShift;

        out |= kNewByte;
    }

    return true;
}

//================================================================================
// Function: LeUint8ToInt32
// Description: Converts uint8_t little-endian encoded array into int32_t value
//================================================================================
bool leUint8ToInt32(IN const std::vector<uint8_t>& in, OUT int32_t& out)
{
    uint32_t tmp = {};
    const bool kUintConversion = leUint8ToUint32(in, tmp);
    if (!kUintConversion)
        return false;

    out = static_cast<int32_t>(tmp);
    return true;
}

//================================================================================
// Function: getReversedDoubleSHA256
// Description: Computes the double SHA-256 hash of the given input buffer and
//              returns the result in reversed byte order.
//================================================================================
std::array<uint8_t, 32> getReversedDoubleSHA256(IN const std::vector<uint8_t>& inputBuffer)
{
    const size_t kBufferSize = inputBuffer.size();

    std::array<uint8_t, 32> firstHash;
    SHA256(inputBuffer.data(), kBufferSize, firstHash.data());

    std::array<uint8_t, 32> finalHash;
    SHA256(firstHash.data(), sizeof(firstHash), finalHash.data());

    std::reverse(finalHash.begin(), finalHash.end());
    return finalHash;
}

//================================================================================
// Function: HashToHexString
// Description: Converts a 32-byte hash stored in a std::array<uint8_t, 32> into
//              a formatted hexadecimal string. Each byte is printed as two
//              uppercase hex digits with leading zeros where necessary.
//================================================================================
std::string hashToHexString(IN const std::array<uint8_t, 32>& data)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (const auto& byte : data)
        oss << std::setw(2) << static_cast<unsigned int>(byte);

    return oss.str();
}

//================================================================================
// Function: HashToHexString
// Description: Converts a hash stored in a std::vector<uint8_t> into a formatted 
//              hexadecimal string. Each byte is printed as two uppercase hex 
//              digits with leading zeros where necessary.
//================================================================================
std::string hashToHexString(IN const std::vector<uint8_t>& data)
{
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (const auto& byte : data)
        oss << std::setw(2) << static_cast<unsigned int>(byte);

    return oss.str();
}

//================================================================================
// Function: ToTime
// Description: Converts a 32-bit Unix timestamp into a human-readable UTC date 
//              and time string.
//================================================================================
std::string toTime(IN const uint32_t time)
{
    std::time_t convertedTime = static_cast<std::time_t>(time);
    std::tm tm = {};
    if (gmtime_s(&tm, &convertedTime))
        return "";

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC");
    return oss.str();
}

//================================================================================
// Function: stringToWideUtf8
// Description: Converts a UTF-8 encoded std::string into a UTF-16 std::wstring.
//================================================================================
std::wstring stringUtf8ToWide(IN const std::string& s)
{
    if (s.empty())
        return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    if (len == 0)
        return std::wstring();

    std::wstring result(len, L'\0');
    len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), result.data(), len);
    if (len == 0)
        return std::wstring();

    return result;
}

//================================================================================
// Function: getBlockFilesInDirectory
// Description: Scans the given directory and collects all files with the ".dat"
//              extension, returning them as a vector of wide strings. Skips
//              non-matching files. Returns an empty vector if directory is empty.
//================================================================================
std::vector<std::wstring> getBlockFilesInDirectory(IN const std::wstring& directory)
{
    constexpr const wchar_t* kBlockExtension = L".dat";

    std::vector<std::wstring> blockFiles;
    for (const auto& file : std::filesystem::directory_iterator(directory))
    {
        const std::filesystem::path kFilePath = file.path();
        if (kFilePath.extension() != kBlockExtension)
            continue;

        blockFiles.emplace_back(kFilePath.wstring());
    }

    return blockFiles;
}
