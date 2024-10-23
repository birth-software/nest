#pragma once 

#ifdef NDEBUG
#define NEST_DEBUG 0
#else
#define NEST_DEBUG 1
#endif

#ifdef STATIC
#define LINK_LIBC 0
#else 
#define LINK_LIBC 1
#endif

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef __uint128_t u128;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef __int128_t s128;

typedef size_t usize;

typedef float f32;
typedef double f64;

typedef u32 Hash32;
typedef u64 Hash64;

#ifdef __cplusplus
#define EXPORT extern "C"
#else
#define EXPORT
#endif

#if defined(__cplusplus) && defined(__linux__)
#define NO_EXCEPT __THROW
#else
#define NO_EXCEPT
#endif

#define STRUCT_FORWARD_DECL(S) typedef struct S S
#define STRUCT(S) STRUCT_FORWARD_DECL(S); struct S
#define UNION_FORWARD_DECL(U) typedef union U U
#define UNION(U) UNION_FORWARD_DECL(U); union U

#define Slice(T) Slice_ ## T
#define SliceP(T) SliceP_ ## T
#define declare_slice_ex(T, StructName) STRUCT(StructName) \
{\
    T* pointer;\
    u64 length;\
}

#define declare_slice(T) declare_slice_ex(T, Slice(T))
#define declare_slice_p(T) declare_slice_ex(T*, SliceP(T))

declare_slice(u8);
declare_slice(u16);
declare_slice(u32);
declare_slice(u64);
declare_slice(s8);
declare_slice(s16);
declare_slice(s32);
declare_slice(s64);

declare_slice_p(char);

typedef Slice(u8) String;
declare_slice(String);

#define NamedEnumMemberEnum(e, enum_member) e ## _ ## enum_member
#define NamedEnumMemberString(e, enum_member) strlit(#enum_member)

typedef SliceP(char) CStringSlice;

#ifdef _WIN32
typedef void* FileDescriptor;
#else
typedef int FileDescriptor;
#endif

#define FOR_N(it, start, end) \
for (u32 it = (start), end__ = (end); it < end__; ++it)

#define FOR_REV_N(it, start, end) \
for (u32 it = (end), start__ = (start); (it--) > start__;)

#define FOR_BIT(it, start, bits) \
for (typeof(bits) _bits_ = (bits), it = (start); _bits_; _bits_ >>= 1, ++it) if (_bits_ & 1)

#define FOREACH_SET(it, set) \
FOR_N(_i, 0, ((set)->arr.capacity + 63) / 64) FOR_BIT(it, _i*64, (set)->arr.pointer[_i])

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")
#define fn static
#define method __attribute__((visibility("internal")))
#define global_variable static
#define forceinline __attribute__((always_inline))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define breakpoint() __builtin_debugtrap()
#define failed_execution() trap()

EXPORT void print(const char* format, ...);

#define trap() bad_exit("Trap reached", __FILE__, __LINE__)
#define array_length(arr) sizeof(arr) / sizeof((arr)[0])
#define KB(n) ((n) * 1024)
#define MB(n) ((n) * 1024 * 1024)
#define GB(n) ((u64)(n) * 1024 * 1024 * 1024)
#define TB(n) ((u64)(n) * 1024 * 1024 * 1024 * 1024)
#define unused(x) (void)(x)
#define may_be_unused __attribute__((unused))
#define truncate_value(Destination, source) (Destination)(source)
#define cast_to(Destination, Source, source) cast_ ## Source ## _to_ ## Destination (source, __FILE__, __LINE__)
#define bad_exit(message, file, line) do { print(message " at {cstr}:{u32}\n", file, line); __builtin_trap(); } while(0)
#define size_until_end(T, field_name) (sizeof(T) - offsetof(T, field_name))
#define SWAP(a, b) \
    do {\
        auto temp = a;\
        a = b;\
        b = temp;\
    } while (0)

#define slice_from_pointer_range(T, start, end) (Slice(T)) { .pointer = start, .length = (u64)(end - start), }

#define strlit_len(s) (sizeof(s) - 1)
#define strlit(s) (String){ .pointer = (u8*)(s), .length = strlit_len(s), }
#define ch_to_str(ch) (String){ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = (arr), .length = array_length(arr) }
#define array_to_bytes(arr) { .pointer = (u8*)(arr), .length = sizeof(arr) }
#define pointer_to_bytes(p) (String) { .pointer = (u8*)(p), .length = sizeof(*p) }
#define scalar_to_bytes(s) pointer_to_bytes(&(s))
#define string_to_c(s) ((char*)((s).pointer))
#define cstr(s) ((String) { .pointer = (u8*)(s), .length = strlen((char*)s), } )

#define case_to_name(prefix, e) case prefix ## e: return strlit(#e)

