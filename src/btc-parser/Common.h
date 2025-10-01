#pragma once

#ifndef IN
#define IN
#endif // !IN

#ifndef OUT
#define OUT
#endif // !OUT

#ifndef INOUT
#define INOUT
#endif // !INOUT

enum class ReturnTypes
{
    SUCCESS = 0, // Operation completed successfully
    COMMAND_ARGUMENTS_ERROR = -1, // Invalid or missing command-line arguments
    BLOCK_PARSING_FAILURE = -2, // Failed to parse Bitcoin block or related data
    RUNTIME_EXCEPTION = -3, // Unexpected runtime error occurred
    EXCEPTION = -4, // Generic exception not covered by other codes
};
