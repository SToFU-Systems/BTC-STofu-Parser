// Precompiled headers
#include "pch.h"

// Project headers
#include "TransactionScriptParser.h"
#include "Utils.h"
#include "Ripemd160.h"


namespace
{
    //================================================================================
    // Enum: ScriptOp
    // Description: Strongly typed Bitcoin Script opcode constants used for parsing
    //              standard scripts (P2PKH, P2SH, SegWit, Taproot).
    //================================================================================
    enum class ScriptOp : uint8_t
    {
        Op0 = 0x00, // empty array of bytes is pushed onto the stack
        Op1 = 0x51, // number 1 is pushed onto the stack
        OpDup = 0x76, // duplicates top stack item
        OpHash160 = 0xA9, // input is hashed twice: first with SHA-256 and then with RIPEMD-160
        OpEqual = 0x87, // returns 1 if the inputs are exactly equal, 0 otherwise
        OpEqualVerify = 0x88, // same as OpEqual, but verifies top stack value afterward 
        OpCheckSig = 0xAC, // all transaction's outputs, inputs, and script are hashed 
        OpReturn = 0x6A, // marks transaction as invalid.
        OpPushBytes20 = 0x14, // followed by 20 bytes of data
        OpPushBytes22 = 0x16, // followed by 22 bytes of data
        OpPushBytes32 = 0x20, // followed by 32 bytes of data
        OpPushData1 = 0x4C, // next byte contains the number of bytes to be pushed onto the stack
        OpPushData2 = 0x4D, // next two bytes contain the number of bytes to be pushed onto the stack in little endian order
        OpPushData4 = 0x4E // next four bytes contain the number of bytes to be pushed onto the stack in little endian order
    };

    //================================================================================
    // Function: convertScriptOp
    // Description: Converts a ScriptOp enum value into its underlying uint8_t
    //              representation.
    //================================================================================
    constexpr uint8_t convertScriptOp(IN const ScriptOp code)
    {
        return static_cast<uint8_t>(code);
    }

    //================================================================================
    // Method: isP2PKH
    // Description: Returns true when the script strictly matches the legacy
    //              Pay-to-PubKey-Hash pattern:
    //              OP_DUP OP_HASH160 PUSHDATA(20) <20-byte pubkey-hash>
    //              OP_EQUALVERIFY OP_CHECKSIG.
    //              Expects total script length of 25 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isP2PKH(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 25;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::OpDup))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::OpHash160))
            return false;

        constexpr size_t kPushPos = 2;
        if (script[kPushPos] != convertScriptOp(ScriptOp::OpPushBytes20))
            return false;

        constexpr size_t kEqualVerifyPos = 23;
        if (script[kEqualVerifyPos] != convertScriptOp(ScriptOp::OpEqualVerify))
            return false;

        constexpr size_t kCheckSigPos = 24;
        if (script[kCheckSigPos] != convertScriptOp(ScriptOp::OpCheckSig))
            return false;

        return true;
    }

    //================================================================================
    // Method: isP2SH
    // Description: Returns true when the script strictly matches the
    //              Pay-to-Script-Hash pattern:
    //              OP_HASH160 PUSHDATA(20) <20-byte script-hash> OP_EQUAL.
    //              Expects total script length of 23 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isP2SH(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 23;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::OpHash160))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::OpPushBytes20))
            return false;

        constexpr size_t kOpEqualPos = 22;
        if (script[kOpEqualPos] != convertScriptOp(ScriptOp::OpEqual))
            return false;

        return true;
    }

    //================================================================================
    // Method: isP2WPKH
    // Description: Returns true when the script matches SegWit v0 P2WPKH:
    //              OP_0 PUSHDATA(20) <20-byte pubkey-hash>.
    //              Expects total script length of 22 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isP2WPKH(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 22;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::Op0))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::OpPushBytes20))
            return false;

        return true;
    }

    //================================================================================
    // Method: isP2WSH
    // Description: Returns true when the script matches SegWit v0 P2WSH:
    //              OP_0 PUSHDATA(32) <32-byte script-hash>.
    //              Expects total script length of 34 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isP2WSH(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 34;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::Op0))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::OpPushBytes32))
            return false;

        return true;
    }

    //================================================================================
    // Method: isTaproot
    // Description: Returns true when the script matches Taproot (SegWit v1):
    //              OP_1 PUSHDATA(32) <32-byte x-only pubkey>.
    //              Expects total script length of 34 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isTaproot(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 34;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::Op1))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::OpPushBytes32))
            return false;

        return true;
    }

    //================================================================================
    // Method: isP2shP2wpkhRedeem
    // Description: Returns true when the script strictly matches the
    //              Pay-to-Script-Hash wrapped Pay-to-Witness-Public-Key-Hash
    //              redeem script pattern:
    //              OP_PUSHBYTES_22 OP_0 OP_PUSHBYTES_20 <20-byte key-hash>.
    //              Expects total script length of 23 bytes. Returns false otherwise.
    //================================================================================
    [[nodiscard]] bool isP2shP2wpkhRedeem(IN std::span<const uint8_t> script)
    {
        constexpr size_t kScriptLength = 23;
        if (script.size() != kScriptLength)
            return false;

        if (script[0] != convertScriptOp(ScriptOp::OpPushBytes22))
            return false;

        if (script[1] != convertScriptOp(ScriptOp::Op0))
            return false;

        constexpr size_t kPushPos = 2;
        if (script[kPushPos] != convertScriptOp(ScriptOp::OpPushBytes20))
            return false;

        return true;
    }
}

