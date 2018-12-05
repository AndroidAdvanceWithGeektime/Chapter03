#ifndef ZZ_CONFIG_H_
#define ZZ_CONFIG_H_

#if defined(_M_X64) || defined(__x86_64__)
#define V8_TARGET_ARCH_X64 1
#elif defined(_M_IX86) || defined(__i386__)
#define V8_TARGET_ARCH_IA32 1
#elif defined(__AARCH64EL__)
#define V8_TARGET_ARCH_ARM64 1
#elif defined(__ARMEL__)
#define V8_TARGET_ARCH_ARM 1
#elif defined(__mips64)
#define V8_TARGET_ARCH_MIPS64 1
#elif defined(__MIPSEB__) || defined(__MIPSEL__)
#define V8_TARGET_ARCH_MIPS 1
#elif defined(_ARCH_PPC)
#define V8_TARGET_ARCH_PPC 1
#else
#error Target architecture was not detected as supported by v8
#endif

#endif