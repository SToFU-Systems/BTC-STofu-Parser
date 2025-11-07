#include "pch.h"

#include "Ripemd160.h"
#include <bit>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>


uint8_t* RIPEMD160::compute(const uint8_t* data, const size_t len, uint8_t* out)
{
	if (!data || !out)
		return nullptr;

	RIPEMD160 self;

	std::string hex = self.message_digest(std::string(reinterpret_cast<const char*>(data), len));

	for (size_t i = 0; i < 20; ++i) {
		unsigned v = 0;
		std::stringstream ss;
		ss << std::hex << hex.substr(i * 2, 2);
		ss >> v;
		out[i] = static_cast<uint8_t>(v & 0xFF);
	}

	return out;
}


std::string RIPEMD160::message_digest(const std::string& message) 
{
	std::vector<int64_t> state = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };// для инициализации состояния хэш-функции

	std::vector<uint8_t> bytes = add_padding(message);// добавление паддинга к сообщению 
	for (uint64_t i = 0; i < bytes.size() / BLOCK_LENGTH; ++i) {
		std::vector<uint32_t> schedule(16, 0);
		for (uint32_t j = 0; j < BLOCK_LENGTH; ++j) {
			schedule[j / 4] |= (bytes[i + j]) << (j % 4 * 8);
		}

		int32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
		int32_t aa = state[0], bb = state[1], cc = state[2], dd = state[3], ee = state[4];
		int32_t t = 0, tt = 0;

		for (uint32_t j = 0; j < 80; ++j) {
			uint32_t jj = j / 16;
			t = std::rotl(a + ff(jj + 1, b, c, d) + schedule[RL[j]] + KL[jj], SL[j]) + e;
			tt = std::rotl(aa + ff(5 - jj, bb, cc, dd) + schedule[RR[j]] + KR[jj], SR[j]) + ee;

			a = e; e = d; d = std::rotl(static_cast<uint32_t>(c), 10); c = b; b = t;
			aa = ee; ee = dd; dd = std::rotl(static_cast<uint32_t>(cc), 10); cc = bb; bb = tt;
		}

		t = state[1] + c + dd;
		state[1] = state[2] + d + ee;
		state[2] = state[3] + e + aa;
		state[3] = state[4] + a + bb;
		state[4] = state[0] + b + cc;
		state[0] = t;
	}

	std::stringstream stream;
	for (uint32_t i = 0; i < state.size() * 4; ++i) {
		int8_t byte_value = static_cast<int8_t>(unsigned_right_shift(state[i / 4], i % 4 * 8));
		stream << std::setfill('0') << std::setw(2) << std::hex << (byte_value & 0xff);
	}
	return stream.str();
}

std::vector<uint8_t> RIPEMD160::add_padding(const std::string& message) {
	std::vector<uint8_t> bytes(message.begin(), message.end());
	bytes.emplace_back(static_cast<uint8_t>(0x80));

	uint32_t padding = BLOCK_LENGTH - (bytes.size() % BLOCK_LENGTH);
	if (padding < 8) {
		padding += BLOCK_LENGTH;
	}
	bytes.resize(bytes.size() + padding - 8, static_cast<uint8_t>(0x0));

	const uint64_t bit_length = 8 * message.length();
	for (uint32_t i = 0; i < 8; ++i) {
		bytes.emplace_back(static_cast<uint8_t>(bit_length >> (8 * i)));
	}
	return bytes;
}

uint32_t RIPEMD160::ff(const uint32_t& group, const uint32_t& x, const uint32_t& y, const uint32_t& z) {
	uint32_t result;
	switch (group) {
	case 1: result = x ^ y ^ z; break;
	case 2: result = (x & y) | (~x & z); break;
	case 3: result = (x | ~y) ^ z; break;
	case 4: result = (x & z) | (y & ~z); break;
	case 5: result = x ^ (y | ~z); break;
	default: throw std::invalid_argument("Unexpected argument: " + group);
	};
	return result;
}

int32_t RIPEMD160::unsigned_right_shift(const int32_t& base, const int32_t& shift) {
	if (shift < 0 || shift >= 32) {
		throw std::invalid_argument("Shift must be in range 0..31: " + shift);
	}
	if (base == 0) {
		return 0;
	}
	return (base > 0) ? base >> shift : static_cast<uint32_t>(base) >> shift;
}

unsigned char* RIPEMD160T(const uint8_t* data, const size_t len, uint8_t* out)
{
	return RIPEMD160::compute(data, len, out);
}