//================================================================================
// Method: parseInputScript
// Description: Decodes Bitcoin input script to generate an input address. 
//              Supports parsing of P2SH-P2WPKH redeem scripts and standard P2PKH 
//              scripts. Returns "Unknown" if the input script cannot be parsed.
//================================================================================
std::string TransactionScriptParser::parseInputScript(IN std::span<const uint8_t> inputScript)
{
    if (inputScript.empty())
        return "Unknown";

    // P2SH-P2WPKH redeem script: PUSHDATA22 OP_0 <20-byte program>
    if (isP2shP2wpkhRedeem(inputScript))
    {
        constexpr size_t kDataStartPos = 3;
        const std::string kMainnetPrefix = "bc";
        const size_t kWitVersion = 0;

        std::vector<uint8_t> witprog(inputScript.begin() + kDataStartPos, inputScript.end());
        return bech32Encode(kMainnetPrefix, 0, witprog);
    }

    // Standard P2PKH: <sig> <pubkey>
    return decodeScriptSigP2PKH(inputScript);
}

//================================================================================
// Method: parseOutputScript
// Description: Decodes Bitcoin output script to generate an output address. 
//              Supports P2PKH, P2SH, P2WPKH, P2WSH, and Taproot scripts. Returns 
//              "OP_RETURN" for data-carrying outputs and "Unknown" for the
//              unrecognized scripts.
//================================================================================
std::string TransactionScriptParser::parseOutputScript(IN std::span<const uint8_t> outputScript)
{
    if (outputScript.empty())
        return "Unknown";

    // OP_RETURN
    if (outputScript[0] == static_cast<uint8_t>(ScriptOp::OpReturn))
        return "OP_RETURN";

    // P2PKH: OP_DUP OP_HASH160 <20 bytes> OP_EQUALVERIFY OP_CHECKSIG
    if (isP2PKH(outputScript))
    {
        constexpr size_t kDataStart = 3;
        constexpr size_t kDataEnd = 23;
        constexpr uint8_t kMainnetP2pkhVersion = 0x00;

        std::vector<uint8_t> version = { kMainnetP2pkhVersion };
        std::vector<uint8_t> payload(outputScript.begin() + kDataStart, outputScript.begin() + kDataEnd);
        return base58Check(version, payload);
    }

    // P2SH: OP_HASH160 <20 bytes> OP_EQUAL
    if (isP2SH(outputScript))
    {
        constexpr size_t kDataStartPos = 2;
        constexpr size_t kDataEndPos = 22;
        constexpr uint8_t kMainnetP2shVersion = 0x05;

        std::vector<uint8_t> version = { kMainnetP2shVersion };
        std::vector<uint8_t> payload(outputScript.begin() + kDataStartPos, outputScript.begin() + kDataEndPos);
        return base58Check(version, payload);
    }

    // P2WPKH: OP_0 <20 bytes>
    const bool kIsP2wpkhScript = isP2WPKH(outputScript);

    // P2WSH: OP_0 <32 bytes>
    const bool kIsP2wshScript = isP2WSH(outputScript);

    // Taproot: OP_1 <32 bytes>
    const bool kIsTaproot = isTaproot(outputScript);

    if (kIsP2wpkhScript || kIsP2wshScript || kIsTaproot)
    {
        constexpr size_t kDataStartPos = 2;
        const size_t kWitVersion = kIsTaproot ? 1 : 0;
        const std::string kMainnetPrefix = "bc";

        std::vector<uint8_t> witProgram(outputScript.begin() + kDataStartPos, outputScript.end());
        return bech32Encode(kMainnetPrefix, kWitVersion, witProgram);
    }

    return "Unknown";
}

