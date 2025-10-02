#pragma once

// Standard Library headers
#include <array>
#include <cstdint>
#include <expected>
#include <vector>

// Project headers
#include "AppErrorCode.hpp"
#include "BitcoinReader.h"
#include "Common.h"
#include "DataTypes.h"


//================================================================================
// Class: SingleBlockParser
// Description: Responsible for reading and parsing raw Bitcoin block data from
//              *.dat files. Provides methods to read data from file and handles 
//              optional XOR obfuscation for the newest Bitcoin Core versions. 
//              Ensures correctness by validating magic numbers, block sizes, 
//              and transaction boundaries. Supports both parsing of a single 
//              block and sequential parsing of all blocks in a file.
//================================================================================
class SingleBlockParser
{
public:
    //================================================================================
    // Method: SingleBlockParser
    // Description: Constructor. Initializes a SingleBlockParser with a non-owning
    //              reference to the given BitcoinReader and prepares internal state
    //              for block parsing.
    //================================================================================
    explicit SingleBlockParser(IN BitcoinReader& reader);


    /* Deleted */
    SingleBlockParser(const SingleBlockParser&) = delete;
    SingleBlockParser(SingleBlockParser&&) noexcept = delete;
    SingleBlockParser& operator=(const SingleBlockParser&) = delete;
    SingleBlockParser& operator=(SingleBlockParser&&) noexcept = delete;


    //================================================================================
    // Method: ~BlockParser
    // Description: Defaulted destructor.
    //================================================================================
    ~SingleBlockParser() = default;


    //================================================================================
    // Method: parse
    // Description: Parses a single Bitcoin block from the underlying reader. Steps:
    //              1. Reads and validates the magic number.
    //              2. Reads and validates the declared block size.
    //              3. Uses BlockHeaderParser to compute the block hash and parse
    //                 the block header.
    //              4. Uses TransactionParser to read all transactions in the block.
    //              5. Verifies that the reader consumed exactly the declared number
    //                 of bytes for the block.
    //              Returns the fully populated Block on success or an error code
    //              wrapped in std::expected if any step fails.
    //================================================================================
    [[nodiscard]] std::expected<Block, AppErrorCode> parse();


private:
    //================================================================================
    // Method: parseMagicNumber
    // Description: Reads a 32-bit magic number from the file and checks that it
    //              matches the expected mainnet constant. Returns the magic number
    //              if valid or AppErrorCode if invalid.
    //================================================================================
    [[nodiscard]] std::expected<uint32_t, AppErrorCode> parseMagicNumber();


    //================================================================================
    // Method: ParseBlockSize
    // Description: Reads the block size from file. Ensures that the
    //              size is greater than zero. Returns true on success, false if
    //              reading fails or the size is invalid.
    //================================================================================
    [[nodiscard]] std::expected<uint32_t, AppErrorCode> parseBlockSize();


private:
    BitcoinReader& m_reader;
};
