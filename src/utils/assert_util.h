#ifndef __SYLAR_UTILS_ASSERT_H__
#define __SYLAR_UTILS_ASSERT_H__

#include "log.h"
#include <assert.h>

#if defined __GNUC__ || defined __llvm__
#define SYLAR_LIKELY(x)      __builtin_expect(!!(x), 1)
#define SYLAR_UNLIKELY(x)    __builtin_expect(!!(x), 0)
#else
#define SYLAR_LIKELY(x)      (x)
#define SYLAR_UNLIKELY(x)    (x)
#endif

#define SYLAR_ASSERT(x) \
    do { \
        if (SYLAR_UNLIKELY(!(x))) { \
            SYLAR_STREAM_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
                << "\nbacktrace:\n" \
                << sylar::utils::BacktraceToString(100, 2); \
            assert(x); \
        } \
    } while (0)

namespace sylar {
namespace utils {

std::string BacktraceToString(int size = 100, int skip = 0);

}
}

#endif