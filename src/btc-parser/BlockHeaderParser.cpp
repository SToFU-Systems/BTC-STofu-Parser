// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockHeaderParser.h"
#include "Logger.hpp"
#include "Utils.h"


//================================================================================
// Method: BlockHeaderParser
// Description: Constructor. Stores a non-owning reference to BitcoinReader, which
//              provides raw data for block header parsing.
//================================================================================
BlockHeaderParser::BlockHeaderParser(IN BitcoinReader& reader)
    : m_reader(reader)
{
}

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
std::expected<BlockHeader, AppErrorCode> BlockHeaderParser::parse()
{
    LOG_VERBOSE("Started parsing block header.");
    BlockHeader header;

    // Version
    LOG_VERBOSE("Parsing version...");
    const auto kVersionReadResult = m_reader.readDataInt32();
    RETURN_IF_UNEXPECTED(kVersionReadResult);

    header.m_version = kVersionReadResult.value();

    // Hash previous block
    LOG_VERBOSE("Parsing hash of previous block...");
    auto kHashPrevReadResult = m_reader.readDataArray32();
    RETURN_IF_UNEXPECTED(kHashPrevReadResult);

    header.m_hashPrevBlock = std::move(kHashPrevReadResult).value();

    // Hash merkle root
    LOG_VERBOSE("Parsing hash of merkle root...");
    auto kHashMerkleReadResult = m_reader.readDataArray32();
    RETURN_IF_UNEXPECTED(kHashMerkleReadResult);

    header.m_hashMerkleRoot = kHashMerkleReadResult.value();

    // Time 
    LOG_VERBOSE("Parsing time...");
    const auto kTimeReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kTimeReadResult);

    header.m_time = kTimeReadResult.value();

    // Bits
    LOG_VERBOSE("Parsing bits...");
    const auto kBitsReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kBitsReadResult);

    header.m_bits = kBitsReadResult.value();

    // Nonce
    LOG_VERBOSE("Parsing nonce...");
    const auto kNonceReadResult = m_reader.readDataUint32();
    RETURN_IF_UNEXPECTED(kNonceReadResult);

    header.m_nonce = kNonceReadResult.value();

    LOG_VERBOSE("Successfully parsed block header.");
    return header;
}

//================================================================================
// Method: getBlockHash
// Description: Reads the current block header from the file, computes its hash 
//              (double SHA-256, reversed). Returns the generated hash on success 
//              or AppErrorCode on failure.
//================================================================================
std::expected<std::array<uint8_t, 32>, AppErrorCode> BlockHeaderParser::getBlockHash()
{
    constexpr size_t kBlockHeaderSize = sizeof(BlockHeader);
    const size_t kStartReadPosition = m_reader.getReadPosition();

    LOG_VERBOSE("Generating current block hash...");

    // Read 80 bytes (block header)
    auto kReadResult = m_reader.readData(kBlockHeaderSize);
    RETURN_IF_UNEXPECTED(kReadResult);

    std::vector<uint8_t> fullBlockHeader = std::move(kReadResult).value();

    // Generate hash
    std::array<uint8_t, 32> hashResult = getDoubleSHA256(fullBlockHeader);
    std::reverse(hashResult.begin(), hashResult.end());

    // Set read position back
    const bool kSetPosResult = m_reader.setReadPosition(kStartReadPosition);
    if (!kSetPosResult)
    {
        LOG_ERROR("setReadPosition failure.");
        return std::unexpected(AppErrorCode::FileError);
    }

    return hashResult;
}
