#ifndef __ALPHA_MACRO_H__
#define __ALPHA_MACRO_H__

#include <string.h>
#include <assert.h>
#include "log.h"
#include "util.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define ALPHA_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define ALPHA_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define ALPHA_LIKELY(x)      (x)
#   define ALPHA_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define ALPHA_ASSERT(x) \
    if(ALPHA_UNLIKELY(!(x))) { \
        ALPHA_LOG_ERROR(ALPHA_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << alphaMin::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

/// 断言宏封装
#define ALPHA_ASSERT2(x, w) \
    if(ALPHA_UNLIKELY(!(x))) { \
        ALPHA_LOG_ERROR(ALPHA_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << alphaMin::BacktraceToString(100, 2, "    "); \
        assert(x); \
    }

#endif