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

#define RETURN_IF_UNEXPECTED(expr) \
    if(!expr.has_value()) \
        return std::unexpected(expr.error());

#define RETURN_ERROR_CODE_IF_UNEXPECTED(expr) \
    if(!expr.has_value()) \
        return expr.error();

#define RETURN_IF_FAILED(expr) \
    if(expr != AppErrorCode::Success) \
        return expr;
