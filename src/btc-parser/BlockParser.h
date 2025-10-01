#pragma once

// Standard Library headers
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>

// Project headers
#include "Common.h"
#include "DataTypes.h"
#include "FileReader.h"


//================================================================================
// Class: BlockParser
// Description: Responsible for reading and parsing raw Bitcoin block data from
//              *.dat files. Provides methods to read data from file and handles 
//              optional XOR obfuscation for the newest Bitcoin Core versions. 
//              Ensures correctness by validating magic numbers, block sizes, 
//              and transaction boundaries. Supports both parsing of a single 
//              block and sequential parsing of all blocks in a file.
//================================================================================
class BlockParser
{
public:
    //================================================================================
    // Method: BlockParser
    // Description: Constructor that initializes a BlockParser instance using the
    //              given block file path. Creates a FileReader to handle block file
    //              input, sets the initial read position and block offset position
    //              to zero, and disables XOR obfuscation by default. Prepares the
    //              parser for reading block data sequentially.
    //================================================================================
    explicit BlockParser(IN const std::wstring& blockPath);


    //================================================================================
    // Method: BlockParser
    // Description: Constructor that initializes a BlockParser instance with both a
    //              block file path and XOR key file path. Calls the primary 
    //              constructor to set up the block reader, then loads the XOR key. 
    //              If loaded successfully, enables XOR obfuscation for subsequent 
    //              block data parsing.
    //================================================================================
    explicit BlockParser(
        IN const std::wstring& blockPath, 
        IN const std::wstring& xorPath
    );


    /* Deleted */
    BlockParser(const BlockParser&) = delete;
    BlockParser(BlockParser&&) noexcept = delete;
    BlockParser& operator=(const BlockParser&) = delete;
    BlockParser& operator=(BlockParser&&) noexcept = delete;


    //================================================================================
    // Method: ~BlockParser
    // Description: Defaulted destructor.
    //================================================================================
    ~BlockParser() = default;


    //================================================================================
    // Method: ParseBlock
    // Description: Parses a single Bitcoin block from the file. Steps:
    //              1. Reads and validates the block magic number.
    //              2. Reads the block size.
    //              3. Parses the block header into the output Block.
    //              4. Reads the transaction counter.
    //              5. Iterates and parses each transaction in the block.
    //              6. Validates that the actual bytes read match the block size.
    //              Updates the internal block offset position after successful
    //              parsing. Returns true if the block is parsed correctly, false if
    //              any validation or parsing step fails.
    //================================================================================
    [[nodiscard]] bool parseBlock(OUT Block& out);


    //================================================================================
    // Method: ParseAllBlocks
    // Description: Parses all blocks sequentially from the opened block file until
    //              EOF. For each block, calls ParseBlock and stores the result in 
    //              the output vector. Returns true if all blocks are parsed correctly,
    //              false if any failed.
    //================================================================================
    [[nodiscard]] bool parseAllBlocks(OUT std::vector<Block>& out);


private:
    //================================================================================
    // Method: ParseMagicNumber
    // Description: Reads and validates the block magic number by comparing it to the 
    //              expected mainnet magic number constant. Returns true if the read 
    //              succeeds and the number matches, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseMagicNumber(OUT uint32_t& out);


    //================================================================================
    // Method: ParseBlockSize
    // Description: Reads the block size from file. Ensures that the
    //              size is greater than zero. Returns true on success, false if
    //              reading fails or the size is invalid.
    //================================================================================
    [[nodiscard]] bool parseBlockSize(OUT uint32_t& out);


    //================================================================================
    // Method: ParseBlockHeader
    // Description: Parses the block header data into the provided BlockHeader
    //              structure. Steps:
    //              - Reads the version.
    //              - Reads the previous block hash.
    //              - Reads the Merkle root hash.
    //              - Reads the timestamp.
    //              - Reads the difficulty bits.
    //              - Reads the nonce.
    //              Returns true if all fields are parsed correctly, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseBlockHeader(OUT BlockHeader& header);


    //================================================================================
    // Method: getCurrentBlockHash
    // Description: Reads the current block header from the file, computes its hash 
    //              (double SHA-256, reversed). Returns the computed hash on success,
    //              otherwise std::nullopt.
    //================================================================================
    [[nodiscard]] std::optional<std::array<uint8_t, 32>> getCurrentBlockHash();

    //================================================================================
    // Method: ParseTransactionCounter
    // Description: Reads a CompactSize-encoded integer representing the number of
    //              transactions in the current block. Returns true on success, false
    //              otherwise.
    //================================================================================
    [[nodiscard]] bool parseTransactionCounter(OUT uint64_t& out);


    //================================================================================
    // Method: ParseTransaction
    // Description: Parses a full Bitcoin transaction into the provided Transaction
    //              structure. Steps:
    //              - Reads the version.
    //              - Reads the optional segwit marker and flag.
    //              - Reads the input counter and parses all transaction inputs.
    //              - Reads the output counter and parses all transaction outputs.
    //              - If segwit is present, parses witness data for each input.
    //              - Reads the transaction lock time.  
    //              Returns true if all fields are parsed correctly, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseTransaction(OUT Transaction& transaction);


