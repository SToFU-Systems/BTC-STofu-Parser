#pragma once

// Standard Library headers
#include <cstdint>
#include <string>
#include <vector>


// https://rosettacode.org/wiki/RIPEMD-160?utm_source=chatgpt.com#C++ url

//================================================================================
// Class: RIPEMD160
// Description: Implements the RIPEMD-160 cryptographic hash algorithm, including
//              message padding, internal bitwise operations, and a public interface 
//              compatible with OpenSSL for hash computation.
//================================================================================
class RIPEMD160
{
public:

    //================================================================================
    // Method: compute
    // Description: Computes the RIPEMD160 hash of input data. Initializes an internal
    //              instance, generates a hex digest via message_digest(), and writes
    //              the binary result (20 bytes) into the provided output buffer.
    //================================================================================
    static uint8_t* compute(const uint8_t* data, const size_t len, uint8_t* out);


    //================================================================================
    // Method: message_digest
    // Description: Computes the RIPEMD-160 digest for a given message string and returns
    //              the hash as a hexadecimal string.
    //================================================================================
    std::string message_digest(const std::string& message);


private:

    //================================================================================
    // Method: add_padding
    // Description: Adds RIPEMD-160 padding to the input message according to the algorithm 
    //              specification, preparing it for block processing.
    //================================================================================
    std::vector<uint8_t> add_padding(const std::string& message);


    //================================================================================
    // Method: ff
    // Description: RIPEMD-160 internal nonlinear function performing bitwise operations 
    //              used during message compression.
    //================================================================================
    uint32_t ff(const uint32_t& group, const uint32_t& x, const uint32_t& y, const uint32_t& z);


    //================================================================================
    // Method: unsigned_right_shift
    // Description: Performs an unsigned right shift operation on a signed 32-bit integer.
    //================================================================================
    int32_t unsigned_right_shift(const int32_t& base, const int32_t& shift);

private:
    const std::vector<uint32_t> SR = 
    {
        8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
        9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
        9,  7, 15, 11,  8,  6,  6, 14, 12, 13,  5, 14, 13, 13,  7,  5,
        15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
        8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11
    };

    const std::vector<uint32_t> SL = 
    {
        11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8,
        7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
        11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
        11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
        9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6
    };

    const std::vector<uint32_t> RR = 
    {
        5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
        6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
        15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
        8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
        12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11
    };

    const std::vector<uint32_t> RL = 
    {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
        3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
        1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
        4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13
    };

    const std::vector<uint32_t> KL = 
    {
        0x00000000, 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xa953fd4e
    };

    const std::vector<uint32_t> KR = 
    {
        0x50a28be6, 0x5c4dd124, 0x6d703ef3, 0x7a6d76e9, 0x00000000
    };

    const uint32_t BLOCK_LENGTH = 64;
};

//================================================================================
// Function: RIPEMD160T
// Description: External function wrapper for computing RIPEMD-160 hash using raw 
//              input buffer. Returns pointer to the output hash bytes.
//================================================================================
uint8_t* RIPEMD160T(const uint8_t* data, const size_t len, uint8_t* out);
