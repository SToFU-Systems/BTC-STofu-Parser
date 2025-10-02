#pragma once

// Standard Library headers
#include <array>
#include <cstdint>
#include <expected>

// Project headers
#include "AppErrorCode.hpp"
#include "BitcoinReader.h"
#include "Common.h"
#include "DataTypes.h"


class BlockHeaderParser
{
public:
    //================================================================================
    // Method: BlockHeaderParser
    // Description: Constructor. Stores a non-owning reference to BitcoinReader, which
    //              provides raw data for block header parsing.
    //================================================================================
    explicit BlockHeaderParser(IN BitcoinReader& reader);


    //================================================================================
    // Method: parse
    // Description: Parses the block header data. Steps:
    //              - Reads the version.
    //              - Reads the previous block hash.
    //              - Reads the Merkle root hash.
    //              - Reads the timestamp.
    //              - Reads the difficulty bits.
    //              - Reads the nonce.
    //              Returns the populated BlockHeader on success or AppErrorCode on 
    //              failure.
    //================================================================================
    [[nodiscard]] std::expected<BlockHeader, AppErrorCode> parse();


    //================================================================================
    // Method: getBlockHash
    // Description: Reads the current block header from the file, computes its hash 
    //              (double SHA-256, reversed). Returns the generated hash on success 
    //              or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<std::array<uint8_t, 32>, AppErrorCode> getBlockHash();


private:
    BitcoinReader& m_reader;
};
