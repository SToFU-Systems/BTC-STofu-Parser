/*
 *  MIT License
 *
 *  Copyright (c) 2021 David Turner
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

//Precompiled Headers
#include "pch.h"

// Project headers
#include "ripemd160.h"

/* Constants etc. taken directly from https://homes.esat.kuleuven.be/~bosselae/ripemd160/pdf/AB-9601/AB-9601.pdf */

namespace
{
    constexpr size_t kDigestWords = 5ULL;
    constexpr size_t kRoundSize = 16ULL;
    constexpr size_t kNumShifts = 80ULL;

    constexpr std::array<uint32_t, kDigestWords> ripemd160_initial_digest =
    { 0x67452301UL, 0xefcdab89UL, 0x98badcfeUL, 0x10325476UL, 0xc3d2e1f0UL };

    constexpr std::array<uint8_t, kRoundSize> ripemd160_rho =
    { 0x7, 0x4, 0xd, 0x1, 0xa, 0x6, 0xf, 0x3, 0xc, 0x0, 0x9, 0x5, 0x2, 0xe, 0xb, 0x8 };

    constexpr std::array<uint8_t, kNumShifts> ripemd160_shifts =
    { 11, 14, 15, 12, 5, 8, 7, 9, 11, 13, 14, 15, 6, 7, 9, 8
    , 12, 13, 11, 15, 6, 9, 9, 7, 12, 15, 11, 13, 7, 8, 7, 7
    , 13, 15, 14, 11, 7, 7, 6, 8, 13, 14, 13, 12, 5, 5, 6, 9
    , 14, 11, 12, 14, 8, 6, 5, 5, 15, 12, 15, 14, 9, 9, 8, 6
    , 15, 12, 13, 13, 9, 5, 8, 6, 14, 11, 12, 11, 8, 6, 5, 5
    };

    constexpr std::array<uint32_t, kDigestWords> ripemd160_constants_left =
    { 0x00000000UL, 0x5a827999UL, 0x6ed9eba1UL, 0x8f1bbcdcUL, 0xa953fd4eUL };

    constexpr std::array<uint32_t, kDigestWords> ripemd160_constants_right =
    { 0x50a28be6UL, 0x5c4dd124UL, 0x6d703ef3UL, 0x7a6d76e9UL, 0x00000000UL };

    constexpr std::array<uint8_t, kDigestWords> ripemd160_fns_left = { 1, 2, 3, 4, 5 };
    constexpr std::array<uint8_t, kDigestWords> ripemd160_fns_right = { 5, 4, 3, 2, 1 };



    //================================================================================
    // Function: ROL
    // Description: Performs a 32-bit rotate-left operation on the given value.
    //================================================================================
    constexpr uint32_t ROL(IN uint32_t x, IN uint32_t n)
    {
        constexpr uint32_t kWordBits = 32U;

        uint32_t a = (x) << (n);
        uint32_t b = ((x) >> (kWordBits - (n)));

        return (a | b);
    }

    //================================================================================
    // Function: ripemd160_compute_round
    // Description: Executes one RIPEMD-160 round (16 steps) for the current "words" state.
    //================================================================================
    static void ripemd160_compute_round(
        IN    const uint32_t* chunk,
        IN    const std::array<uint8_t, kNumShifts>& shifts,
        IN    uint8_t fn,
        IN    uint32_t k,
        IN    size_t shift_offset,
        INOUT std::array<uint32_t, kDigestWords>& words,
        INOUT const std::array<uint8_t, kRoundSize>& index
    )
    {
        constexpr uint32_t kRotateConst = 10U;

        for (uint8_t i = 0; i < kRoundSize; i++)
        {
            uint32_t tmp{};
            switch (fn)
            {
            case 1:
                tmp = words[1] ^ words[2] ^ words[3];
                break;
            case 2:
                tmp = (words[1] & words[2]) | (~words[1] & words[3]);
                break;
            case 3:
                tmp = (words[1] | ~words[2]) ^ words[3];
                break;
            case 4:
                tmp = (words[1] & words[3]) | (words[2] & ~words[3]);
                break;
            case 5:
                tmp = words[1] ^ (words[2] | ~words[3]);
                break;
            }

            tmp += words[0] + chunk[index[i]] + k;
            tmp = ROL(tmp, shifts[shift_offset + index[i]]) + words[4];
            words[0] = words[4];
            words[4] = words[3];
            words[3] = ROL(words[2], kRotateConst);
            words[2] = words[1];
            words[1] = tmp;
        }
    }

    //================================================================================
    // Function: ripemd160_compute_line
    // Description: Internal function performing one RIPEMD-160 transform line.
    //================================================================================
    void ripemd160_compute_line(
        IN    const uint32_t* chunk,
        IN    const std::array<uint8_t, kNumShifts>& shifts,
        IN    const std::array<uint32_t, kDigestWords>& ks,
        IN    const std::array<uint8_t, kDigestWords>& fns,
        INOUT uint32_t* digest,
        INOUT std::array<uint8_t, kRoundSize>& index,
        OUT   std::array<uint32_t, kDigestWords>& words
    )
    {
        std::copy_n(digest, kDigestWords, words.begin());

        size_t shift_offset = 0;
        for (uint8_t round = 0; /* breaks out mid-loop */; round++)
        {
            uint32_t k = ks[round];
            uint8_t  fn = fns[round];

            ripemd160_compute_round( chunk, shifts, fn, k, shift_offset, words, index);

            if (round == 4)
                break;

            shift_offset += kRoundSize;
            std::array<uint8_t, kRoundSize> index_tmp{};

            for (uint8_t i = 0; i < index_tmp.size(); i++)
            {
                index_tmp[i] = ripemd160_rho[index[i]];
            }

            std::copy_n(index_tmp.begin(), index_tmp.size(), index.begin());
        }
    }

