// Precompiled headers
#include "pch.h"

// Project headers
#include "BitcoinReader.h"
#include "Logger.hpp"
#include "Utils.h"


//================================================================================
// Method: BitcoinReader
// Description: Constructor. Opens the target block file through FileReader,
//              initializes the logical read position to zero, and leaves XOR
//              de-obfuscation disabled.
//================================================================================
BitcoinReader::BitcoinReader(IN const std::wstring& blockPath)
    : 
    m_fileReader(blockPath),
    m_readPosition(0),
    m_doXorObfuscation(false)
{
    LOG_VERBOSE("Initialized BitcoinReader for file: {}", blockPath);
}

//================================================================================
// Method: BitcoinReader
// Description: Constructor. Initializes the block file reader and then loads the
//              XOR key from the provided path. Enables XOR de-obfuscation when
//              a valid key of expected size is read.
//================================================================================
BitcoinReader::BitcoinReader(IN const std::wstring& blockPath, IN const std::wstring& xorPath)
    : BitcoinReader(blockPath)
{
    constexpr size_t kXorKeyBytes = 8;

    if (xorPath.empty())
        return;

    FileReader xorFile(xorPath);
    const bool kXorReadResult = xorFile.readBytes(kXorKeyBytes, xorKey);
    if (!kXorReadResult)
        throw std::runtime_error("Couldn't read the xor.dat");

    if (xorKey.size() != kXorKeyBytes)
        throw std::runtime_error("Incorrect size of xor key");

    m_doXorObfuscation = true;
    LOG_VERBOSE("Enabled XOR obfuscation for data reading.");
}

//================================================================================
// Method: xorObfuscation
// Description: Applies XOR-based obfuscation to the provided data buffer using 
//              the stored XOR key. Each byte of the data is XOR'ed with a 
//              corresponding key byte, chosen by using the absolute read position.
//              Returns true on successful obfuscation, false otherwise.
//================================================================================
bool BitcoinReader::xorObfuscation(IN const size_t readPosition, INOUT std::vector<uint8_t>& data)
{
    if (data.empty())
    {
        LOG_ERROR("Input buffer is empty.")
        return false;
    }

    if (xorKey.empty())
    {
        LOG_ERROR("XOR key is empty.")
        return false;
    }

    constexpr size_t kXorKeyBytes = 8;
    bool isCorrectSize = xorKey.size() == kXorKeyBytes;
    if (!isCorrectSize)
    {
        LOG_ERROR("Incorrect XOR key length. Expected: {}, instead received: {}.", kXorKeyBytes, xorKey.size());
        return false;
    }

    size_t keySize = xorKey.size();
    for (size_t i = 0; i < data.size(); ++i)
    {
        const size_t offset = i + readPosition;
        const size_t index = offset % keySize;
        data[i] ^= xorKey[index];
    }

    return true;
}

//================================================================================
// Method: readData
// Description: Reads the requested number of bytes into a vector and advances the
//              internal read position. If XOR de-obfuscation is enabled, applies
//              it using the pre-read file position for correct key alignment.
//              Returns the buffer on success or AppErrorCode on failure.
//================================================================================
std::expected<std::vector<uint8_t>, AppErrorCode> BitcoinReader::readData(IN const size_t bytes)
{
    std::vector<uint8_t> out;

    if (bytes == 0)
        return out;

    // needed for xor obfuscation
    const size_t kPreReadPosition = m_fileReader.getReadPosition();

    const bool kReadResult = m_fileReader.readBytes(bytes, out);
    if (!kReadResult)
    {
        LOG_ERROR("Failed to read {} bytes from file.", bytes);
        return std::unexpected(AppErrorCode::FileReadingFailure);
    }

    m_readPosition = m_fileReader.getReadPosition();

    if (!m_doXorObfuscation)
        return out;

    const bool kObfuscationResult = xorObfuscation(kPreReadPosition, out);
    if (!kObfuscationResult)
    {
        LOG_ERROR("Failed to XOR-obfuscate the parsed data.");
        return std::unexpected(AppErrorCode::XorObfuscationFailure);
    }

    return out;
}

