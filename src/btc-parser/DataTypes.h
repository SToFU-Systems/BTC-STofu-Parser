#pragma once

// Standard Library headers
#include <array>
#include <cstdint>
#include <vector>
#include <string>


struct BlockHeader
{
    int32_t m_version;
    std::array<uint8_t, 32> m_hashPrevBlock;
    std::array<uint8_t, 32> m_hashMerkleRoot;
    uint32_t m_time;
    uint32_t m_bits;
    uint32_t m_nonce;
};

struct TxIn
{
    std::array<uint8_t, 32> m_hashPreviousTx;
    uint32_t m_prevTxOutIndex;
    // txin script length
    std::vector<uint8_t> m_scriptSignature;
    uint32_t m_sequenceNumber;

    /* Not in the block, but important to have */
    std::string m_inputAddress;
};

struct TxOut
{
    uint64_t m_value;
    // script length
    std::vector<uint8_t> m_scriptPubKey;

    /* Not in the block, but important to have */
    std::string m_outputAddress;
};

struct Witness
{
    std::vector<std::vector<uint8_t>> m_witnessData;
};

struct Transaction
{
    int32_t m_version;
    bool m_segwitFlag;
    // in-counter
    std::vector<TxIn> m_vIn;
    // out-counter
    std::vector<TxOut> m_vOut;
    std::vector<Witness> m_witnesses;
    uint32_t m_lockTime;

    /* Not in the block, but important to have */
    std::array<uint8_t, 32> m_currentTransactionHash;
};

struct Block
{
    // magic
    // block size
    BlockHeader m_blockHeader;
    // transaction counter
    std::vector<Transaction> m_transactions;

    /* Not in the block, but important to have */
    std::array<uint8_t, 32> m_currentBlockHash;
};
