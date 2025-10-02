#pragma once

// Standard Library headers
#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Project headers
#include "Common.h"
#include "DataTypes.h"


//================================================================================
// Function: LeUint8ToUint16
// Description: Converts uint8_t little-endian encoded array into uint16_t value
//================================================================================
[[nodiscard]] bool leUint8ToUint16(IN const std::vector<uint8_t>& in, OUT uint16_t& out);


//================================================================================
// Function: LeUint8ToUint32
// Description: Converts uint8_t little-endian encoded array into uint32_t value
//================================================================================
[[nodiscard]] bool leUint8ToUint32(IN const std::vector<uint8_t>& in, OUT uint32_t& out);


//================================================================================
// Function: LeUint8ToUint64
// Description: Converts uint8_t little-endian encoded array into uint64_t value
//================================================================================
[[nodiscard]] bool leUint8ToUint64(IN const std::vector<uint8_t>& in, OUT uint64_t& out);


//================================================================================
// Function: LeUint8ToInt32
// Description: Converts uint8_t little-endian encoded array into int32_t value
//================================================================================
[[nodiscard]] bool leUint8ToInt32(IN const std::vector<uint8_t>& in, OUT int32_t& out);


//================================================================================
// Function: getReversedDoubleSHA256
// Description: Computes the double SHA-256 hash of the given input buffer and
//              returns the result in reversed byte order.
//================================================================================
std::array<uint8_t, 32> getDoubleSHA256(IN const std::vector<uint8_t>& inputBuffer);


//================================================================================
// Function: HashToHexString
// Description: Converts a 32-byte hash stored in a std::array<uint8_t, 32> into
//              a formatted hexadecimal string. Each byte is printed as two
//              uppercase hex digits with leading zeros where necessary.
//================================================================================
std::string hashToHexString(IN const std::array<uint8_t, 32>& data);


//================================================================================
// Function: HashToHexString
// Description: Converts a hash stored in a std::vector<uint8_t> into a formatted 
//              hexadecimal string. Each byte is printed as two uppercase hex 
//              digits with leading zeros where necessary.
//================================================================================
std::string hashToHexString(IN const std::vector<uint8_t>& data);


//================================================================================
// Function: ToTime
// Description: Converts a 32-bit Unix timestamp into a human-readable UTC date 
//              and time string
//================================================================================
std::string toTime(IN const uint32_t time);


//================================================================================
// Function: stringToWideUtf8
// Description: Converts a UTF-8 encoded std::string into a UTF-16 std::wstring.
//================================================================================
std::wstring stringUtf8ToWide(IN const std::string& s);


//================================================================================
// Function: wideStringToUtf8
// Description: Converts a wide Unicode string to a UTF8 encoded std::string.
//================================================================================
std::string wideStringToUtf8(IN std::wstring_view wideString);


//================================================================================
// Function: getBlockFilesInDirectory
// Description: Scans the given directory and collects all files with the ".dat"
//              extension, returning them as a vector of wide strings. Skips
//              non-matching files. Returns an empty vector if directory is empty.
//================================================================================
std::vector<std::wstring> getBlockFilesInDirectory(IN const std::wstring& directory);