//================================================================================
// Method: readDataUint16
// Description: Reads sizeof(uint16_t) bytes from the block file and converts them 
//              from little-endian encoding into uint16_t value. Uses LeUint8ToUint16 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
std::expected<uint16_t, AppErrorCode> BitcoinReader::readDataUint16()
{
    constexpr size_t kOutSize = sizeof(uint16_t);

    auto kReadResult = readData(kOutSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> buffer = std::move(kReadResult).value();

    uint16_t out;
    const bool kConvertResult = leUint8ToUint16(buffer, out);
    if (!kConvertResult)
    {
        LOG_ERROR("Failed to convert uint8_t to uint16_t.");
        return std::unexpected(AppErrorCode::IntegerConversionFailure);
    }

    return out;
}

//================================================================================
// Method: readDataUint32
// Description: Reads sizeof(uint32_t) bytes from the block file and converts them 
//              from little-endian encoding into uint32_t value. Uses LeUint8ToUint32 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
std::expected<uint32_t, AppErrorCode> BitcoinReader::readDataUint32()
{
    constexpr size_t kOutSize = sizeof(uint32_t);

    auto kReadResult = readData(kOutSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> buffer = std::move(kReadResult).value();

    uint32_t out;
    const bool kConvertResult = leUint8ToUint32(buffer, out);
    if (!kConvertResult)
    {
        LOG_ERROR("Failed to convert uint8_t to uint32_t.");
        return std::unexpected(AppErrorCode::IntegerConversionFailure);
    }

    return out;
}

//================================================================================
// Method: readDataUint64
// Description: Reads sizeof(uint64_t) bytes from the block file and converts them 
//              from little-endian encoding into uint64_t value. Uses LeUint8ToUint64 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
std::expected<uint64_t, AppErrorCode> BitcoinReader::readDataUint64()
{
    constexpr size_t kOutSize = sizeof(uint64_t);

    auto kReadResult = readData(kOutSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> buffer = std::move(kReadResult).value();

    uint64_t out;
    const bool kConvertResult = leUint8ToUint64(buffer, out);
    if (!kConvertResult)
    {
        LOG_ERROR("Failed to convert uint8_t to uint64_t.");
        return std::unexpected(AppErrorCode::IntegerConversionFailure);
    }

    return out;
}

//================================================================================
// Method: readDataInt32
// Description: Reads sizeof(int32_t) bytes from the block file and converts them 
//              from little-endian encoding into int32_t value. Uses LeUint8ToInt32 
//              for the conversion. Returns true on successful read and conversion, 
//              false otherwise.
//================================================================================
std::expected<int32_t, AppErrorCode> BitcoinReader::readDataInt32()
{
    constexpr size_t kOutSize = sizeof(int32_t);

    auto kReadResult = readData(kOutSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> buffer = std::move(kReadResult).value();

    int32_t out;
    const bool kConvertResult = leUint8ToInt32(buffer, out);
    if (!kConvertResult)
    {
        LOG_ERROR("Failed to convert uint8_t to int32_t.");
        return std::unexpected(AppErrorCode::IntegerConversionFailure);
    }

    return out;
}

//================================================================================
// Method: readDataArray32
// Description: Reads 32 bytes from the block file and stores them in
//              std::array<uint8_t, 32>. Performs reverse_copy to account 
//              little-endian encoding. Returns true if 32 bytes are successfully 
//              read and copied, false if insufficient data is available.
//================================================================================
std::expected<std::array<uint8_t, 32>, AppErrorCode> BitcoinReader::readDataArray32()
{
    constexpr size_t kOutSize = 32;

    auto kReadResult = readData(kOutSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> buffer = std::move(kReadResult).value();

    if (buffer.size() < kOutSize)
    {
        LOG_ERROR("Incorrect read array size. Expected: {}, instead received: {}.", kOutSize, buffer.size());
        return std::unexpected(AppErrorCode::FileReadingFailure);
    }

    std::array<uint8_t, kOutSize> out;
    std::reverse_copy(buffer.begin(), buffer.begin() + kOutSize, out.begin());
    return out;
}

//================================================================================
// Method: readDataCompactSize
// Description: Decodes a CompactSize-encoded integer from the given first byte
//              and reads the remaining bytes if needed. 
//              CompactSize is the Bitcoin variable-length integer encoding:
//              - If firstByte < 0xFD: value is the byte itself
//              - If firstByte == 0xFD: next 2 bytes (uint16_t) are read
//              - If firstByte == 0xFE: next 4 bytes (uint32_t) are read
//              - If firstByte == 0xFF: next 8 bytes (uint64_t) are read
//              Returns true on successful decoding, false if reading or 
//              validation fails.
//================================================================================
std::expected<uint64_t, AppErrorCode> BitcoinReader::readDataCompactSize(IN const uint8_t firstByte)
{
    if (firstByte < 0xFD)
        return firstByte;

    uint64_t out;
    switch (firstByte)
    {
    case 0xFD:
    {
        auto uint16Result = readDataUint16();
        RETURN_IF_UNEXPECTED(uint16Result);

        out = static_cast<uint64_t>(uint16Result.value());
        break;
    }

    case 0xFE:
    {
        auto uint32Result = readDataUint32();
        RETURN_IF_UNEXPECTED(uint32Result);

        out = static_cast<uint64_t>(uint32Result.value());
        break;
    }

    case 0xFF:
    {
        auto uint64Result = readDataUint64();
        RETURN_IF_UNEXPECTED(uint64Result);

        out = uint64Result.value();
        break;
    }

    default:
        LOG_ERROR("Unexpected value from reading CompactSize.");
        return std::unexpected(AppErrorCode::BlockParsingFailure);
    }

    return out;
}

//================================================================================
// Method: readDataCompactSize
// Description: Reads the first byte from the file and call overloaded 
//              ReadDataCompactSize to decode the Bitcoin variable-length integer.
//              Returns true on successful decoding, false if reading or 
//              validation fails.
//================================================================================
std::expected<uint64_t, AppErrorCode> BitcoinReader::readDataCompactSize()
{
    auto kReadBufferResult = readData(1);
    RETURN_IF_UNEXPECTED(kReadBufferResult);

    std::vector<uint8_t> buffer = std::move(kReadBufferResult).value();

    const uint8_t kFirstByte = buffer[0];
    auto kReadCompactResult = readDataCompactSize(kFirstByte);
    RETURN_IF_UNEXPECTED(kReadCompactResult);

    return kReadCompactResult.value();
}

//================================================================================
// Method: getReadPosition
// Description: Returns the current file read position (in bytes).
//================================================================================
size_t BitcoinReader::getReadPosition() const
{
    return m_readPosition;
}

//================================================================================
// Method: setReadPosition
// Description: Updates file read position to the input variable. 
//              Returns true on success, false otherwise.
//================================================================================
bool BitcoinReader::setReadPosition(IN const size_t& newPos)
{
    const bool kSetPosResult = m_fileReader.setReadPosition(newPos);
    if (!kSetPosResult)
    {
        LOG_ERROR("Failed to set position in file.");
        return false;
    }

    m_readPosition = newPos;
    LOG_VERBOSE("Set new file reading position at {}.", newPos);
    return true;
}

//================================================================================
// Method: isEof
// Description: Wrapper of FileReader isEof() method. Returns true if file reader
//              has reached the end of file, false otherwise.
//================================================================================
bool BitcoinReader::isEof()
{
    return m_fileReader.isEof();
}

//================================================================================
// Method: getFileSize
// Description: Wrapper of FileReader getFileSize() method. Returns size (in bytes)
//              of currently opened file.
//================================================================================
size_t BitcoinReader::getFileSize() const
{
    return m_fileReader.getFileSize();
}
