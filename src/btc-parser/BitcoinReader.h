#pragma once

// Standard Library headers
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <vector>

// Project headers
#include "AppErrorCode.hpp"
#include "Common.h"
#include "FileReader.h"


class BitcoinReader
{
public:
    //================================================================================
    // Method: BitcoinReader
    // Description: Constructor. Opens the target block file through FileReader,
    //              initializes the logical read position to zero, and leaves XOR
    //              de-obfuscation disabled.
    //================================================================================
    explicit BitcoinReader(IN const std::wstring& blockPath);


    //================================================================================
    // Method: BitcoinReader
    // Description: Constructor. Initializes the block file reader and then loads the
    //              XOR key from the provided path. Enables XOR de-obfuscation when
    //              a valid key of expected size is read.
    //================================================================================
    explicit BitcoinReader(IN const std::wstring& blockPath, IN const std::wstring& xorPath);


    //================================================================================
    // Method: xorObfuscation
    // Description: Applies XOR-based obfuscation to the provided data buffer using 
    //              the stored XOR key. Each byte of the data is XOR'ed with a 
    //              corresponding key byte, chosen by using the absolute read position.
    //              Returns true on successful obfuscation, false otherwise.
    //================================================================================
    [[nodiscard]] bool xorObfuscation(IN const size_t readPosition, INOUT std::vector<uint8_t>& data);


    //================================================================================
    // Method: readData
    // Description: Reads the requested number of bytes into a vector and advances the
    //              internal read position. If XOR de-obfuscation is enabled, applies
    //              it using the pre-read file position for correct key alignment.
    //              Returns the buffer on success or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<uint8_t>, AppErrorCode> readData(IN const size_t bytes);


    //================================================================================
    // Method: readDataUint16
    // Description: Reads sizeof(uint16_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint16_t value. Uses LeUint8ToUint16 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] std::expected<uint16_t, AppErrorCode> readDataUint16();


    //================================================================================
    // Method: readDataUint32
    // Description: Reads sizeof(uint32_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint32_t value. Uses LeUint8ToUint32 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] std::expected<uint32_t, AppErrorCode> readDataUint32();


    //================================================================================
    // Method: readDataUint64
    // Description: Reads sizeof(uint64_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint64_t value. Uses LeUint8ToUint64 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] std::expected<uint64_t, AppErrorCode> readDataUint64();


    //================================================================================
    // Method: readDataInt32
    // Description: Reads sizeof(int32_t) bytes from the block file and converts them 
    //              from little-endian encoding into int32_t value. Uses LeUint8ToInt32 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] std::expected<int32_t, AppErrorCode> readDataInt32();


    //================================================================================
    // Method: readDataArray32
    // Description: Reads 32 bytes from the block file and stores them in
    //              std::array<uint8_t, 32>. Performs reverse_copy to account 
    //              little-endian encoding. Returns true if 32 bytes are successfully 
    //              read and copied, false if insufficient data is available.
    //================================================================================
    [[nodiscard]] std::expected<std::array<uint8_t, 32>, AppErrorCode> readDataArray32();


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
    [[nodiscard]] std::expected<uint64_t, AppErrorCode> readDataCompactSize(IN const uint8_t firstByte);


    //================================================================================
    // Method: readDataCompactSize
    // Description: Reads the first byte from the file and call overloaded 
    //              ReadDataCompactSize to decode the Bitcoin variable-length integer.
    //              Returns true on successful decoding, false if reading or 
    //              validation fails.
    //================================================================================
    [[nodiscard]] std::expected<uint64_t, AppErrorCode> readDataCompactSize();


    //================================================================================
    // Method: getReadPosition
    // Description: Returns the current file read position (in bytes).
    //================================================================================
    [[nodiscard]] size_t getReadPosition() const;


    //================================================================================
    // Method: setReadPosition
    // Description: Updates file read position to the input variable. 
    //              Returns true on success, false otherwise.
    //================================================================================
    [[nodiscard]] bool setReadPosition(IN const size_t& newPos);


    //================================================================================
    // Method: isEof
    // Description: Wrapper of FileReader isEof() method. Returns true if file reader
    //              has reached the end of file, false otherwise.
    //================================================================================
    [[nodiscard]] bool isEof();


    //================================================================================
    // Method: getFileSize
    // Description: Wrapper of FileReader getFileSize() method. Returns size (in bytes)
    //              of currently opened file.
    //================================================================================
    [[nodiscard]] size_t getFileSize() const;


private:
    FileReader m_fileReader;
    size_t m_readPosition;

    bool m_doXorObfuscation;
    std::vector<uint8_t> xorKey;
};
