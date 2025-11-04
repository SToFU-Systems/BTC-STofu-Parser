// Precompiled headers
#include "pch.h"

// Project headers
#include "BlockPrinter.h"
#include "Utils.h"
#include "JsonUtils.h"

//================================================================================
// Method: BlockPrinter
// Description: Initializes a BlockPrinter instance with the provided 
//              std::ostream reference, which will be used as the output 
//              destination for block printing.
//================================================================================
BlockPrinter::BlockPrinter(IN const Block& block, IN std::ostream& stream, IN const bool isJson)
    : 
    m_block(block),
    m_stream(stream),
    m_isJson(isJson)
{
}

//================================================================================
// Method: printBlock
// Description: Prints the content of a Bitcoin block to ostream. First prints 
//              the block header using PrintBlockHeader, then prints all 
//              transactions contained in the block using printAllTransactions.
//================================================================================
void BlockPrinter::printBlock() const
{
    if (m_isJson)
    {
        nlohmann::ordered_json jsonOutput;
        blockToJson(m_block, jsonOutput);
        m_stream << jsonOutput.dump(4) << '\n';
        
        return;
    }

    m_stream << "===Block===\n";

    // Current Block Hash
    m_stream << "* Current Block Hash: " << hashToHexString(m_block.m_currentBlockHash) << '\n';

    // Header
    printBlockHeader(m_block.m_blockHeader);

    // Transactions
    printAllTransactions();

    m_stream << '\n';
}

//================================================================================
// Method: printBlockHeader
// Description: Prints all fields of a Bitcoin block header to ostream.
//              Outputs version, previous block hash, Merkle root, timestamp,
//              difficulty bits, and nonce. 
//================================================================================
void BlockPrinter::printBlockHeader(IN const BlockHeader& header) const
{
    m_stream << "===Block Header===\n";
    m_stream << "* Version: " << header.m_version << '\n';
    m_stream << "* Previous Hash: " << hashToHexString(header.m_hashPrevBlock) << '\n';
    m_stream << "* Merkle Root: " << hashToHexString(header.m_hashMerkleRoot) << '\n';
    m_stream << "* Time: " << toTime(header.m_time) << '\n';
    m_stream << "* Bits: " << header.m_bits << '\n';
    m_stream << "* Nonce: " << std::hex << header.m_nonce << std::dec << '\n';
}

//================================================================================
// Method: printAllTransactions
// Description: Iterates through all transactions in the block to output their 
//              details using printTransaction.
//================================================================================
void BlockPrinter::printAllTransactions() const
{
    const size_t kTransactionsCounter = m_block.m_transactions.size();
    for (size_t i = 0; i < kTransactionsCounter; ++i)
    {
        m_stream << "\n===Transaction #" << i + 1 << "===\n";
        printTransaction(m_block.m_transactions[i]);
    }
}

//================================================================================
// Method: printTransaction
// Description: Prints all components of a Bitcoin transaction to ostream.
//              Outputs version, segwit flag, number of inputs,
//              number of outputs and lock time. Additionally calls helper methods
//              to print input/output transactions and witness data (if present).
//================================================================================
void BlockPrinter::printTransaction(IN const Transaction& transaction) const
{
    m_stream << "* Current Transaction Hash: " << hashToHexString(transaction.m_currentTransactionHash) << '\n';
    m_stream << "* Version: " << transaction.m_version << '\n';
    m_stream << "* SegWit Flag: " << transaction.m_segwitFlag << '\n';
    m_stream << "* Input Transactions: " << transaction.m_vIn.size() << '\n';
    m_stream << "* Output Transactions: " << transaction.m_vOut.size() << '\n';

    printTxIn(transaction.m_vIn);
    printTxOut(transaction.m_vOut);
    
    if(transaction.m_segwitFlag)
        printWitness(transaction.m_witnesses);

    m_stream << "\n* Lock Time: " << transaction.m_lockTime << '\n';
}

//================================================================================
// Method: printTxIn
// Description: Prints all input transactions stored in Bitcoin transaction to 
//              ostream. For each input, prints previous transaction hash, script 
//              signature, and sequence number. 
//================================================================================
void BlockPrinter::printTxIn(IN const std::vector<TxIn>& txIn) const
{
    for (size_t i = 0; i < txIn.size(); ++i)
    {
        m_stream << "\n===Input Transaction #" << i + 1 << "===\n";
        m_stream << "* Previous Transaction: " << hashToHexString(txIn[i].m_hashPreviousTx) << '\n';
        m_stream << "* Script Signature: " << hashToHexString(txIn[i].m_scriptSignature) << '\n';
        m_stream << "* Input address: " << txIn[i].m_inputAddress << '\n';
        m_stream << "* Sequence Number: " << std::hex << txIn[i].m_sequenceNumber << std::dec << '\n';
    }
}

//================================================================================
// Method: printTxOut
// Description: Prints all output transactions stored in Bitcoin transaction to 
//              ostream. For each output, prints satoshi value and scriptPubKey.
//================================================================================
void BlockPrinter::printTxOut(IN const std::vector<TxOut>& txOut) const
{
    for (size_t i = 0; i < txOut.size(); ++i)
    {
        m_stream << "\n===Output Transaction #" << i + 1 << "===\n";
        m_stream << "* Satoshi Value: " << txOut[i].m_value << '\n';
        m_stream << "* Script PubKey: " << hashToHexString(txOut[i].m_scriptPubKey) << '\n';
        m_stream << "* Output address: " << txOut[i].m_outputAddress << '\n';
    }
}

//================================================================================
// Method: printWitness
// Description: Prints all witness data stored in Bitcoin transaction to ostream.
//================================================================================
void BlockPrinter::printWitness(IN const std::vector<Witness>& witness) const
{
    for (size_t i = 0; i < witness.size(); ++i)
    {
        m_stream << "\n===Witness #" << i + 1 << "===\n";
        for (const auto& data : witness[i].m_witnessData)
        {
            m_stream << "* " << hashToHexString(data) << '\n';
        }
    }
}