    //================================================================================
    // Method: ParseTransactionFlag
    // Description: Parses the optional transaction segwit flag (00 01). If the flag
    //              is present, sets output to true. If not, rewinds the read
    //              position and sets output to false. Returns true if parsing or 
    //              fallback succeeds, false otherwise.
    //================================================================================
    [[nodiscard]] std::optional<bool> parseTransactionFlag();


    //================================================================================
    // Method: ParseTxIn
    // Description: Parses a single transaction input (TxIn). Steps:
    //              - Reads the previous transaction hash.
    //              - Reads the output index being referenced.
    //              - Reads the script length.
    //              - Reads the scriptSig data.
    //              - Reads the sequence number.  
    //              Returns true if all fields are parsed correctly, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseTxIn(OUT TxIn& out);


    //================================================================================
    // Method: ParseTxOut
    // Description: Parses a single transaction output (TxOut). Steps:
    //              - Reads the output value in satoshis (uint64).
    //              - Reads the script length (CompactSize).
    //              - Reads the scriptPubKey data.  
    //              Returns true if all fields are parsed correctly, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseTxOut(OUT TxOut& out);


    //================================================================================
    // Method: ParseWitness
    // Description: Parses witness data associated with a segwit transaction. Reads
    //              a CompactSize counter for the number of witness elements, then
    //              for each element reads its length (CompactSize) and data bytes.
    //              Appends all witness data into the output buffer. Returns true
    //              if all witness elements are parsed successfully, false otherwise.
    //================================================================================
    [[nodiscard]] bool parseWitness(OUT std::vector<uint8_t>& out);


    //================================================================================
    // Method: getCurrentTransactionHash
    // Description: Computes the transaction hash (double SHA-256, reversed) for the
    //              transaction defined by start and finish positions. Returns the
    //              computed hash on success, otherwise std::nullopt.
    //================================================================================
    [[nodiscard]] std::optional<std::array<uint8_t, 32>> getCurrentTransactionHash(
        IN const size_t& startReadPosition,
        IN const size_t& finishReadPosition
    );

    //================================================================================
    // Method: XorObfuscation
    // Description: Applies XOR-based obfuscation to the provided data buffer using 
    //              the stored XOR key. Each byte of the data is XOR'ed with a 
    //              corresponding key byte, chosen by using the absolute read position.
    //              Returns true on successful obfuscation, false otherwise.
    //================================================================================
    [[nodiscard]] bool xorObfuscation(IN const size_t readPosition, INOUT std::vector<uint8_t>& data);


    //================================================================================
    // Method: ReadData
    // Description: Reads a specified number of bytes from the block file into the
    //              output vector. Updates the internal read position after a
    //              successful read. If XOR obfuscation is enabled, applies it to
    //              the data using the original pre-read file position. Returns true
    //              on success, false if the file read or obfuscation fails.
    //================================================================================
    [[nodiscard]] bool readData(IN const size_t bytes, OUT std::vector<uint8_t>& out);


    //================================================================================
    // Method: ReadDataUint16
    // Description: Reads sizeof(uint16_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint16_t value. Uses LeUint8ToUint16 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] bool readDataUint16(OUT uint16_t& out);


    //================================================================================
    // Method: ReadDataUint32
    // Description: Reads sizeof(uint32_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint32_t value. Uses LeUint8ToUint32 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] bool readDataUint32(OUT uint32_t& out);
    

    //================================================================================
    // Method: ReadDataUint64
    // Description: Reads sizeof(uint64_t) bytes from the block file and converts them 
    //              from little-endian encoding into uint64_t value. Uses LeUint8ToUint64 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] bool readDataUint64(OUT uint64_t& out);
    

    //================================================================================
    // Method: ReadDataInt32
    // Description: Reads sizeof(int32_t) bytes from the block file and converts them 
    //              from little-endian encoding into int32_t value. Uses LeUint8ToInt32 
    //              for the conversion. Returns true on successful read and conversion, 
    //              false otherwise.
    //================================================================================
    [[nodiscard]] bool readDataInt32(OUT int32_t& out);


    //================================================================================
    // Method: ReadDataArray32
    // Description: Reads 32 bytes from the block file and stores them in
    //              std::array<uint8_t, 32>. Performs reverse_copy to account 
    //              little-endian encoding. Returns true if 32 bytes are successfully 
    //              read and copied, false if insufficient data is available.
    //================================================================================
    [[nodiscard]] bool readDataArray32(OUT std::array<uint8_t, 32>& out);


    //================================================================================
    // Method: ReadDataCompactSize
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
    [[nodiscard]] bool readDataCompactSize(IN const uint8_t firstByte, OUT uint64_t& out);


    //================================================================================
    // Method: ReadDataCompactSize
    // Description: Reads the first byte from the file and call overloaded 
    //              ReadDataCompactSize to decode the Bitcoin variable-length integer.
    //              Returns true on successful decoding, false if reading or 
    //              validation fails.
    //================================================================================
    [[nodiscard]] bool readDataCompactSize(OUT uint64_t& out);


    //================================================================================
    // Method: updateReadPosition
    // Description: Updates file read position to the input variable. 
    //              Returns true on success, false otherwise.
    //================================================================================
    [[nodiscard]] bool updateReadPosition(IN const size_t& newPos);

private:
    FileReader m_fileReader;
    size_t m_readPosition;
    size_t m_blockOffsetPosition;

    bool m_doXorObfuscation;
    std::vector<uint8_t> xorKey;
};
