#include "pch.h"

// Project headers
#include "Ripemd160.h"

//================================================================================
// Function: RIPEMD160::compute
// Description: Computes the RIPEMD160 hash of input data. Initializes an internal
//              instance, generates a hex digest via message_digest(), and writes
//              the binary result (20 bytes) into the provided output buffer.
//================================================================================
uint8_t* RIPEMD160::compute(const uint8_t* data, const size_t len, uint8_t* out)
{
	if (!data || !out)
		return nullptr;

	RIPEMD160 self;

	std::string hex = self.message_digest(std::string(reinterpret_cast<const char*>(data), len));

	for (size_t i = 0; i < 20; ++i)
	{
		unsigned v = 0;

		std::stringstream ss;
		ss << std::hex << hex.substr(i * 2, 2);
		ss >> v;

		out[i] = static_cast<uint8_t>(v & 0xFF);
	}

	return out;
}

//================================================================================
// Function: RIPEMD160::message_digest
// Description: Produces a RIPEMD160 hex digest from a string input. Performs
//              message padding, block scheduling, and compression rounds. Returns
//              a lowercase hexadecimal representation of the resulting hash.
//================================================================================
std::string RIPEMD160::message_digest(const std::string& message)
{
	std::vector<int64_t> state = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };

	std::vector<uint8_t> bytes = add_padding(message);
	for (uint64_t i = 0; i < bytes.size() / BLOCK_LENGTH; ++i)
	{
		std::vector<uint32_t> schedule(16, 0);
		for (uint32_t j = 0; j < BLOCK_LENGTH; ++j)
		{
			schedule[j / 4] |= (bytes[i + j]) << (j % 4 * 8);
		}

		int32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
		int32_t aa = state[0], bb = state[1], cc = state[2], dd = state[3], ee = state[4];
		int32_t t = 0, tt = 0;

		for (uint32_t j = 0; j < 80; ++j)
		{
			uint32_t jj = j / 16;
			t = std::rotl(a + ff(jj + 1, b, c, d) + schedule[RL[j]] + KL[jj], SL[j]) + e;
			tt = std::rotl(aa + ff(5 - jj, bb, cc, dd) + schedule[RR[j]] + KR[jj], SR[j]) + ee;

			a = e;
			e = d;
			d = std::rotl(static_cast<uint32_t>(c), 10);
			c = b;
			b = t;

			aa = ee;
			ee = dd;
			dd = std::rotl(static_cast<uint32_t>(cc), 10);
			cc = bb;
			bb = tt;
		}

		t = state[1] + c + dd;
		state[1] = state[2] + d + ee;
		state[2] = state[3] + e + aa;
		state[3] = state[4] + a + bb;
		state[4] = state[0] + b + cc;
		state[0] = t;
	}

	std::stringstream stream;
	for (uint32_t i = 0; i < state.size() * 4; ++i)
	{
		int8_t byte_value = static_cast<int8_t>(unsigned_right_shift(state[i / 4], i % 4 * 8));
		stream << std::setfill('0') << std::setw(2) << std::hex << (byte_value & 0xff);
	}

	return stream.str();
}

//================================================================================
// Function: RIPEMD160::add_padding
// Description: Adds RIPEMD160-specific padding to the message. Appends a '1' bit,
//              zeros until message length is congruent to 56 mod 64, then appends
//              the original message length in bits as a 64-bit little-endian value.
//================================================================================
std::vector<uint8_t> RIPEMD160::add_padding(const std::string& message)
{
	std::vector<uint8_t> bytes(message.begin(), message.end());
	bytes.emplace_back(static_cast<uint8_t>(0x80));

	uint32_t padding = BLOCK_LENGTH - (bytes.size() % BLOCK_LENGTH);

	if (padding < 8)
		padding += BLOCK_LENGTH;

	bytes.resize(bytes.size() + padding - 8, static_cast<uint8_t>(0x0));

	const uint64_t bit_length = 8 * message.length();
	for (uint32_t i = 0; i < 8; ++i)
	{
		bytes.emplace_back(static_cast<uint8_t>(bit_length >> (8 * i)));
	}

	return bytes;
}

//================================================================================
// Function: RIPEMD160::ff
// Description: Implements the five nonlinear functions used in each group of the
//              RIPEMD160 compression process. Returns a 32-bit transformed value.
//================================================================================
uint32_t RIPEMD160::ff(const uint32_t& group, const uint32_t& x, const uint32_t& y, const uint32_t& z)
{
	uint32_t result;
	switch (group)
	{
	case 1:
		result = x ^ y ^ z;
		break;
	case 2:
		result = (x & y) | (~x & z);
		break;
	case 3:
		result = (x | ~y) ^ z;
		break;
	case 4:
		result = (x & z) | (y & ~z);
		break;
	case 5:
		result = x ^ (y | ~z);
		break;
	default:
		throw std::invalid_argument("Unexpected argument: " + group);
	};

	return result;
}

//================================================================================
// Function: RIPEMD160::unsigned_right_shift
// Description: Performs an unsigned right shift on a 32-bit signed integer,
//              emulating Java-style behavior for logical right shifts.
//================================================================================
int32_t RIPEMD160::unsigned_right_shift(const int32_t& base, const int32_t& shift)
{
	if (shift < 0 || shift >= 32)
		throw std::invalid_argument("Shift must be in range 0..31: " + shift);

	if (base == 0)
		return 0;

	return (base > 0) ? base >> shift : static_cast<uint32_t>(base) >> shift;
}

//================================================================================
// Function: RIPEMD160T
// Description: C-style wrapper for RIPEMD160::compute, used for compatibility with
//              external code expecting a simple function call interface.
//================================================================================
uint8_t* RIPEMD160T(const uint8_t* data, const size_t len, uint8_t* out)
{
	return RIPEMD160::compute(data, len, out);
}
