#pragma once

// Project headers
#include "Common.h"
#include "DataTypes.h"


//================================================================================
// Class: ConsoleBlockPrinter
// Description: Provides a formatted, human-readable console output of parsed
//              Bitcoin block structures.
//================================================================================
class ConsoleBlockPrinter
{
public:
    //================================================================================
    // Method: ConsoleBlockPrinter
    // Description: Initializes a ConsoleBlockPrinter instance with the provided 
    //              std::ostream reference, which will be used as the output 
    //              destination for block printing.
    //================================================================================
    explicit ConsoleBlockPrinter(const Block& block, std::ostream& stream);


    //================================================================================
    // Method: printBlock
    // Description: Prints the content of a Bitcoin block to ostream. First prints 
    //              the block header using PrintBlockHeader, then prints all 
    //              transactions contained in the block using printAllTransactions.
    //================================================================================
    void printBlock() const;


private:
    //================================================================================
    // Method: printBlockHeader
    // Description: Prints all fields of a Bitcoin block header to ostream.
    //              Outputs version, previous block hash, Merkle root, timestamp,
    //              difficulty bits, and nonce. 
    //================================================================================
    void printBlockHeader(IN const BlockHeader& header) const;


    //================================================================================
    // Method: printAllTransactions
    // Description: Iterates through all transactions in the block to output their 
    //              details using printTransaction.
    //================================================================================
    void printAllTransactions() const;


    //================================================================================
    // Method: printTransaction
    // Description: Prints all components of a Bitcoin transaction to ostream.
    //              Outputs version, segwit flag, number of inputs,
    //              number of outputs and lock time. Additionally calls helper methods
    //              to print input/output transactions and witness data (if present).
    //================================================================================
    void printTransaction(IN const Transaction& transaction) const;


    //================================================================================
    // Method: printTxIn
    // Description: Prints all input transactions stored in Bitcoin transaction to 
    //              ostream. For each input, prints previous transaction hash, script 
    //              signature, and sequence number. 
    //================================================================================
    void printTxIn(IN const std::vector<TxIn>& txIn) const;

   
    //================================================================================
    // Method: printTxOut
    // Description: Prints all output transactions stored in Bitcoin transaction to 
    //              ostream. For each output, prints satoshi value and scriptPubKey.
    //================================================================================
    void printTxOut(IN const std::vector<TxOut>& txOut) const;


    //================================================================================
    // Method: printWitness
    // Description: Prints all witness data stored in Bitcoin transaction to ostream.
    //================================================================================
    void printWitness(IN const std::vector<std::vector<uint8_t>>& witness) const;


private:
    const Block& m_block;
    std::ostream& m_stream;
};
