#pragma once

//================================================================================
// Function: RIPEMD160T
// Description: External function wrapper for computing RIPEMD-160 hash using raw 
//              input buffer. Returns pointer to the output hash bytes.
//================================================================================
uint8_t* RIPEMD160T(const uint8_t* data, const size_t len, uint8_t* out);
