#ifndef ZZ_LOGGING_H_
#define ZZ_LOGGING_H_

#include "vm_core/platform/platform.h"
#include <stdio.h>

#define zDLog zFatal

void zFatal(const char *file, int line, const char *format, ...);

#ifndef DLOG
#ifdef DEBUG
#define DLOG(fmt, ...) zz::OS::Print(fmt, __VA_ARGS__)
#else
#define DLOG(fmt, ...)
#endif

#endif

#ifdef DEBUG
#define FATAL(...) zFatal(__FILE__, __LINE__, __VA_ARGS__)
#else
#define FATAL(...) zFatal("", 0, __VA_ARGS__)
#endif
#define UNIMPLEMENTED() FATAL("%s\n", "unimplemented code")
#define UNREACHABLE() FATAL("%s\n", "unreachable code")

namespace zz {

// CHECK dies with a fatal error if condition is not true.  It is *not*
// controlled by DEBUG, so the check will be executed regardless of
// compilation mode.
//
// We make sure CHECK et al. always evaluates their arguments, as
// doing CHECK(FunctionWithSideEffect()) is a common idiom.
#define CHECK_WITH_MSG(condition, message)                                                                             \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      FATAL("Check failed: %s.", message);                                                                             \
    }                                                                                                                  \
  } while (0)
#define CHECK(condition) CHECK_WITH_MSG(condition, #condition)

#ifdef DEBUG

#define DCHECK_WITH_MSG(condition, message)                                                                            \
  do {                                                                                                                 \
    if (!(condition)) {                                                                                                \
      FATAL(__FILE__, __LINE__, message);                                                                              \
    }                                                                                                                  \
  } while (0)
#define DCHECK(condition) DCHECK_WITH_MSG(condition, #condition)

// Helper macro for binary operators.
// Don't use this macro directly in your code, use CHECK_EQ et al below.
#define CHECK_OP(name, op, lhs, rhs)                                                                                   \
  do {                                                                                                                 \
    if (!(lhs op rhs)) {                                                                                               \
      FATAL("Check failed: %s.", #lhs " " #op " " #rhs);                                                               \
    }                                                                                                                  \
  } while (0)

#define DCHECK_OP(name, op, lhs, rhs)                                                                                  \
  do {                                                                                                                 \
    if (!((lhs)op(rhs))) {                                                                                             \
      FATAL(__FILE__, __LINE__, "");                                                                                   \
    }                                                                                                                  \
  } while (0)

#else
// Make all CHECK functions discard their log strings to reduce code
// bloat for official release builds.

#define CHECK_OP(name, op, lhs, rhs)                                                                                   \
  do {                                                                                                                 \
    bool _cond = lhs op rhs;                                                                                           \
    CHECK_WITH_MSG(_cond, #lhs " " #op " " #rhs);                                                                      \
  } while (0)

#define DCHECK_WITH_MSG(condition, msg) void(0);

#endif

#define CHECK_EQ(lhs, rhs) CHECK_OP(EQ, ==, lhs, rhs)
#define CHECK_NE(lhs, rhs) CHECK_OP(NE, !=, lhs, rhs)
#define CHECK_LE(lhs, rhs) CHECK_OP(LE, <=, lhs, rhs)
#define CHECK_LT(lhs, rhs) CHECK_OP(LT, <, lhs, rhs)
#define CHECK_GE(lhs, rhs) CHECK_OP(GE, >=, lhs, rhs)
#define CHECK_GT(lhs, rhs) CHECK_OP(GT, >, lhs, rhs)
#define CHECK_NULL(val) CHECK((val) == nullptr)
#define CHECK_NOT_NULL(val) CHECK((val) != nullptr)

} // namespace zz

#ifdef DEBUG
#define DCHECK_EQ(lhs, rhs) DCHECK_OP(EQ, ==, lhs, rhs)
#define DCHECK_NE(lhs, rhs) DCHECK_OP(NE, !=, lhs, rhs)
#define DCHECK_GT(lhs, rhs) DCHECK_OP(GT, >, lhs, rhs)
#define DCHECK_GE(lhs, rhs) DCHECK_OP(GE, >=, lhs, rhs)
#define DCHECK_LT(lhs, rhs) DCHECK_OP(LT, <, lhs, rhs)
#define DCHECK_LE(lhs, rhs) DCHECK_OP(LE, <=, lhs, rhs)
#define DCHECK_NULL(val) DCHECK((val) == nullptr)
#define DCHECK_NOT_NULL(val) DCHECK((val) != nullptr)
#define DCHECK_IMPLIES(lhs, rhs) DCHECK_WITH_MSG(!(lhs) || (rhs), #lhs " implies " #rhs)
#else
#define DCHECK(condition) ((void)0)
#define DCHECK_EQ(v1, v2) ((void)0)
#define DCHECK_NE(v1, v2) ((void)0)
#define DCHECK_GT(v1, v2) ((void)0)
#define DCHECK_GE(v1, v2) ((void)0)
#define DCHECK_LT(v1, v2) ((void)0)
#define DCHECK_LE(v1, v2) ((void)0)
#define DCHECK_NULL(val) ((void)0)
#define DCHECK_NOT_NULL(val) ((void)0)
#define DCHECK_IMPLIES(v1, v2) ((void)0)
#endif

#endif