const may_be_unused global_variable u8 brace_open = '{';
const may_be_unused global_variable u8 brace_close = '}';

const may_be_unused global_variable u8 parenthesis_open = '(';
const may_be_unused global_variable u8 parenthesis_close = ')';

const may_be_unused global_variable u8 bracket_open = '[';
const may_be_unused global_variable u8 bracket_close = ']';

#define s_get(s, i) (s).pointer[i]
#define s_get_pointer(s, i) &((s).pointer[i])
#define s_get_slice(T, s, start, end) (Slice(T)){ .pointer = ((s).pointer) + (start), .length = (end) - (start) }
#define s_equal(a, b) ((a).length == (b).length && memcmp((a).pointer, (b).pointer, sizeof(*((a).pointer)) * (a).length) == 0)

#if NEST_DEBUG
#define assert(x) if (unlikely(!(x))) { bad_exit("Assert failed: \"" # x "\"", __FILE__, __LINE__); }
#else
#define assert(x) unlikely(!(x))
#endif

#ifndef __cplusplus
// Undefine unreachable if needed to provide a more safe-guard implementation
#ifdef unreachable
#undef unreachable
#endif
#if NEST_DEBUG
#define unreachable() bad_exit("Unreachable triggered", __FILE__, __LINE__)
#else
#define unreachable() __builtin_unreachable()
#endif

#define static_assert(x) _Static_assert((x), "Static assert failed!")
#define alignof(x) _Alignof(x)
#define auto __auto_type
#else
#define restrict __restrict
#endif

#define todo() do { print("TODO at {cstr}:{u32}\n", __FILE__, __LINE__); __builtin_trap(); } while(0)

u64 align_forward(u64 value, u64 alignment);
u64 align_backward(u64 value, u64 alignment);
u8 log2_alignment(u64 alignment);
u8 is_power_of_two(u64 value);
u8 first_bit_set_32(u32 value);
u64 first_bit_set_64(u64 value);

EXPORT void* memcpy(void* const restrict dst, const void* const restrict src, usize size) NO_EXCEPT;
EXPORT void* memmove(void* const dst, const void* const src, usize n) NO_EXCEPT;
EXPORT void* memset(void* dst, int n, usize size) NO_EXCEPT;
EXPORT int memcmp(const void* a, const void* b, usize n) NO_EXCEPT;
EXPORT usize strlen (const char* c_string) NO_EXCEPT;
EXPORT int strcmp(const char* s1, const char* s2) NO_EXCEPT;
EXPORT int strncmp(const char* s1, const char* s2, usize length) NO_EXCEPT;

u8 cast_u32_to_u8(u32 source, const char* name, int line);
u16 cast_u32_to_u16(u32 source, const char* name, int line);
s16 cast_u32_to_s16(u32 source, const char* name, int line);
s32 cast_u32_to_s32(u32 source, const char* name, int line);
u8 cast_u64_to_u8(u64 source, const char* name, int line);
u16 cast_u64_to_u16(u64 source, const char* name, int line);
u32 cast_u64_to_u32(u64 source, const char* name, int line);
s32 cast_u64_to_s32(u64 source, const char* name, int line);
s64 cast_u64_to_s64(u64 source, const char* name, int line);
u8 cast_s32_to_u8(s32 source, const char* name, int line);
u16 cast_s32_to_u16(s32 source, const char* name, int line);
u32 cast_s32_to_u32(s32 source, const char* name, int line);
u64 cast_s32_to_u64(s32 source, const char* name, int line);
s16 cast_s32_to_s16(s32 source, const char* name, int line);
u16 cast_s64_to_u16(s64 source, const char* name, int line);
u32 cast_s64_to_u32(s64 source, const char* name, int line);
u64 cast_s64_to_u64(s64 source, const char* name, int line);
s32 cast_s64_to_s32(s64 source, const char* name, int line);

u32 format_decimal(String buffer, u64 decimal);
u32 format_hexadecimal(String buffer, u64 hexadecimal);
u64 format_float(String buffer, f64 value_double);

u64 is_decimal_digit(u8 ch);
u32 is_space(u8 ch, u8 next_ch);
u8 get_next_ch_safe(String string, u64 index);
u64 is_identifier_start(u8 ch);
u64 is_identifier_ch(u8 ch);
u64 is_alphabetic(u8 ch);

u64 parse_decimal(String string);

global_variable const Hash64 fnv_offset = 14695981039346656037ull;
global_variable const u64 fnv_prime = 1099511628211ull;

Hash32 hash32_fib_end(Hash32 hash);
Hash32 hash64_fib_end(Hash64 hash);

Hash64 hash_byte(Hash64 source, u8 ch);
Hash64 hash_bytes(String bytes);
Hash32 hash64_to_hash32(Hash64 hash64);

u64 round_up_to_next_power_of_2(u64 n);