//================================================================================
// Method: decodeScriptSigP2PKH
// Description: Decodes a P2PKH input script and extracts the public key. Then 
//              computes the SHA-256 hash of the public key, followed by a 
//              RIPEMD-160 hash, to derive the public key hash. Finally, it 
//              prepends the P2PKH version byte and returns Base58Check-encoded 
//              address. Returns "Unknown" if the script is malformed or too short
//              to contain a valid public key.
//================================================================================
std::string TransactionScriptParser::decodeScriptSigP2PKH(IN std::span<const uint8_t> scriptSig)
{
    constexpr uint8_t kMainnetP2pkhVersion = 0x00;

    size_t pubkeyStart = scriptSig[0] + 1;
    if (scriptSig.size() < pubkeyStart + 1)
        return "Unknown";

    size_t pubkeyEnd = pubkeyStart + scriptSig[pubkeyStart] + 1;
    if (scriptSig.size() < pubkeyEnd)
        return "Unknown";

    std::vector<uint8_t> pubkey(
        scriptSig.begin() + pubkeyStart + 1,
        scriptSig.begin() + pubkeyEnd
    );

    uint8_t sha256Hash[SHA256_DIGEST_LENGTH];
    SHA256(pubkey.data(), pubkey.size(), sha256Hash);

    uint8_t ripemd160Hash[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160T(sha256Hash, SHA256_DIGEST_LENGTH, ripemd160Hash);

    std::vector<uint8_t> version = { kMainnetP2pkhVersion };
    std::vector<uint8_t> payload(ripemd160Hash, ripemd160Hash + RIPEMD160_DIGEST_LENGTH);
    return base58Check(version, payload);
}

//================================================================================
// Method: base58Check
// Description: Encodes a version-prefixed payload into a Base58Check Bitcoin 
//              address. Appends a checksum (first 4 bytes of double SHA256) to 
//              the input, then encodes using the Base58 alphabet. Preserves 
//              leading zeros as '1' characters in the result string.
//================================================================================
std::string TransactionScriptParser::base58Check(
    IN std::span<const uint8_t> version,
    IN std::span<const uint8_t> payload
)
{
    constexpr size_t kByteBase = 256;
    constexpr size_t kResultBase = 58;
    constexpr size_t kChecksumBytes = 4;
    constexpr const char* kBase58Alphabet = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

    // Combine version prefix and payload
    std::vector<uint8_t> fullPayload;
    const size_t kFullPayloadSize = version.size() + payload.size() + kChecksumBytes;
    fullPayload.reserve(kFullPayloadSize);

    fullPayload.insert(fullPayload.end(), version.begin(), version.end());
    fullPayload.insert(fullPayload.end(), payload.begin(), payload.end());

    // Count leading zero bytes to preserve them as '1' in Base58
    size_t leadingZeros = 0;
    for (const uint8_t byte : fullPayload)
    {
        if (byte != 0)
            break;

        ++leadingZeros;
    }

    // Double SHA256 for checksum
    std::array<uint8_t, 32> checksum = getDoubleSHA256(fullPayload);

    // Add checksum (first 4 bytes) to fullPayload
    fullPayload.insert(fullPayload.end(), checksum.begin(), checksum.begin() + kChecksumBytes);

    // Perform Base58 conversion by repeated division
    std::vector<uint8_t> digits;
    for (const uint8_t byte : fullPayload)
    {
        int carry = byte;
        for (size_t i = 0; i < digits.size(); ++i)
        {
            carry += kByteBase * digits[i];
            digits[i] = carry % kResultBase;
            carry /= kResultBase;
        }
        while (carry > 0)
        {
            digits.push_back(carry % kResultBase);
            carry /= kResultBase;
        }
    }

    std::string result = std::string(leadingZeros, '1');
    for (auto it = digits.rbegin(); it != digits.rend(); ++it)
        result += kBase58Alphabet[*it];

    return result;
}

//================================================================================
// Method: bech32Encode
// Description: Encodes witness program data into a Bech32 Bitcoin address using
//              the provided human-readable prefix, witness version, and witness 
//              program. Applies checksum generation and appends it to the result. 
//              Returns the full Bech32 string.
//================================================================================
std::string TransactionScriptParser::bech32Encode(
    IN const std::string& prefix,
    IN const int witVersion,
    IN std::span<const uint8_t> witProgram
)
{
    constexpr const char* kBech32Charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
    constexpr int kFromBits = 8;
    constexpr int kToBits = 5;

    constexpr size_t kBench32Xor = 1;
    constexpr size_t kBench32mXor = 0x2bc830a3;
    const size_t kChecksumXorValue = witVersion == 0 ? kBench32Xor : kBench32mXor;

    // Start data with witness version
    std::vector<uint32_t> data = { static_cast<uint32_t>(witVersion) };

    // Convert witness program bytes to 5-bit groups
    std::vector<uint32_t> converted = convertBits(witProgram, kFromBits, kToBits);
    data.insert(data.end(), converted.begin(), converted.end());

    // Compute and append checksum
    std::vector<uint32_t> checksum = createBech32Checksum(prefix, data, kChecksumXorValue);
    data.insert(data.end(), checksum.begin(), checksum.end());

    // Encode data
    std::string result = prefix + "1";
    for (uint32_t d : data)
        result += kBech32Charset[d];

    return result;
}

//================================================================================
// Method: convertBits
// Description: Converts a stream of input data from one bit resolution to another,
//              commonly from 8-bit bytes to 5-bit groups for Bech32 encoding. Can
//              optionally pad the result if leftover bits remain. Returns container
//              of converted values.
//================================================================================
std::vector<uint32_t> TransactionScriptParser::convertBits(
    IN std::span<const uint8_t> data,
    IN const int fromBits,
    IN const int toBits,
    IN const bool pad
)
{
    // accumulator for bit grouping
    uint32_t accumulator = 0;

    // mask for target bit size
    uint32_t targetMask = (1 << toBits) - 1;

    // number of bits currently in accumulator
    int currentBits = 0;

    std::vector<uint32_t> result;
    for (const uint8_t value : data)
    {
        accumulator = (accumulator << fromBits) | value;
        currentBits += fromBits;
        while (currentBits >= toBits)
        {
            currentBits -= toBits;
            result.push_back((accumulator >> currentBits) & targetMask);
        }
    }

    // If padding is enabled, flush remaining bits by left-shifting
    if (pad && currentBits)
        result.push_back((accumulator << (toBits - currentBits)) & targetMask);

    return result;
}

//================================================================================
// Method: hrpExpand
// Description: Expands the human-readable prefix (HRP) for Bech32 encoding by
//              splitting each character into high and low bits and adding a zero
//              separator. Returns the expanded HRP values as integers.
//================================================================================
std::vector<uint32_t> TransactionScriptParser::hrpExpand(IN const std::string& prefix)
{
    constexpr size_t kHighBitsShift = 5;
    constexpr size_t kLowBitsMask = 31;

    std::vector<uint32_t> result;

    // Add high bits of each character (top 3 bits)
    for (const unsigned char c : prefix)
        result.push_back(c >> kHighBitsShift);

    // Separator (zero value between high and low parts)
    result.push_back(0);

    // Add low bits of each character (lower 5 bits)
    for (const unsigned char c : prefix)
        result.push_back(c & kLowBitsMask);

    return result;
}

//================================================================================
// Method: bech32Polymod
// Description: Core Bech32 checksum function that computes a 30-bit cyclic
//              redundancy checksum over input values. Uses generator constants
//              defined by the Bech32 specification. Returns the polymod result
//              used in checksum calculation.
//================================================================================
uint32_t TransactionScriptParser::bech32Polymod(IN std::span<const uint32_t> values)
{
    constexpr std::array<uint32_t, 5> bech32Gen = { 0x3b6a57b2, 0x26508e6d, 0x1ea119fa, 0x3d4233dd, 0x2a1462b3 };

    constexpr size_t kTopBitsShift = 25;
    constexpr size_t kChecksumMask25Bits = 0x1ffffff;
    constexpr size_t kDigitBits = 5;

    // initial checksum value
    uint32_t checksum = 1;

    for (const uint32_t value : values)
    {
        // extract top 5 bits
        uint32_t b = checksum >> kTopBitsShift;

        checksum = ((checksum & kChecksumMask25Bits) << kDigitBits) ^ value;
        for (int i = 0; i < bech32Gen.size(); ++i)
            checksum ^= ((b >> i) & 1) ? bech32Gen[i] : 0; // apply generator if bit set
    }
    return checksum;
}

//================================================================================
// Method: createBech32Checksum
// Description: Produces a 6-character checksum for a Bech32 string. Expands the
//              human-readable prefix, appends data, and computes the checksum 
//              using the polymod algorithm. Returns the checksum values to be 
//              appended to the Bech32 address.
//================================================================================
std::vector<uint32_t> TransactionScriptParser::createBech32Checksum(
    IN const std::string& prefix,
    IN std::span<const uint32_t> data,
    IN const size_t xorValue
)
{
    constexpr size_t kChecksumLength = 6;
    constexpr size_t kBitsPerValue = 5;
    constexpr size_t kValueMask = 31;

    // Expand HRP and append input data
    std::vector<uint32_t> values = hrpExpand(prefix);
    values.insert(values.end(), data.begin(), data.end());

    // Add six zero placeholders for checksum
    values.insert(values.end(), kChecksumLength, 0);

    // Compute polymod and flip final bits with XOR
    uint32_t polymod = bech32Polymod(values) ^ xorValue;

    // Extract 6 groups of 5 bits as checksum values
    std::vector<uint32_t> checksum(kChecksumLength);
    for (int i = 0; i < kChecksumLength; ++i)
    {
        const int kShift = kBitsPerValue * (kChecksumLength - i - 1);
        checksum[i] = (polymod >> kShift) & kValueMask;
    }

    return checksum;
}