    //================================================================================
    // Function: ripemd160_update_digest
    // Description: Updates the digest using one 512-bit chunk of data.
    //================================================================================
    void ripemd160_update_digest(IN const uint32_t* chunk, INOUT uint32_t* digest)
    {
        std::array<uint8_t, kRoundSize> index{};
        //initial permutation for left line is the identity
        std::iota(index.begin(), index.end(), uint8_t{ 0 });

        std::array<uint32_t, kDigestWords> words_left{};
        ripemd160_compute_line(chunk,  ripemd160_shifts,  ripemd160_constants_left, ripemd160_fns_left, digest, index, words_left);

        //initial permutation for right line is 5+9i (mod 16)
        index[0] = 5;

        for (uint8_t i = 1; i < index.size(); i++)
        {
            index[i] = (index[i - 1] + 9) & 0x0f;
        }

        std::array<uint32_t, kDigestWords> words_right{};
        ripemd160_compute_line(chunk, ripemd160_shifts, ripemd160_constants_right, ripemd160_fns_right, digest, index, words_right);

        //update digest
        for (size_t i = 0; i < kDigestWords; ++i)
        {
            digest[i] += words_left[(i + 1) % 5] + words_right[(i + 2) % 5];
        }

        //final rotation
        words_left[0] = digest[0];
        digest[0] = digest[1];
        digest[1] = digest[2];
        digest[2] = digest[3];
        digest[3] = digest[4];
        digest[4] = words_left[0];
    }
}

//================================================================================
// Function: RIPEMD160T
// Description: Computes the RIPEMD-160 hash of the input buffer.
//================================================================================
void RIPEMD160T(IN std::span<const uint8_t> data, OUT std::span<uint8_t> digest_bytes)
{

    constexpr size_t   kBlockSizeBytes = 0x40ULL;// RIPEMD-160 block size: 64 bytes
    constexpr uint8_t  kBlockRemainderMask = 0x3fU;// Mask for data_len % 64 (0x3F = 63)
    constexpr uint8_t  kPaddingByte = 0x80U;// Padding start byte: 1000 0000 (one '1' bit)

    constexpr size_t   kLengthLswOffset = 0x38ULL; // Offset of low 32-bit message length (56)
    constexpr size_t   kLengthMswOffset = 0x3cULL; // Offset of high 32-bit message length (60)

    constexpr uint32_t kLowLengthShift = 3U;   // Shift left by 3: convert message length from bytes to bits
    constexpr uint32_t kHighLengthShift = 29U;  // Shift right by 29: upper 32 bits of the 64-bit bit-length

    const uint32_t data_len = static_cast<uint32_t>(data.size());

    //NB assumes correct endianness
    uint32_t* digest = reinterpret_cast<uint32_t*>(digest_bytes.data());

    std::copy_n(ripemd160_initial_digest.begin(), kDigestWords, digest);

    const uint8_t* last_chunk_start = data.data() + (data_len & (~kBlockRemainderMask));
    const uint8_t* ptr = data.data();
    while (ptr < last_chunk_start)
    {
        ripemd160_update_digest( reinterpret_cast< const uint32_t*>(ptr), digest);
        ptr += kBlockSizeBytes;
    }

    std::array<uint8_t, kBlockSizeBytes> last_chunk{};
    uint8_t leftover_size = data_len & kBlockRemainderMask;

    for (uint8_t i = 0; i < leftover_size; i++) 
    {
        last_chunk[i] = *ptr++;
    }

    //append a single 1 bit and then zeroes, leaving 8 bytes for the length at the end
    last_chunk[leftover_size] = kPaddingByte;

    std::fill_n(last_chunk.data() + (leftover_size + 1), kBlockSizeBytes - (leftover_size + 1), uint8_t{ 0 });

    if (leftover_size >= kLengthLswOffset) 
    {
        //no room for size in this chunk, add another chunk of zeroes
        ripemd160_update_digest(reinterpret_cast<uint32_t*>(last_chunk.data()), digest);

        std::fill_n(last_chunk.data(), kLengthLswOffset, uint8_t{0});
    }

    uint32_t* length_lsw = reinterpret_cast<uint32_t*>(last_chunk.data() + kLengthLswOffset);
    *length_lsw = (data_len << kLowLengthShift);
    uint32_t* length_msw = reinterpret_cast<uint32_t*>(last_chunk.data() + kLengthMswOffset);
    *length_msw = (data_len >> kHighLengthShift);

    ripemd160_update_digest(reinterpret_cast<uint32_t*>(last_chunk.data()), digest);
}
