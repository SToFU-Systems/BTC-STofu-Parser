#pragma once

// Standard Library headers
#include <span>

// Project headers
#include "Common.h"
#include "DataTypes.h"


class TransactionScriptParser
{
public:
    //================================================================================
    // Method: parseInputScript
    // Description: Decodes Bitcoin input script to generate an input address. 
    //              Supports parsing of P2SH-P2WPKH redeem scripts and standard P2PKH 
    //              scripts. Returns "Unknown" if the input script cannot be parsed.
    //================================================================================
    std::string parseInputScript(IN std::span<const uint8_t> txIn);


    //================================================================================
    // Method: parseOutputScript
    // Description: Decodes Bitcoin output script to generate an output address. 
    //              Supports P2PKH, P2SH, P2WPKH, P2WSH, and Taproot scripts. Returns 
    //              "OP_RETURN" for data-carrying outputs and "Unknown" for 
    //              unrecognized scripts.
    //================================================================================
    std::string parseOutputScript(IN std::span<const uint8_t> txOut);


private:
    //================================================================================
    // Method: decodeScriptSigP2PKH
    // Description: Decodes a P2PKH input script and extracts the public key. Then 
    //              computes the SHA-256 hash of the public key, followed by a 
    //              RIPEMD-160 hash, to derive the public key hash. Finally, it 
    //              prepends the P2PKH version byte and returns Base58Check-encoded 
    //              address. Returns "Unknown" if the script is malformed or too short
    //              to contain a valid public key.
    //================================================================================
    std::string decodeScriptSigP2PKH(IN std::span<const uint8_t> scriptSig);


    //================================================================================
    // Method: base58Check
    // Description: Encodes a version-prefixed payload into a Base58Check Bitcoin 
    //              address. Appends a checksum (first 4 bytes of double SHA256) to 
    //              the input, then encodes using the Base58 alphabet. Preserves 
    //              leading zeros as '1' characters in the result string.
    //================================================================================
    std::string base58Check(
        IN std::span<const uint8_t> version, 
        IN std::span<const uint8_t> payload
    );


    //================================================================================
    // Method: bech32Encode
    // Description: Encodes witness program data into a Bech32 Bitcoin address using
    //              the provided human-readable prefix, witness version, and witness 
    //              program. Applies checksum generation and appends it to the result. 
    //              Returns the full Bech32 string.
    //================================================================================
    std::string bech32Encode(
        IN const std::string& prefix,
        IN const int witVersion,
        IN std::span<const uint8_t> witProgram
    );


    //================================================================================
    // Method: convertBits
    // Description: Converts a stream of input data from one bit resolution to another,
    //              commonly from 8-bit bytes to 5-bit groups for Bech32 encoding. Can
    //              optionally pad the result if leftover bits remain. Returns container
    //              of converted values.
    //================================================================================
    std::vector<uint32_t> convertBits(
        IN std::span<const uint8_t> data,
        IN const int fromBits,
        IN const int toBits,
        IN const bool pad = true
    );


    //================================================================================
    // Method: hrpExpand
    // Description: Expands the human-readable prefix (HRP) for Bech32 encoding by
    //              splitting each character into high and low bits and adding a zero
    //              separator. Returns the expanded HRP values as integers.
    //================================================================================
    std::vector<uint32_t> hrpExpand(IN const std::string& prefix);


    //================================================================================
    // Method: bech32Polymod
    // Description: Core Bech32 checksum function that computes a 30-bit cyclic
    //              redundancy checksum over input values. Uses generator constants
    //              defined by the Bech32 specification. Returns the polymod result
    //              used in checksum calculation.
    //================================================================================
    uint32_t bech32Polymod(IN std::span<const uint32_t> values);


    //================================================================================
    // Method: createBech32Checksum
    // Description: Produces a 6-character checksum for a Bech32 string. Expands the
    //              human-readable prefix, appends data, and computes the checksum 
    //              using the polymod algorithm. Returns the checksum values to be 
    //              appended to the Bech32 address.
    //================================================================================
    std::vector<uint32_t> createBech32Checksum(
        IN const std::string& prefix,
        IN std::span<const uint32_t> data,
        IN const size_t xorValue
    );
};
