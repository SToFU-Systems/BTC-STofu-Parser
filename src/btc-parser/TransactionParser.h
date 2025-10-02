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
#include "TransactionScriptParser.h"


class TransactionParser
{
public:
    //================================================================================
    // Method: TransactionParser
    // Description: Constructor. Stores a non-owning reference to a BitcoinReader
    //              instance, which provides raw data for transaction parsing.
    //================================================================================
    explicit TransactionParser(IN BitcoinReader& reader);


    //================================================================================
    // Method: parseAllTransactions
    // Description: Reads the transaction counter and sequentially parses that many
    //              transactions. Returns a vector of parsed Transaction objects on
    //              success, or an AppErrorCode if any parsing step fails.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<Transaction>, AppErrorCode> parseAllTransactions();


    //================================================================================
    // Method: parseOneTransaction
    // Description: Parses a single Bitcoin transaction from the reader. Extracts the
    //              version, optional segwit marker/flag, inputs, outputs, optional
    //              witness data, and lock time. Also computes and stores the
    //              transaction hash. Returns the populated Transaction on success or
    //              an AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<Transaction, AppErrorCode> parseOneTransaction();


private:
    //================================================================================
    // Method: parseTransactionCounter
    // Description: Reads a CompactSize-encoded integer representing the number of
    //              transactions in the current block. Returns the count on success or
    //              AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<uint64_t, AppErrorCode> parseTransactionCounter();


    //================================================================================
    // Method: parseTransactionFlag
    // Description: Attempts to read the optional segwit flag sequence (0x00 0x01). 
    //              Returns true if present, false if absent, and rewinds the reader 
    //              if not found. Returns AppErrorCode on error.
    //================================================================================
    [[nodiscard]] std::expected<bool, AppErrorCode> parseTransactionFlag();


    //================================================================================
    // Method: parseAllTxIn
    // Description: Reads the input counter and parses each transaction input. Returns
    //              a vector of TxIn objects on success or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<TxIn>, AppErrorCode> parseAllTxIn();


    //================================================================================
    // Method: parseTxIn
    // Description: Parses a single transaction input (TxIn). Steps:
    //              - Reads the previous transaction hash.
    //              - Reads the output index being referenced.
    //              - Reads the script length.
    //              - Reads the scriptSig data.
    //              - Reads the sequence number.  
    //              Returns a TxIn on success or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<TxIn, AppErrorCode> parseTxIn();


    //================================================================================
    // Method: parseAllTxOut
    // Description: Reads the output counter and parses each transaction output.
    //              Returns a vector of TxOut objects on success or AppErrorCode on
    //              failure.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<TxOut>, AppErrorCode> parseAllTxOut();


    //================================================================================
    // Method: parseTxOut
    // Description: Parses a single transaction output (TxOut). Steps:
    //              - Reads the output value in satoshis (uint64).
    //              - Reads the script length (CompactSize).
    //              - Reads the scriptPubKey data.  
    //              Returns a TxOut on success or AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<TxOut, AppErrorCode> parseTxOut();


    //================================================================================
    // Method: parseAllWitnesses
    // Description: Parses all witness data for a segwit transaction. Reads the given
    //              number of witness stacks, parsing each one in sequence, and stores
    //              them in the output vector. Returns the vector on success or
    //              AppErrorCode if any witness fails to parse.
    //================================================================================
    [[nodiscard]] std::expected<std::vector<Witness>, AppErrorCode> parseAllWitnesses(IN const size_t witCount);


    //================================================================================
    // Method: parseWitness
    // Description: Parses a single witness stack. Reads the element count, then for
    //              each element reads its length and data bytes. Returns a Witness on
    //              success or an AppErrorCode on failure.
    //================================================================================
    [[nodiscard]] std::expected<Witness, AppErrorCode> parseWitness();


    //================================================================================
    // Method: getCurrentTransactionHash
    // Description: Computes the transaction hash (double SHA-256, reversed) for the
    //              transaction defined by start and finish positions. Returns the
    //              computed hash on success, otherwise AppErrorCode.
    //================================================================================
    [[nodiscard]] std::expected<std::array<uint8_t, 32>, AppErrorCode> getCurrentTransactionHash(
        IN const size_t& startReadPosition,
        IN const size_t& finishReadPosition
    );


private:
    BitcoinReader& m_reader;
    TransactionScriptParser m_scriptParser;
};
