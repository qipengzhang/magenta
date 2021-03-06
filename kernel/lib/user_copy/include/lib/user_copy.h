// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <arch/user_copy.h>
#include <magenta/compiler.h>
#include <err.h>
#include <mxtl/user_ptr.h>

__BEGIN_CDECLS

inline status_t copy_to_user_unsafe(void* dst, const void* src, size_t len) {
  return arch_copy_to_user(dst, src, len);
}
inline status_t copy_from_user_unsafe(void* dst, const void* src, size_t len) {
  return arch_copy_from_user(dst, src, len);
}

// Convenience functions for common data types.
#define MAKE_COPY_TO_USER_UNSAFE(name, type) \
    static inline status_t name ## _unsafe(type *dst, type value) { \
        return copy_to_user_unsafe(dst, &value, sizeof(type)); \
    }

MAKE_COPY_TO_USER_UNSAFE(copy_to_user_u8, uint8_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_u16, uint16_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_32, int32_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_u32, uint32_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_u64, uint64_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_uptr, uintptr_t);
MAKE_COPY_TO_USER_UNSAFE(copy_to_user_iptr, intptr_t);

#undef MAKE_COPY_TO_USER_UNSAFE

#define MAKE_COPY_FROM_USER_UNSAFE(name, type) \
    static inline status_t name ## _unsafe(type *dst, const type *src) { \
        return copy_from_user_unsafe(dst, src, sizeof(type)); \
    }

MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_u8, uint8_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_u16, uint16_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_u32, uint32_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_32, int32_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_u64, uint64_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_uptr, uintptr_t);
MAKE_COPY_FROM_USER_UNSAFE(copy_from_user_iptr, intptr_t);

#undef MAKE_COPY_FROM_USER

__END_CDECLS

#if __cplusplus

template <typename T>
inline status_t copy_to_user(mxtl::user_ptr<T> dst, const void* src, size_t len) {
  return copy_to_user_unsafe(dst.get(), src, len);
}
template <typename T>
inline status_t copy_from_user(void* dst, const mxtl::user_ptr<T> src, size_t len) {
  return copy_from_user_unsafe(dst, src.get(), len);
}

// Convenience functions for common data types, this time with more C++.
#define MAKE_COPY_TO_USER(name, type) \
    static inline status_t name(mxtl::user_ptr<type> dst, type value) { \
        return copy_to_user(dst, &value, sizeof(type)); \
    }

MAKE_COPY_TO_USER(copy_to_user_u8, uint8_t);
MAKE_COPY_TO_USER(copy_to_user_u16, uint16_t);
MAKE_COPY_TO_USER(copy_to_user_32, int32_t);
MAKE_COPY_TO_USER(copy_to_user_u32, uint32_t);
MAKE_COPY_TO_USER(copy_to_user_u64, uint64_t);
MAKE_COPY_TO_USER(copy_to_user_uptr, uintptr_t);
MAKE_COPY_TO_USER(copy_to_user_iptr, intptr_t);

#undef MAKE_COPY_TO_USER

#define MAKE_COPY_FROM_USER(name, type) \
    static inline status_t name(type *dst, const mxtl::user_ptr<const type> src) { \
        return copy_from_user(dst, src, sizeof(type)); \
    } \
    static inline status_t name(type *dst, const mxtl::user_ptr<type> src) { \
        return copy_from_user(dst, src, sizeof(type)); \
    }

MAKE_COPY_FROM_USER(copy_from_user_u8, uint8_t);
MAKE_COPY_FROM_USER(copy_from_user_u16, uint16_t);
MAKE_COPY_FROM_USER(copy_from_user_u32, uint32_t);
MAKE_COPY_FROM_USER(copy_from_user_32, int32_t);
MAKE_COPY_FROM_USER(copy_from_user_u64, uint64_t);
MAKE_COPY_FROM_USER(copy_from_user_uptr, uintptr_t);
MAKE_COPY_FROM_USER(copy_from_user_iptr, intptr_t);

#undef MAKE_COPY_FROM_USER

#endif  // __cplusplus
