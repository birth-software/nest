#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#endif

#ifdef NDEBUG
#define _DEBUG 0
#else
#define _DEBUG 1
#endif

#ifdef STATIC
#define LINK_LIBC 0
#else 
#define LINK_LIBC 1
#endif

#if LINK_LIBC
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
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

typedef float f32;
typedef double f64;

typedef u32 Hash32;
typedef u64 Hash64;

#ifdef _WIN32
typedef HANDLE FileDescriptor;
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

#define STRUCT_FORWARD_DECL(S) typedef struct S S
#define STRUCT(S) STRUCT_FORWARD_DECL(S); struct S
#define UNION_FORWARD_DECL(U) typedef union U U
#define UNION(U) UNION_FORWARD_DECL(U); union U
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")
#define fn static
#define method __attribute__((visibility("internal")))
#define global static
#define forceinline __attribute__((always_inline))
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#define breakpoint() __builtin_debugtrap()
#define fail() trap()
#define trap() bad_exit("Trap reached", __FILE__, __LINE__)
#define array_length(arr) sizeof(arr) / sizeof((arr)[0])
#define KB(n) ((n) * 1024)
#define MB(n) ((n) * 1024 * 1024)
#define GB(n) ((u64)(n) * 1024 * 1024 * 1024)
#define TB(n) ((u64)(n) * 1024 * 1024 * 1024 * 1024)
#define unused(x) (void)(x)
#define may_be_unused __attribute__((unused))
#define truncate(Destination, source) (Destination)(source)
#define cast(Destination, Source, source) cast_ ## Source ## _to_ ## Destination (source, __FILE__, __LINE__)
#define bad_exit(message, file, line) do { print(message " at {cstr}:{u32}\n", file, line); __builtin_trap(); } while(0)
#define size_until_end(T, field_name) (sizeof(T) - offsetof(T, field_name))
#define SWAP(a, b) \
    do {\
        auto temp = a;\
        a = b;\
        b = temp;\
    } while (0)

fn 
#if _WIN32
u64
#else
#if LINK_LIBC
struct timespec
#else
u64 
#endif
#endif
timestamp()
{
#ifdef _WIN32
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return (u64)li.QuadPart;
#else
#if LINK_LIBC
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
#else
#if defined(__x86_64__)
    return __rdtsc();
#else
    return 0;
#endif
#endif
#endif
}


#if LINK_LIBC
global struct timespec cpu_resolution;
#else
global u64 cpu_frequency;
#endif


may_be_unused fn void print(const char* format, ...);

may_be_unused fn u8 cast_u32_to_u8(u32 source, const char* name, int line)
{
#if _DEBUG
    if (source > UINT8_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u8)source;
    return result;
}

may_be_unused fn u16 cast_u32_to_u16(u32 source, const char* name, int line)
{
#if _DEBUG
    if (source > UINT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u16)source;
    return result;
}

may_be_unused fn s16 cast_u32_to_s16(u32 source, const char* name, int line)
{
#if _DEBUG
    if (source > INT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s16)source;
    return result;
}

may_be_unused fn s32 cast_u32_to_s32(u32 source, const char* name, int line)
{
#if _DEBUG
    if (source > INT32_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s32)source;
    return result;
}

may_be_unused fn u8 cast_u64_to_u8(u64 source, const char* name, int line)
{
#if _DEBUG
    if (source > UINT8_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u8)source;
    return result;
}

may_be_unused fn u16 cast_u64_to_u16(u64 source, const char* name, int line)
{
#if _DEBUG
    if (source > UINT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u16)source;
    return result;
}

may_be_unused fn u32 cast_u64_to_u32(u64 source, const char* name, int line)
{
#if _DEBUG
    if (source > UINT32_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u32)source;
    return result;
}

may_be_unused fn s32 cast_u64_to_s32(u64 source, const char* name, int line)
{
#if _DEBUG
    if (source > INT32_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s32)source;
    return result;
}

may_be_unused fn s64 cast_u64_to_s64(u64 source, const char* name, int line)
{
#if _DEBUG
    if (source > INT64_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s64)source;
    return result;
}

may_be_unused fn u8 cast_s32_to_u8(s32 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
    if ((u32)source > UINT8_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u8)source;
    return result;
}

may_be_unused fn u16 cast_s32_to_u16(s32 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
    if ((u32)source > UINT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u16)source;
    return result;
}

may_be_unused fn u32 cast_s32_to_u32(s32 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u32)source;
    return result;
}

may_be_unused fn u64 cast_s32_to_u64(s32 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u64)source;
    return result;
}

may_be_unused fn s16 cast_s32_to_s16(s32 source, const char* name, int line)
{
#if _DEBUG
    if (source > INT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
    if (source < INT16_MIN)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s16)source;
    return result;
}

may_be_unused fn u16 cast_s64_to_u16(s64 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
    if (source > UINT16_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u16)source;
    return result;
}

may_be_unused fn u32 cast_s64_to_u32(s64 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u32)source;
    return result;
}

may_be_unused fn u64 cast_s64_to_u64(s64 source, const char* name, int line)
{
#if _DEBUG
    if (source < 0)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (u64)source;
    return result;
}

may_be_unused fn s32 cast_s64_to_s32(s64 source, const char* name, int line)
{
#if _DEBUG
    if (source < INT32_MIN)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }

    if (source > INT32_MAX)
    {
        print("Cast failed at {cstr}:{u32}\n", name, line);
        trap();
    }
#else 
    unused(name);
    unused(line);
#endif
    auto result = (s32)source;
    return result;
}

#if _DEBUG
#define assert(x) if (unlikely(!(x))) { bad_exit("Assert failed: \"" # x "\"", __FILE__, __LINE__); }
#else
#define assert(x) unlikely(!(x))
#endif

#ifdef unreachable
#undef unreachable
#endif

#if _DEBUG
#define unreachable() bad_exit("Unreachable triggered", __FILE__, __LINE__)
#else
#define unreachable() __builtin_unreachable()
#endif

#ifdef static_assert
#undef static_assert
#endif
#define static_assert(x) _Static_assert((x), "Static assert failed!")
#define alignof(x) _Alignof(x)
#define auto __auto_type

#define todo() do { print("TODO at {cstr}:{u32}\n", __FILE__, __LINE__); __builtin_trap(); } while(0)

fn FileDescriptor stdout_get()
{
#if _WIN32
    auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
    assert(handle != INVALID_HANDLE_VALUE);
    return handle;
#else
    return 1;
#endif
}

#if __APPLE__
    const global u64 page_size = KB(16);
#else
    const global u64 page_size = KB(4);
#endif

typedef enum TimeUnit
{
    TIME_UNIT_NANOSECONDS,
    TIME_UNIT_MICROSECONDS,
    TIME_UNIT_MILLISECONDS,
    TIME_UNIT_SECONDS,
} TimeUnit;

may_be_unused fn f64 resolve_timestamp(
#if _WIN32
        u64 start, u64 end,
#else
#if LINK_LIBC
        struct timespec start, struct timespec end,
#else
        u64 start, u64 end,
#endif
#endif
        TimeUnit time_unit)
{
#if _WIN32
    unused(start);
    unused(end);
    unused(time_unit);
    todo();
#else
#if LINK_LIBC
    assert(end.tv_sec >= start.tv_sec);
    struct timespec result = {
        .tv_sec = end.tv_sec - start.tv_sec,
        .tv_nsec = end.tv_nsec - start.tv_nsec,
    };

    auto ns_in_a_sec = 1000000000;
    if (result.tv_nsec < 0)
    {
        result.tv_sec -= 1;
        result.tv_nsec += ns_in_a_sec;
    }

    auto ns = result.tv_sec * ns_in_a_sec + result.tv_nsec;
    switch (time_unit)
    {
    case TIME_UNIT_NANOSECONDS:
        return (f64)ns;
    case TIME_UNIT_MICROSECONDS:
        return (f64)ns / 1000.0;
    case TIME_UNIT_MILLISECONDS:
        return (f64)ns / 1000000.0;
    case TIME_UNIT_SECONDS:
        return (f64)ns / 1000000000.0;
    }
#else
    assert(end >= start);
    auto ticks = end - start;
    f64 s = (f64)(end - start);
    if (cpu_frequency)
    {
        s = s / (f64)cpu_frequency;

        switch (time_unit)
        {
            case TIME_UNIT_NANOSECONDS:
                return s / 1000000000.0;
            case TIME_UNIT_MICROSECONDS:
                return s / 1000000.0;
            case TIME_UNIT_MILLISECONDS:
                return s / 1000.0;
            case TIME_UNIT_SECONDS:
                return s;
        }
    }
    else
    {
        // warning: rdtsc frequency not queried (returning ticks as are)
        return s;
    }
#endif
#endif
}

const may_be_unused global u8 brace_open = '{';
const may_be_unused global u8 brace_close = '}';

const may_be_unused global u8 parenthesis_open = '(';
const may_be_unused global u8 parenthesis_close = ')';

const may_be_unused global u8 bracket_open = '[';
const may_be_unused global u8 bracket_close = ']';

may_be_unused fn u8 log2_alignment(u64 alignment)
{
    assert(alignment != 0);
    assert((alignment & (alignment - 1)) == 0);
    u64 left = (sizeof(alignment) * 8) - 1;
    auto right = cast(u64, s32, __builtin_clzll(alignment));
    auto result = cast(u8, u64, left - right);
    return result;
}

// Lehmer's generator
// https://lemire.me/blog/2019/03/19/the-fastest-conventional-random-number-generator-that-can-pass-big-crush/
may_be_unused global __uint128_t rn_state;
may_be_unused fn u64 generate_random_number()
{
    rn_state *= 0xda942042e4dd58b5;
    return rn_state >> 64;
}

may_be_unused fn u64 round_up_to_next_power_of_2(u64 n)
{
    n -= 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n += 1;
    return n;
}

may_be_unused fn u64 absolute_int(s64 n)
{
    return n < 0 ? cast(u64, s64, -n) : cast(u64, s64, n);
}

#if LINK_LIBC == 0
int strcmp(const char* s1, const char* s2)
{
    auto diff = 0;
    while (1)
    {
        auto ch1 = *s1;
        auto ch2 = *s2;
        diff = ch1 - ch2;
        if (ch1 == 0 || ch2 == 0 || diff)
        {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return diff;
}
void* memcpy(void* const restrict dst, const void* const restrict src, u64 size)
{
    auto* destination = (u8*)dst;
    auto* source = (u8*)src;

    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = source[i];
    }

    return dst;
}

void* memmove(void* const dst, const void* const src, u64 n)
{
    // Implementation
    // https://opensource.apple.com/source/network_cmds/network_cmds-481.20.1/unbound/compat/memmove.c.auto.html
    uint8_t* from = (uint8_t*) src;
    uint8_t* to = (uint8_t*) dst;

    if (from == to || n == 0)
    return dst;
    if (to > from && to-from < (s64)n) {
    /* to overlaps with from */
    /*  <from......>         */
    /*         <to........>  */
    /* copy in reverse, to avoid overwriting from */
    u64 i;
    for(i=n-1; i>=0; i--)
    to[i] = from[i];
    return dst;
    }
    if (from > to && from-to < (int)n) {
    /* to overlaps with from */
    /*        <from......>   */
    /*  <to........>         */
    /* copy forwards, to avoid overwriting from */
    u64 i;
    for(i=0; i<n; i++)
    to[i] = from[i];
    return dst;
    }
    memcpy(dst, src, n);
    return dst;
}

void* memset(void* dst, u8 n, u64 size)
{
    auto* destination = (u8*)dst;
    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = n;
    }

    return dst;
}

fn int memcmp(const void* a, const void* b, u64 n)
{
    auto *s1 = (u8*)a;
    auto *s2 = (u8*)b;

    while (n-- > 0)
    {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

fn u64 strlen (const char* c_string)
{
    auto* it = c_string;
    while (*it)
    {
        it += 1;
    }
    return (u64)(it - c_string);
}
#endif
#define slice_from_pointer_range(T, start, end) (Slice(T)) { .pointer = start, .length = (u64)(end - start), }

#define strlit_len(s) (sizeof(s) - 1)
#define strlit(s) (String){ .pointer = (u8*)(s), .length = strlit_len(s), }
#define ch_to_str(ch) (String){ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = (arr), .length = array_length(arr) }
#define array_to_bytes(arr) { .pointer = (u8*)(arr), .length = sizeof(arr) }
#define pointer_to_bytes(p) (String) { .pointer = (u8*)(p), .length = sizeof(*p) }
#define struct_to_bytes(s) pointer_to_bytes(&(s))
#define string_to_c(s) ((char*)((s).pointer))
#define cstr(s) ((String) { .pointer = (u8*)(s), .length = strlen(s), } )

#define case_to_name(prefix, e) case prefix ## e: return strlit(#e)

#define Slice(T) Slice_ ## T
#define SliceP(T) SliceP_ ## T
#define declare_slice_ex(T, StructName) STRUCT(StructName) \
{\
    T* pointer;\
    u64 length;\
}

#define declare_slice(T) declare_slice_ex(T, Slice(T))
#define declare_slice_p(T) declare_slice_ex(T*, SliceP(T))

#define s_get(s, i) (s).pointer[i]
#define s_get_pointer(s, i) &((s).pointer[i])
#define s_get_slice(T, s, start, end) (Slice(T)){ .pointer = ((s).pointer) + (start), .length = (end) - (start) }
#define s_equal(a, b) ((a).length == (b).length && memcmp((a).pointer, (b).pointer, sizeof(*((a).pointer)) * (a).length) == 0)

declare_slice(u8);
declare_slice(s32);
typedef Slice(u8) String;
// Array of strings
declare_slice(String);
declare_slice_p(char);
typedef SliceP_char CStringSlice;

// fn s32 string_first_ch(String string, u8 ch)
// {
//     s32 result = -1;
//     for (u64 i = 0; i < string.length; i += 1)
//     {
//         if (string.pointer[i] == ch)
//         {
//             result = i;
//             break;
//         }
//     }
//
//     return result;
// }

fn s64 string_last_ch(String string, u8 ch)
{
    s64 result = -1;
    u64 i = string.length;
    while (i > 0)
    {
        i -= 1;
        if (string.pointer[i] == ch)
        {
            result = cast(s64, u64, i);
            break;
        }
    }

    return result;
}

// fn String string_dir(String string)
// {
//     String result = {};
//     auto index = string_last_ch(string, '/');
//     if (index != -1)
//     {
//         result = s_get_slice(u8, string, 0, index);
//     }
//
//     return result;
// }

may_be_unused fn String string_base(String string)
{
    String result = {};
    auto maybe_index = string_last_ch(string, '/');
    if (maybe_index != -1)
    {
        auto index = cast(u64, s64, maybe_index);
        result = s_get_slice(u8, string, index + 1, string.length);
    }

    return result;
}

may_be_unused fn String string_no_extension(String string)
{
    String result = {};
    auto maybe_index = string_last_ch(string, '.');
    if (maybe_index != -1)
    {
        auto index = cast(u64, s64, maybe_index);
        result = s_get_slice(u8, string, 0, index);
    }

    return result;
}

fn u64 parse_decimal(String string)
{
    u64 value = 0;
    for (u64 i = 0; i < string.length; i += 1)
    {
        u8 ch = s_get(string, i);
        assert(((ch >= '0') & (ch <= '9')));
        value = (value * 10) + (ch - '0');
    }

    return value;
}

fn u64 safe_flag(u64 value, u64 flag)
{
    u64 result = value & ((u64)0 - flag);
    return result;
}

may_be_unused fn u8 get_next_ch_safe(String string, u64 index)
{
    u64 next_index = index + 1;
    u64 is_in_range = next_index < string.length;
    u64 safe_index = safe_flag(next_index, is_in_range);
    u8 unsafe_result = string.pointer[safe_index];
    u64 safe_result = safe_flag(unsafe_result, is_in_range);
    assert(safe_result < 256);
    return (u8)safe_result;
}

may_be_unused fn u32 is_space(u8 ch, u8 next_ch)
{
    u32 is_comment = (ch == '/') & (next_ch == '/');
    u32 is_whitespace = ch == ' ';
    u32 is_vertical_tab = ch == 0x0b;
    u32 is_horizontal_tab = ch == '\t';
    u32 is_line_feed = ch == '\n';
    u32 is_carry_return = ch == '\r';
    u32 result = (((is_vertical_tab | is_horizontal_tab) | (is_line_feed | is_carry_return)) | (is_comment | is_whitespace));
    return result;
}

fn u64 is_lower(u8 ch)
{
    return (ch >= 'a') & (ch <= 'z');
}

fn u64 is_upper(u8 ch)
{
    return (ch >= 'A') & (ch <= 'Z');
}

fn u64 is_alphabetic(u8 ch)
{
    return is_lower(ch) | is_upper(ch);
}

fn u64 is_decimal_digit(u8 ch)
{
    return (ch >= '0') & (ch <= '9');
}

// fn u64 is_hex_digit(u8 ch)
// {
//     return (is_decimal_digit(ch) | ((ch == 'a' | ch == 'A') | (ch == 'b' | ch == 'B'))) | (((ch == 'c' | ch == 'C') | (ch == 'd' | ch == 'D')) | ((ch == 'e' | ch == 'E') | (ch == 'f' | ch == 'F')));
// }


fn u64 is_identifier_start(u8 ch)
{
    u64 alphabetic = is_alphabetic(ch);
    u64 is_underscore = ch == '_';
    return alphabetic | is_underscore;
}

may_be_unused fn u64 is_identifier_ch(u8 ch)
{
    u64 identifier_start = is_identifier_start(ch);
    u64 decimal = is_decimal_digit(ch);
    return identifier_start | decimal;
}

global const Hash64 fnv_offset = 14695981039346656037ull;
global const u64 fnv_prime = 1099511628211ull;

fn Hash64 hash_byte(Hash64 source, u8 ch)
{
    source ^= ch;
    source *= fnv_prime;
    return source;
}

may_be_unused fn Hash64 hash_bytes(String bytes)
{
    u64 result = fnv_offset;
    for (u64 i = 0; i < bytes.length; i += 1)
    {
        result = hash_byte(result, bytes.pointer[i]);
    }

    return result;
}

may_be_unused fn Hash32 hash64_to_hash32(Hash64 hash64)
{
    Hash32 low = hash64 & 0xffff;
    Hash32 high = (hash64 >> 32) & 0xffff;
    Hash32 result = (high << 16) | low;
    return result;
}

#if LINK_LIBC == 0
#ifdef __linux__
may_be_unused fn forceinline long syscall0(long n)
{
    long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall1(long n, long a1)
{
    long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall2(long n, long a1, long a2)
{
    long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
    : "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall3(long n, long a1, long a2, long a3)
{
    long ret;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
    "d"(a3) : "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall4(long n, long a1, long a2, long a3, long a4)
{
    long ret;
    register long r10 __asm__("r10") = a4;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
    "d"(a3), "r"(r10): "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
    "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
    return ret;
}

may_be_unused fn forceinline long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
    long ret;
    register long r10 __asm__("r10") = a4;
    register long r8 __asm__("r8") = a5;
    register long r9 __asm__("r9") = a6;
    __asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
    "d"(a3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
    return ret;
}

enum SyscallX86_64 : u64 {
    syscall_x86_64_read = 0,
    syscall_x86_64_write = 1,
    syscall_x86_64_open = 2,
    syscall_x86_64_close = 3,
    syscall_x86_64_stat = 4,
    syscall_x86_64_fstat = 5,
    syscall_x86_64_lstat = 6,
    syscall_x86_64_poll = 7,
    syscall_x86_64_lseek = 8,
    syscall_x86_64_mmap = 9,
    syscall_x86_64_mprotect = 10,
    syscall_x86_64_munmap = 11,
    syscall_x86_64_brk = 12,
    syscall_x86_64_rt_sigaction = 13,
    syscall_x86_64_rt_sigprocmask = 14,
    syscall_x86_64_rt_sigreturn = 15,
    syscall_x86_64_ioctl = 16,
    syscall_x86_64_pread64 = 17,
    syscall_x86_64_pwrite64 = 18,
    syscall_x86_64_readv = 19,
    syscall_x86_64_writev = 20,
    syscall_x86_64_access = 21,
    syscall_x86_64_pipe = 22,
    syscall_x86_64_select = 23,
    syscall_x86_64_sched_yield = 24,
    syscall_x86_64_mremap = 25,
    syscall_x86_64_msync = 26,
    syscall_x86_64_mincore = 27,
    syscall_x86_64_madvise = 28,
    syscall_x86_64_shmget = 29,
    syscall_x86_64_shmat = 30,
    syscall_x86_64_shmctl = 31,
    syscall_x86_64_dup = 32,
    syscall_x86_64_dup2 = 33,
    syscall_x86_64_pause = 34,
    syscall_x86_64_nanosleep = 35,
    syscall_x86_64_getitimer = 36,
    syscall_x86_64_alarm = 37,
    syscall_x86_64_setitimer = 38,
    syscall_x86_64_getpid = 39,
    syscall_x86_64_sendfile = 40,
    syscall_x86_64_socket = 41,
    syscall_x86_64_connect = 42,
    syscall_x86_64_accept = 43,
    syscall_x86_64_sendto = 44,
    syscall_x86_64_recvfrom = 45,
    syscall_x86_64_sendmsg = 46,
    syscall_x86_64_recvmsg = 47,
    syscall_x86_64_shutdown = 48,
    syscall_x86_64_bind = 49,
    syscall_x86_64_listen = 50,
    syscall_x86_64_getsockname = 51,
    syscall_x86_64_getpeername = 52,
    syscall_x86_64_socketpair = 53,
    syscall_x86_64_setsockopt = 54,
    syscall_x86_64_getsockopt = 55,
    syscall_x86_64_clone = 56,
    syscall_x86_64_fork = 57,
    syscall_x86_64_vfork = 58,
    syscall_x86_64_execve = 59,
    syscall_x86_64_exit = 60,
    syscall_x86_64_wait4 = 61,
    syscall_x86_64_kill = 62,
    syscall_x86_64_uname = 63,
    syscall_x86_64_semget = 64,
    syscall_x86_64_semop = 65,
    syscall_x86_64_semctl = 66,
    syscall_x86_64_shmdt = 67,
    syscall_x86_64_msgget = 68,
    syscall_x86_64_msgsnd = 69,
    syscall_x86_64_msgrcv = 70,
    syscall_x86_64_msgctl = 71,
    syscall_x86_64_fcntl = 72,
    syscall_x86_64_flock = 73,
    syscall_x86_64_fsync = 74,
    syscall_x86_64_fdatasync = 75,
    syscall_x86_64_truncate = 76,
    syscall_x86_64_ftruncate = 77,
    syscall_x86_64_getdents = 78,
    syscall_x86_64_getcwd = 79,
    syscall_x86_64_chdir = 80,
    syscall_x86_64_fchdir = 81,
    syscall_x86_64_rename = 82,
    syscall_x86_64_mkdir = 83,
    syscall_x86_64_rmdir = 84,
    syscall_x86_64_creat = 85,
    syscall_x86_64_link = 86,
    syscall_x86_64_unlink = 87,
    syscall_x86_64_symlink = 88,
    syscall_x86_64_readlink = 89,
    syscall_x86_64_chmod = 90,
    syscall_x86_64_fchmod = 91,
    syscall_x86_64_chown = 92,
    syscall_x86_64_fchown = 93,
    syscall_x86_64_lchown = 94,
    syscall_x86_64_umask = 95,
    syscall_x86_64_gettimeofday = 96,
    syscall_x86_64_getrlimit = 97,
    syscall_x86_64_getrusage = 98,
    syscall_x86_64_sysinfo = 99,
    syscall_x86_64_times = 100,
    syscall_x86_64_ptrace = 101,
    syscall_x86_64_getuid = 102,
    syscall_x86_64_syslog = 103,
    syscall_x86_64_getgid = 104,
    syscall_x86_64_setuid = 105,
    syscall_x86_64_setgid = 106,
    syscall_x86_64_geteuid = 107,
    syscall_x86_64_getegid = 108,
    syscall_x86_64_setpgid = 109,
    syscall_x86_64_getppid = 110,
    syscall_x86_64_getpgrp = 111,
    syscall_x86_64_setsid = 112,
    syscall_x86_64_setreuid = 113,
    syscall_x86_64_setregid = 114,
    syscall_x86_64_getgroups = 115,
    syscall_x86_64_setgroups = 116,
    syscall_x86_64_setresuid = 117,
    syscall_x86_64_getresuid = 118,
    syscall_x86_64_setresgid = 119,
    syscall_x86_64_getresgid = 120,
    syscall_x86_64_getpgid = 121,
    syscall_x86_64_setfsuid = 122,
    syscall_x86_64_setfsgid = 123,
    syscall_x86_64_getsid = 124,
    syscall_x86_64_capget = 125,
    syscall_x86_64_capset = 126,
    syscall_x86_64_rt_sigpending = 127,
    syscall_x86_64_rt_sigtimedwait = 128,
    syscall_x86_64_rt_sigqueueinfo = 129,
    syscall_x86_64_rt_sigsuspend = 130,
    syscall_x86_64_sigaltstack = 131,
    syscall_x86_64_utime = 132,
    syscall_x86_64_mknod = 133,
    syscall_x86_64_uselib = 134,
    syscall_x86_64_personality = 135,
    syscall_x86_64_ustat = 136,
    syscall_x86_64_statfs = 137,
    syscall_x86_64_fstatfs = 138,
    syscall_x86_64_sysfs = 139,
    syscall_x86_64_getpriority = 140,
    syscall_x86_64_setpriority = 141,
    syscall_x86_64_sched_setparam = 142,
    syscall_x86_64_sched_getparam = 143,
    syscall_x86_64_sched_setscheduler = 144,
    syscall_x86_64_sched_getscheduler = 145,
    syscall_x86_64_sched_get_priority_max = 146,
    syscall_x86_64_sched_get_priority_min = 147,
    syscall_x86_64_sched_rr_get_interval = 148,
    syscall_x86_64_mlock = 149,
    syscall_x86_64_munlock = 150,
    syscall_x86_64_mlockall = 151,
    syscall_x86_64_munlockall = 152,
    syscall_x86_64_vhangup = 153,
    syscall_x86_64_modify_ldt = 154,
    syscall_x86_64_pivot_root = 155,
    syscall_x86_64__sysctl = 156,
    syscall_x86_64_prctl = 157,
    syscall_x86_64_arch_prctl = 158,
    syscall_x86_64_adjtimex = 159,
    syscall_x86_64_setrlimit = 160,
    syscall_x86_64_chroot = 161,
    syscall_x86_64_sync = 162,
    syscall_x86_64_acct = 163,
    syscall_x86_64_settimeofday = 164,
    syscall_x86_64_mount = 165,
    syscall_x86_64_umount2 = 166,
    syscall_x86_64_swapon = 167,
    syscall_x86_64_swapoff = 168,
    syscall_x86_64_reboot = 169,
    syscall_x86_64_sethostname = 170,
    syscall_x86_64_setdomainname = 171,
    syscall_x86_64_iopl = 172,
    syscall_x86_64_ioperm = 173,
    syscall_x86_64_create_module = 174,
    syscall_x86_64_init_module = 175,
    syscall_x86_64_delete_module = 176,
    syscall_x86_64_get_kernel_syms = 177,
    syscall_x86_64_query_module = 178,
    syscall_x86_64_quotactl = 179,
    syscall_x86_64_nfsservctl = 180,
    syscall_x86_64_getpmsg = 181,
    syscall_x86_64_putpmsg = 182,
    syscall_x86_64_afs_syscall = 183,
    syscall_x86_64_tuxcall = 184,
    syscall_x86_64_security = 185,
    syscall_x86_64_gettid = 186,
    syscall_x86_64_readahead = 187,
    syscall_x86_64_setxattr = 188,
    syscall_x86_64_lsetxattr = 189,
    syscall_x86_64_fsetxattr = 190,
    syscall_x86_64_getxattr = 191,
    syscall_x86_64_lgetxattr = 192,
    syscall_x86_64_fgetxattr = 193,
    syscall_x86_64_listxattr = 194,
    syscall_x86_64_llistxattr = 195,
    syscall_x86_64_flistxattr = 196,
    syscall_x86_64_removexattr = 197,
    syscall_x86_64_lremovexattr = 198,
    syscall_x86_64_fremovexattr = 199,
    syscall_x86_64_tkill = 200,
    syscall_x86_64_time = 201,
    syscall_x86_64_futex = 202,
    syscall_x86_64_sched_setaffinity = 203,
    syscall_x86_64_sched_getaffinity = 204,
    syscall_x86_64_set_thread_area = 205,
    syscall_x86_64_io_setup = 206,
    syscall_x86_64_io_destroy = 207,
    syscall_x86_64_io_getevents = 208,
    syscall_x86_64_io_submit = 209,
    syscall_x86_64_io_cancel = 210,
    syscall_x86_64_get_thread_area = 211,
    syscall_x86_64_lookup_dcookie = 212,
    syscall_x86_64_epoll_create = 213,
    syscall_x86_64_epoll_ctl_old = 214,
    syscall_x86_64_epoll_wait_old = 215,
    syscall_x86_64_remap_file_pages = 216,
    syscall_x86_64_getdents64 = 217,
    syscall_x86_64_set_tid_address = 218,
    syscall_x86_64_restart_syscall = 219,
    syscall_x86_64_semtimedop = 220,
    syscall_x86_64_fadvise64 = 221,
    syscall_x86_64_timer_create = 222,
    syscall_x86_64_timer_settime = 223,
    syscall_x86_64_timer_gettime = 224,
    syscall_x86_64_timer_getoverrun = 225,
    syscall_x86_64_timer_delete = 226,
    syscall_x86_64_clock_settime = 227,
    syscall_x86_64_clock_gettime = 228,
    syscall_x86_64_clock_getres = 229,
    syscall_x86_64_clock_nanosleep = 230,
    syscall_x86_64_exit_group = 231,
    syscall_x86_64_epoll_wait = 232,
    syscall_x86_64_epoll_ctl = 233,
    syscall_x86_64_tgkill = 234,
    syscall_x86_64_utimes = 235,
    syscall_x86_64_vserver = 236,
    syscall_x86_64_mbind = 237,
    syscall_x86_64_set_mempolicy = 238,
    syscall_x86_64_get_mempolicy = 239,
    syscall_x86_64_mq_open = 240,
    syscall_x86_64_mq_unlink = 241,
    syscall_x86_64_mq_timedsend = 242,
    syscall_x86_64_mq_timedreceive = 243,
    syscall_x86_64_mq_notify = 244,
    syscall_x86_64_mq_getsetattr = 245,
    syscall_x86_64_kexec_load = 246,
    syscall_x86_64_waitid = 247,
    syscall_x86_64_add_key = 248,
    syscall_x86_64_request_key = 249,
    syscall_x86_64_keyctl = 250,
    syscall_x86_64_ioprio_set = 251,
    syscall_x86_64_ioprio_get = 252,
    syscall_x86_64_inotify_init = 253,
    syscall_x86_64_inotify_add_watch = 254,
    syscall_x86_64_inotify_rm_watch = 255,
    syscall_x86_64_migrate_pages = 256,
    syscall_x86_64_openat = 257,
    syscall_x86_64_mkdirat = 258,
    syscall_x86_64_mknodat = 259,
    syscall_x86_64_fchownat = 260,
    syscall_x86_64_futimesat = 261,
    syscall_x86_64_fstatat64 = 262,
    syscall_x86_64_unlinkat = 263,
    syscall_x86_64_renameat = 264,
    syscall_x86_64_linkat = 265,
    syscall_x86_64_symlinkat = 266,
    syscall_x86_64_readlinkat = 267,
    syscall_x86_64_fchmodat = 268,
    syscall_x86_64_faccessat = 269,
    syscall_x86_64_pselect6 = 270,
    syscall_x86_64_ppoll = 271,
    syscall_x86_64_unshare = 272,
    syscall_x86_64_set_robust_list = 273,
    syscall_x86_64_get_robust_list = 274,
    syscall_x86_64_splice = 275,
    syscall_x86_64_tee = 276,
    syscall_x86_64_sync_file_range = 277,
    syscall_x86_64_vmsplice = 278,
    syscall_x86_64_move_pages = 279,
    syscall_x86_64_utimensat = 280,
    syscall_x86_64_epoll_pwait = 281,
    syscall_x86_64_signalfd = 282,
    syscall_x86_64_timerfd_create = 283,
    syscall_x86_64_eventfd = 284,
    syscall_x86_64_fallocate = 285,
    syscall_x86_64_timerfd_settime = 286,
    syscall_x86_64_timerfd_gettime = 287,
    syscall_x86_64_accept4 = 288,
    syscall_x86_64_signalfd4 = 289,
    syscall_x86_64_eventfd2 = 290,
    syscall_x86_64_epoll_create1 = 291,
    syscall_x86_64_dup3 = 292,
    syscall_x86_64_pipe2 = 293,
    syscall_x86_64_inotify_init1 = 294,
    syscall_x86_64_preadv = 295,
    syscall_x86_64_pwritev = 296,
    syscall_x86_64_rt_tgsigqueueinfo = 297,
    syscall_x86_64_perf_event_open = 298,
    syscall_x86_64_recvmmsg = 299,
    syscall_x86_64_fanotify_init = 300,
    syscall_x86_64_fanotify_mark = 301,
    syscall_x86_64_prlimit64 = 302,
    syscall_x86_64_name_to_handle_at = 303,
    syscall_x86_64_open_by_handle_at = 304,
    syscall_x86_64_clock_adjtime = 305,
    syscall_x86_64_syncfs = 306,
    syscall_x86_64_sendmmsg = 307,
    syscall_x86_64_setns = 308,
    syscall_x86_64_getcpu = 309,
    syscall_x86_64_process_vm_readv = 310,
    syscall_x86_64_process_vm_writev = 311,
    syscall_x86_64_kcmp = 312,
    syscall_x86_64_finit_module = 313,
    syscall_x86_64_sched_setattr = 314,
    syscall_x86_64_sched_getattr = 315,
    syscall_x86_64_renameat2 = 316,
    syscall_x86_64_seccomp = 317,
    syscall_x86_64_getrandom = 318,
    syscall_x86_64_memfd_create = 319,
    syscall_x86_64_kexec_file_load = 320,
    syscall_x86_64_bpf = 321,
    syscall_x86_64_execveat = 322,
    syscall_x86_64_userfaultfd = 323,
    syscall_x86_64_membarrier = 324,
    syscall_x86_64_mlock2 = 325,
    syscall_x86_64_copy_file_range = 326,
    syscall_x86_64_preadv2 = 327,
    syscall_x86_64_pwritev2 = 328,
    syscall_x86_64_pkey_mprotect = 329,
    syscall_x86_64_pkey_alloc = 330,
    syscall_x86_64_pkey_free = 331,
    syscall_x86_64_statx = 332,
    syscall_x86_64_io_pgetevents = 333,
    syscall_x86_64_rseq = 334,
    syscall_x86_64_pidfd_send_signal = 424,
    syscall_x86_64_io_uring_setup = 425,
    syscall_x86_64_io_uring_enter = 426,
    syscall_x86_64_io_uring_register = 427,
    syscall_x86_64_open_tree = 428,
    syscall_x86_64_move_mount = 429,
    syscall_x86_64_fsopen = 430,
    syscall_x86_64_fsconfig = 431,
    syscall_x86_64_fsmount = 432,
    syscall_x86_64_fspick = 433,
    syscall_x86_64_pidfd_open = 434,
    syscall_x86_64_clone3 = 435,
    syscall_x86_64_close_range = 436,
    syscall_x86_64_openat2 = 437,
    syscall_x86_64_pidfd_getfd = 438,
    syscall_x86_64_faccessat2 = 439,
    syscall_x86_64_process_madvise = 440,
    syscall_x86_64_epoll_pwait2 = 441,
    syscall_x86_64_mount_setattr = 442,
    syscall_x86_64_quotactl_fd = 443,
    syscall_x86_64_landlock_create_ruleset = 444,
    syscall_x86_64_landlock_add_rule = 445,
    syscall_x86_64_landlock_restrict_self = 446,
    syscall_x86_64_memfd_secret = 447,
    syscall_x86_64_process_mrelease = 448,
    syscall_x86_64_futex_waitv = 449,
    syscall_x86_64_set_mempolicy_home_node = 450,
    syscall_x86_64_cachestat = 451,
    syscall_x86_64_fchmodat2 = 452,
    syscall_x86_64_map_shadow_stack = 453,
    syscall_x86_64_futex_wake = 454,
    syscall_x86_64_futex_wait = 455,
    syscall_x86_64_futex_requeue = 456,
};
#endif
#endif

#ifndef _WIN32
may_be_unused fn void* posix_mmap(void* address, size_t length, int protection_flags, int map_flags, int fd, signed long offset)
{
#if LINK_LIBC
    return mmap(address, length, protection_flags, map_flags, fd, offset);
#else 
#ifdef __linux__
    return (void*) syscall6(syscall_x86_64_mmap, (s64)address, cast(s64, u64, length), protection_flags, map_flags, fd, offset);
#else
#error "Unsupported operating system for static linking" 
#endif
#endif
}

may_be_unused fn int syscall_mprotect(void *address, size_t length, int protection_flags)
{
#if LINK_LIBC
    return mprotect(address, length, protection_flags);
#else 
#ifdef __linux__
    return cast(s32, s64, syscall3(syscall_x86_64_mprotect, (s64)address, cast(s64, u64, length), protection_flags));
#else
    return mprotect(address, length, protection_flags);
#endif
#endif
}

may_be_unused fn int syscall_open(const char *file_path, int flags, int mode)
{
#if LINK_LIBC
    return open(file_path, flags, mode);
#else
#ifdef __linux__
    return cast(s32, s64, syscall3(syscall_x86_64_open, (s64)file_path, flags, mode));
#else
    return open(file_path, flags, mode);
#endif
#endif
}

may_be_unused fn int syscall_close(int fd)
{
#if LINK_LIBC
    return close(fd);
#else
#ifdef __linux__
    return cast(s32, s64, syscall1(syscall_x86_64_close, fd));
#else
    return close(fd);
#endif
#endif
}

fn int syscall_fstat(int fd, struct stat *buffer)
{
#if LINK_LIBC
    return fstat(fd, buffer);
#else
#ifdef __linux__
    return cast(s32, s64, syscall2(syscall_x86_64_fstat, fd, (s64)buffer));
#else
    return fstat(fd, buffer);
#endif
#endif
}

may_be_unused fn ssize_t syscall_read(int fd, void* buffer, size_t bytes)
{
#if LINK_LIBC
    return read(fd, buffer, bytes);
#else
#ifdef __linux__
    return syscall3(syscall_x86_64_read, fd, (s64)buffer, (s64)bytes);
#else
    return read(fd, buffer, bytes);
#endif
#endif
}

may_be_unused fn ssize_t syscall_write(int fd, const void *buffer, size_t bytes)
{
#if LINK_LIBC
    return write(fd, buffer, bytes);
#else
#ifdef __linux__
    return syscall3(syscall_x86_64_write, fd, (s64)buffer, (s64)bytes);
#else
    return write(fd, buffer, bytes);
#endif
#endif
}

may_be_unused fn int syscall_mkdir(String path, u32 mode)
{
    assert(path.pointer[path.length] == 0);
#if LINK_LIBC
    return mkdir((char*)path.pointer, mode);
#else
    return cast(s32, s64, syscall2(syscall_x86_64_mkdir, (s64)path.pointer, (s64)mode));
#endif
}

may_be_unused fn int syscall_rmdir(String path)
{
    assert(path.pointer[path.length] == 0);
#if LINK_LIBC
    return rmdir((char*)path.pointer);
#else
    return cast(s32, s64, syscall1(syscall_x86_64_rmdir, (s64)path.pointer));
#endif
}

may_be_unused fn int syscall_unlink(String path)
{
    assert(path.pointer[path.length] == 0);
#if LINK_LIBC
    return unlink((char*)path.pointer);
#else
    return cast(s32, s64, syscall1(syscall_x86_64_unlink, (s64)path.pointer));
#endif
}

may_be_unused fn pid_t syscall_fork()
{
#if LINK_LIBC
    return fork();
#else
    return cast(s32, s64, syscall0(syscall_x86_64_fork));
#endif

}

may_be_unused fn signed long syscall_execve(const char* path, char *const argv[], char *const envp[])
{
#if LINK_LIBC
    return execve(path, argv, envp);
#else
    return syscall3(syscall_x86_64_execve, (s64)path, (s64)argv, (s64)envp);
#endif
}

may_be_unused fn pid_t syscall_waitpid(pid_t pid, int* status, int options)
{
#if LINK_LIBC
    return waitpid(pid, status, options);
#else
    return cast(s32, s64, syscall4(syscall_x86_64_wait4, pid, (s64)status, options, 0));
#endif
}

may_be_unused fn int syscall_gettimeofday(struct timeval* tv, struct timezone* tz)
{
#if LINK_LIBC
    return gettimeofday(tv, tz);
#else
    return cast(s32, s64, syscall2(syscall_x86_64_gettimeofday, (s64)tv, (s64)tz));
#endif
}

may_be_unused [[noreturn]] [[gnu::cold]] fn void syscall_exit(int status)
{
#if LINK_LIBC
    _exit(status);
#else
#ifdef __linux__
    (void)syscall1(231, status);
    trap();
#else
    _exit(status);
#endif
#endif
}
#endif

may_be_unused fn u64 os_timer_freq()
{
    return 1000 * 1000;
}

may_be_unused fn u64 os_timer_get()
{
#if _WIN32
    LARGE_INTEGER large_integer;
    QueryPerformanceCounter(&large_integer);
    return (u64)large_integer.QuadPart;
#else
    struct timeval tv;
    syscall_gettimeofday(&tv, 0);
    auto result = os_timer_freq() * cast(u64, s64, tv.tv_sec) + cast(u64, s64, tv.tv_usec);
    return result;
#endif
}

STRUCT(OSFileOpenFlags)
{
    u32 truncate:1;
    u32 executable:1;
    u32 write:1;
    u32 read:1;
    u32 create:1;
};

may_be_unused fn u8 os_file_descriptor_is_valid(FileDescriptor fd)
{
#if _WIN32
    return fd != INVALID_HANDLE_VALUE;
#else
    return fd >= 0;
#endif
}

may_be_unused fn FileDescriptor os_file_open(String path, OSFileOpenFlags flags)
{
    assert(path.pointer[path.length] == 0);
#if _WIN32

    DWORD dwDesiredAccess = 0;
    dwDesiredAccess |= flags.read * GENERIC_READ;
    dwDesiredAccess |= flags.write * GENERIC_WRITE;
    dwDesiredAccess |= flags.write * GENERIC_EXECUTE;
    DWORD dwShareMode = 0;
    LPSECURITY_ATTRIBUTES lpSecurityAttributes = 0;
    DWORD dwCreationDisposition = 0;
    dwCreationDisposition |= (!flags.create) * OPEN_EXISTING;
    dwCreationDisposition |= flags.create * CREATE_ALWAYS;
    DWORD dwFlagsAndAttributes = 0;
    dwFlagsAndAttributes |= FILE_ATTRIBUTE_NORMAL;
    HANDLE hTemplateFile = 0;
    auto handle = CreateFileA((char*)path.pointer, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    return handle;
#else
    int posix_flags = 0;
    posix_flags |= O_WRONLY * (flags.write & !flags.read);
    posix_flags |= O_RDONLY * ((!flags.write) & flags.read);
    posix_flags |= O_RDWR * (flags.write & flags.read);
    posix_flags |= O_CREAT * flags.create;
    posix_flags |= O_TRUNC * flags.truncate;

    int permissions;
    if (flags.executable && flags.write)
    {
        permissions = 0755;
    }
    else
    {
        permissions = 0644;
    }
    auto result = syscall_open((char*)path.pointer, posix_flags, permissions);
    return result;
#endif
}

may_be_unused fn u64 os_file_get_size(FileDescriptor fd)
{
#if _WIN32
    LARGE_INTEGER file_size;
    GetFileSizeEx(fd, &file_size);
    return (u64)file_size.QuadPart;
#else
    struct stat stat_buffer;
    int stat_result = syscall_fstat(fd, &stat_buffer);
    assert(stat_result == 0);
    auto size = cast(u64, s64, stat_buffer.st_size);
    return size;
#endif
}

may_be_unused fn void os_file_write(FileDescriptor fd, String content)
{
#if _WIN32
    WriteFile(fd, content.pointer, cast(u32, u64, content.length), 0, 0);
#else
    syscall_write(fd, content.pointer, content.length);
#endif
}

may_be_unused fn void os_file_read(FileDescriptor fd, String buffer, u64 byte_count)
{
    assert(byte_count <= buffer.length);
    if (byte_count <= buffer.length)
    {
#if _WIN32
        ReadFile(fd, buffer.pointer, cast(u32, u64, byte_count), 0, 0);
#else
        syscall_read(fd, buffer.pointer, byte_count);
#endif
    }
}

may_be_unused fn void os_file_close(FileDescriptor fd)
{
#if _WIN32
    CloseHandle(fd);
#else
    syscall_close(fd);
#endif
}

may_be_unused fn void calibrate_cpu_timer()
{
#ifndef SILENT
#if LINK_LIBC
    // clock_getres(CLOCK_MONOTONIC, &cpu_resolution);
#else
    u64 miliseconds_to_wait = 100;
    u64 cpu_start = timestamp();
    u64 os_frequency = os_timer_freq();
    u64 os_elapsed = 0;
    u64 os_start = os_timer_get();
    u64 os_wait_time = os_frequency * miliseconds_to_wait / 1000;

    while (os_elapsed < os_wait_time)
    {
        auto os_end = os_timer_get();
        os_elapsed = os_end - os_start;
    }

    u64 cpu_end = timestamp();
    u64 cpu_elapsed = cpu_end - cpu_start;
    cpu_frequency = os_frequency * cpu_elapsed / os_elapsed;
#endif
#endif
}

STRUCT(OSReserveProtectionFlags)
{
    u32 read:1;
    u32 write:1;
    u32 execute:1;
    u32 reserved:29;
};

STRUCT(OSReserveMapFlags)
{
    u32 priv:1;
    u32 anon:1;
    u32 noreserve:1;
    u32 reserved:29;
};

fn u8* os_reserve(u64 base, u64 size, OSReserveProtectionFlags protection, OSReserveMapFlags map)
{
#if _WIN32
    DWORD map_flags = 0;
    map_flags |= (MEM_RESERVE * map.noreserve);
    DWORD protection_flags = 0;
    protection_flags |= PAGE_READWRITE * (!protection.write && !protection.read);
    protection_flags |= PAGE_READWRITE * (protection.write && protection.read);
    protection_flags |= PAGE_READONLY * (protection.write && !protection.read);
    return (u8*)VirtualAlloc((void*)base, size, map_flags, protection_flags);
#else
    int protection_flags = (protection.read * PROT_READ) | (protection.write * PROT_WRITE) | (protection.execute * PROT_EXEC);
    int map_flags = (map.anon * MAP_ANONYMOUS) | (map.priv * MAP_PRIVATE) | (map.noreserve * MAP_NORESERVE);
    u8* result = (u8*)posix_mmap((void*)base, size, protection_flags, map_flags, -1, 0);
    assert(result != MAP_FAILED);
    return result;
#endif
}

fn void commit(void* address, u64 size)
{
#if _WIN32
    VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
#else
    int result = syscall_mprotect(address, size, PROT_READ | PROT_WRITE);
    assert(result == 0);
#endif
}

may_be_unused fn void os_directory_make(String path)
{
    assert(path.pointer[path.length] == 0);
#if _WIN32
    CreateDirectoryA((char*)path.pointer, 0);
#else
    syscall_mkdir(path, 0755);
#endif
}

may_be_unused fn u64 align_forward(u64 value, u64 alignment)
{
    u64 mask = alignment - 1;
    u64 result = (value + mask) & ~mask;
    return result;
}

may_be_unused fn u64 align_backward(u64 value, u64 alignment)
{
    u64 result = value & ~(alignment - 1);
    return result;
}

fn u32 format_hexadecimal(String buffer, u64 hexadecimal)
{
    u64 value = hexadecimal;
    if (value)
    {
        u8 reverse_buffer[16];
        u8 reverse_index = 0;

        while (value)
        {
            u8 digit_value = value % 16;
            u8 ascii_ch = digit_value >= 10 ? (digit_value + 'a' - 10) : (digit_value + '0');
            value /= 16;
            reverse_buffer[reverse_index] = ascii_ch;
            reverse_index += 1;
        }

        u32 index = 0;

        while (reverse_index > 0)
        {
            reverse_index -= 1;
            buffer.pointer[index] = reverse_buffer[reverse_index];
            index += 1;
        }

        return index;
    }
    else
    {
        buffer.pointer[0] = '0';
        return 1;
    }
}

fn u32 format_decimal(String buffer, u64 decimal)
{
    u64 value = decimal;
    if (value)
    {
        u8 reverse_buffer[64];
        u8 reverse_index = 0;

        while (value)
        {
            u8 digit_value = (value % 10);
            u8 ascii_ch = digit_value + '0';
            value /= 10;
            reverse_buffer[reverse_index] = ascii_ch;
            reverse_index += 1;
        }

        u32 index = 0;
        while (reverse_index > 0)
        {
            reverse_index -= 1;
            buffer.pointer[index] = reverse_buffer[reverse_index];
            index += 1;
        }

        return index;
    }
    else
    {
        buffer.pointer[0] = '0';
        return 1;
    }
}

STRUCT(SmallIntResult)
{
    u64 mantissa;
    s32 exponent;
    u8 is_small_int;
};

#define double_mantissa_bits 52
#define double_exponent_bits 11
#define double_bias 1023

#define double_pow5_bitcount 125
#define double_pow5_inv_bitcount 125

// Returns floor(log_10(2^e)); requires 0 <= e <= 1650.
fn u32 log10_pow2(const s32 e)
{
  // The first value this approximation fails for is 2^1651 which is just greater than 10^297.
  assert(e >= 0);
  assert(e <= 1650);
  return (((u32) e) * 78913) >> 18;
}

// Returns e == 0 ? 1 : ceil(log_2(5^e)); requires 0 <= e <= 3528.
fn s32 pow5_bits(const s32 e)
{
  // This approximation works up to the point that the multiplication overflows at e = 3529.
  // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
  // than 2^9297.
  assert(e >= 0);
  assert(e <= 3528);
  return (s32) (((((u32) e) * 1217359) >> 19) + 1);
}

#define DOUBLE_POW5_INV_BITCOUNT 125
#define DOUBLE_POW5_BITCOUNT 125

#define DOUBLE_POW5_INV_TABLE_SIZE 342
#define DOUBLE_POW5_TABLE_SIZE 326

global const u8 DIGIT_TABLE[200] = {
  '0','0','0','1','0','2','0','3','0','4','0','5','0','6','0','7','0','8','0','9',
  '1','0','1','1','1','2','1','3','1','4','1','5','1','6','1','7','1','8','1','9',
  '2','0','2','1','2','2','2','3','2','4','2','5','2','6','2','7','2','8','2','9',
  '3','0','3','1','3','2','3','3','3','4','3','5','3','6','3','7','3','8','3','9',
  '4','0','4','1','4','2','4','3','4','4','4','5','4','6','4','7','4','8','4','9',
  '5','0','5','1','5','2','5','3','5','4','5','5','5','6','5','7','5','8','5','9',
  '6','0','6','1','6','2','6','3','6','4','6','5','6','6','6','7','6','8','6','9',
  '7','0','7','1','7','2','7','3','7','4','7','5','7','6','7','7','7','8','7','9',
  '8','0','8','1','8','2','8','3','8','4','8','5','8','6','8','7','8','8','8','9',
  '9','0','9','1','9','2','9','3','9','4','9','5','9','6','9','7','9','8','9','9'
};

global const u64 DOUBLE_POW5_INV_SPLIT[DOUBLE_POW5_INV_TABLE_SIZE][2] =
{
    {                    1u, 2305843009213693952u }, { 11068046444225730970u, 1844674407370955161u },
    {  5165088340638674453u, 1475739525896764129u }, {  7821419487252849886u, 1180591620717411303u },
    {  8824922364862649494u, 1888946593147858085u }, {  7059937891890119595u, 1511157274518286468u },
    { 13026647942995916322u, 1208925819614629174u }, {  9774590264567735146u, 1934281311383406679u },
    { 11509021026396098440u, 1547425049106725343u }, { 16585914450600699399u, 1237940039285380274u },
    { 15469416676735388068u, 1980704062856608439u }, { 16064882156130220778u, 1584563250285286751u },
    {  9162556910162266299u, 1267650600228229401u }, {  7281393426775805432u, 2028240960365167042u },
    { 16893161185646375315u, 1622592768292133633u }, {  2446482504291369283u, 1298074214633706907u },
    {  7603720821608101175u, 2076918743413931051u }, {  2393627842544570617u, 1661534994731144841u },
    { 16672297533003297786u, 1329227995784915872u }, { 11918280793837635165u, 2126764793255865396u },
    {  5845275820328197809u, 1701411834604692317u }, { 15744267100488289217u, 1361129467683753853u },
    {  3054734472329800808u, 2177807148294006166u }, { 17201182836831481939u, 1742245718635204932u },
    {  6382248639981364905u, 1393796574908163946u }, {  2832900194486363201u, 2230074519853062314u },
    {  5955668970331000884u, 1784059615882449851u }, {  1075186361522890384u, 1427247692705959881u },
    { 12788344622662355584u, 2283596308329535809u }, { 13920024512871794791u, 1826877046663628647u },
    {  3757321980813615186u, 1461501637330902918u }, { 10384555214134712795u, 1169201309864722334u },
    {  5547241898389809503u, 1870722095783555735u }, {  4437793518711847602u, 1496577676626844588u },
    { 10928932444453298728u, 1197262141301475670u }, { 17486291911125277965u, 1915619426082361072u },
    {  6610335899416401726u, 1532495540865888858u }, { 12666966349016942027u, 1225996432692711086u },
    { 12888448528943286597u, 1961594292308337738u }, { 17689456452638449924u, 1569275433846670190u },
    { 14151565162110759939u, 1255420347077336152u }, {  7885109000409574610u, 2008672555323737844u },
    {  9997436015069570011u, 1606938044258990275u }, {  7997948812055656009u, 1285550435407192220u },
    { 12796718099289049614u, 2056880696651507552u }, {  2858676849947419045u, 1645504557321206042u },
    { 13354987924183666206u, 1316403645856964833u }, { 17678631863951955605u, 2106245833371143733u },
    {  3074859046935833515u, 1684996666696914987u }, { 13527933681774397782u, 1347997333357531989u },
    { 10576647446613305481u, 2156795733372051183u }, { 15840015586774465031u, 1725436586697640946u },
    {  8982663654677661702u, 1380349269358112757u }, { 18061610662226169046u, 2208558830972980411u },
    { 10759939715039024913u, 1766847064778384329u }, { 12297300586773130254u, 1413477651822707463u },
    { 15986332124095098083u, 2261564242916331941u }, {  9099716884534168143u, 1809251394333065553u },
    { 14658471137111155161u, 1447401115466452442u }, {  4348079280205103483u, 1157920892373161954u },
    { 14335624477811986218u, 1852673427797059126u }, {  7779150767507678651u, 1482138742237647301u },
    {  2533971799264232598u, 1185710993790117841u }, { 15122401323048503126u, 1897137590064188545u },
    { 12097921058438802501u, 1517710072051350836u }, {  5988988032009131678u, 1214168057641080669u },
    { 16961078480698431330u, 1942668892225729070u }, { 13568862784558745064u, 1554135113780583256u },
    {  7165741412905085728u, 1243308091024466605u }, { 11465186260648137165u, 1989292945639146568u },
    { 16550846638002330379u, 1591434356511317254u }, { 16930026125143774626u, 1273147485209053803u },
    {  4951948911778577463u, 2037035976334486086u }, {   272210314680951647u, 1629628781067588869u },
    {  3907117066486671641u, 1303703024854071095u }, {  6251387306378674625u, 2085924839766513752u },
    { 16069156289328670670u, 1668739871813211001u }, {  9165976216721026213u, 1334991897450568801u },
    {  7286864317269821294u, 2135987035920910082u }, { 16897537898041588005u, 1708789628736728065u },
    { 13518030318433270404u, 1367031702989382452u }, {  6871453250525591353u, 2187250724783011924u },
    {  9186511415162383406u, 1749800579826409539u }, { 11038557946871817048u, 1399840463861127631u },
    { 10282995085511086630u, 2239744742177804210u }, {  8226396068408869304u, 1791795793742243368u },
    { 13959814484210916090u, 1433436634993794694u }, { 11267656730511734774u, 2293498615990071511u },
    {  5324776569667477496u, 1834798892792057209u }, {  7949170070475892320u, 1467839114233645767u },
    { 17427382500606444826u, 1174271291386916613u }, {  5747719112518849781u, 1878834066219066582u },
    { 15666221734240810795u, 1503067252975253265u }, { 12532977387392648636u, 1202453802380202612u },
    {  5295368560860596524u, 1923926083808324180u }, {  4236294848688477220u, 1539140867046659344u },
    {  7078384693692692099u, 1231312693637327475u }, { 11325415509908307358u, 1970100309819723960u },
    {  9060332407926645887u, 1576080247855779168u }, { 14626963555825137356u, 1260864198284623334u },
    { 12335095245094488799u, 2017382717255397335u }, {  9868076196075591040u, 1613906173804317868u },
    { 15273158586344293478u, 1291124939043454294u }, { 13369007293925138595u, 2065799902469526871u },
    {  7005857020398200553u, 1652639921975621497u }, { 16672732060544291412u, 1322111937580497197u },
    { 11918976037903224966u, 2115379100128795516u }, {  5845832015580669650u, 1692303280103036413u },
    { 12055363241948356366u, 1353842624082429130u }, {   841837113407818570u, 2166148198531886609u },
    {  4362818505468165179u, 1732918558825509287u }, { 14558301248600263113u, 1386334847060407429u },
    { 12225235553534690011u, 2218135755296651887u }, {  2401490813343931363u, 1774508604237321510u },
    {  1921192650675145090u, 1419606883389857208u }, { 17831303500047873437u, 2271371013423771532u },
    {  6886345170554478103u, 1817096810739017226u }, {  1819727321701672159u, 1453677448591213781u },
    { 16213177116328979020u, 1162941958872971024u }, { 14873036941900635463u, 1860707134196753639u },
    { 15587778368262418694u, 1488565707357402911u }, {  8780873879868024632u, 1190852565885922329u },
    {  2981351763563108441u, 1905364105417475727u }, { 13453127855076217722u, 1524291284333980581u },
    {  7073153469319063855u, 1219433027467184465u }, { 11317045550910502167u, 1951092843947495144u },
    { 12742985255470312057u, 1560874275157996115u }, { 10194388204376249646u, 1248699420126396892u },
    {  1553625868034358140u, 1997919072202235028u }, {  8621598323911307159u, 1598335257761788022u },
    { 17965325103354776697u, 1278668206209430417u }, { 13987124906400001422u, 2045869129935088668u },
    {   121653480894270168u, 1636695303948070935u }, {    97322784715416134u, 1309356243158456748u },
    { 14913111714512307107u, 2094969989053530796u }, {  8241140556867935363u, 1675975991242824637u },
    { 17660958889720079260u, 1340780792994259709u }, { 17189487779326395846u, 2145249268790815535u },
    { 13751590223461116677u, 1716199415032652428u }, { 18379969808252713988u, 1372959532026121942u },
    { 14650556434236701088u, 2196735251241795108u }, {   652398703163629901u, 1757388200993436087u },
    { 11589965406756634890u, 1405910560794748869u }, {  7475898206584884855u, 2249456897271598191u },
    {  2291369750525997561u, 1799565517817278553u }, {  9211793429904618695u, 1439652414253822842u },
    { 18428218302589300235u, 2303443862806116547u }, {  7363877012587619542u, 1842755090244893238u },
    { 13269799239553916280u, 1474204072195914590u }, { 10615839391643133024u, 1179363257756731672u },
    {  2227947767661371545u, 1886981212410770676u }, { 16539753473096738529u, 1509584969928616540u },
    { 13231802778477390823u, 1207667975942893232u }, {  6413489186596184024u, 1932268761508629172u },
    { 16198837793502678189u, 1545815009206903337u }, {  5580372605318321905u, 1236652007365522670u },
    {  8928596168509315048u, 1978643211784836272u }, { 18210923379033183008u, 1582914569427869017u },
    {  7190041073742725760u, 1266331655542295214u }, {   436019273762630246u, 2026130648867672343u },
    {  7727513048493924843u, 1620904519094137874u }, {  9871359253537050198u, 1296723615275310299u },
    {  4726128361433549347u, 2074757784440496479u }, {  7470251503888749801u, 1659806227552397183u },
    { 13354898832594820487u, 1327844982041917746u }, { 13989140502667892133u, 2124551971267068394u },
    { 14880661216876224029u, 1699641577013654715u }, { 11904528973500979224u, 1359713261610923772u },
    {  4289851098633925465u, 2175541218577478036u }, { 18189276137874781665u, 1740432974861982428u },
    {  3483374466074094362u, 1392346379889585943u }, {  1884050330976640656u, 2227754207823337509u },
    {  5196589079523222848u, 1782203366258670007u }, { 15225317707844309248u, 1425762693006936005u },
    {  5913764258841343181u, 2281220308811097609u }, {  8420360221814984868u, 1824976247048878087u },
    { 17804334621677718864u, 1459980997639102469u }, { 17932816512084085415u, 1167984798111281975u },
    { 10245762345624985047u, 1868775676978051161u }, {  4507261061758077715u, 1495020541582440929u },
    {  7295157664148372495u, 1196016433265952743u }, {  7982903447895485668u, 1913626293225524389u },
    { 10075671573058298858u, 1530901034580419511u }, {  4371188443704728763u, 1224720827664335609u },
    { 14372599139411386667u, 1959553324262936974u }, { 15187428126271019657u, 1567642659410349579u },
    { 15839291315758726049u, 1254114127528279663u }, {  3206773216762499739u, 2006582604045247462u },
    { 13633465017635730761u, 1605266083236197969u }, { 14596120828850494932u, 1284212866588958375u },
    {  4907049252451240275u, 2054740586542333401u }, {   236290587219081897u, 1643792469233866721u },
    { 14946427728742906810u, 1315033975387093376u }, { 16535586736504830250u, 2104054360619349402u },
    {  5849771759720043554u, 1683243488495479522u }, { 15747863852001765813u, 1346594790796383617u },
    { 10439186904235184007u, 2154551665274213788u }, { 15730047152871967852u, 1723641332219371030u },
    { 12584037722297574282u, 1378913065775496824u }, {  9066413911450387881u, 2206260905240794919u },
    { 10942479943902220628u, 1765008724192635935u }, {  8753983955121776503u, 1412006979354108748u },
    { 10317025513452932081u, 2259211166966573997u }, {   874922781278525018u, 1807368933573259198u },
    {  8078635854506640661u, 1445895146858607358u }, { 13841606313089133175u, 1156716117486885886u },
    { 14767872471458792434u, 1850745787979017418u }, {   746251532941302978u, 1480596630383213935u },
    {   597001226353042382u, 1184477304306571148u }, { 15712597221132509104u, 1895163686890513836u },
    {  8880728962164096960u, 1516130949512411069u }, { 10793931984473187891u, 1212904759609928855u },
    { 17270291175157100626u, 1940647615375886168u }, {  2748186495899949531u, 1552518092300708935u },
    {  2198549196719959625u, 1242014473840567148u }, { 18275073973719576693u, 1987223158144907436u },
    { 10930710364233751031u, 1589778526515925949u }, { 12433917106128911148u, 1271822821212740759u },
    {  8826220925580526867u, 2034916513940385215u }, {  7060976740464421494u, 1627933211152308172u },
    { 16716827836597268165u, 1302346568921846537u }, { 11989529279587987770u, 2083754510274954460u },
    {  9591623423670390216u, 1667003608219963568u }, { 15051996368420132820u, 1333602886575970854u },
    { 13015147745246481542u, 2133764618521553367u }, {  3033420566713364587u, 1707011694817242694u },
    {  6116085268112601993u, 1365609355853794155u }, {  9785736428980163188u, 2184974969366070648u },
    { 15207286772667951197u, 1747979975492856518u }, {  1097782973908629988u, 1398383980394285215u },
    {  1756452758253807981u, 2237414368630856344u }, {  5094511021344956708u, 1789931494904685075u },
    {  4075608817075965366u, 1431945195923748060u }, {  6520974107321544586u, 2291112313477996896u },
    {  1527430471115325346u, 1832889850782397517u }, { 12289990821117991246u, 1466311880625918013u },
    { 17210690286378213644u, 1173049504500734410u }, {  9090360384495590213u, 1876879207201175057u },
    { 18340334751822203140u, 1501503365760940045u }, { 14672267801457762512u, 1201202692608752036u },
    { 16096930852848599373u, 1921924308174003258u }, {  1809498238053148529u, 1537539446539202607u },
    { 12515645034668249793u, 1230031557231362085u }, {  1578287981759648052u, 1968050491570179337u },
    { 12330676829633449412u, 1574440393256143469u }, { 13553890278448669853u, 1259552314604914775u },
    {  3239480371808320148u, 2015283703367863641u }, { 17348979556414297411u, 1612226962694290912u },
    {  6500486015647617283u, 1289781570155432730u }, { 10400777625036187652u, 2063650512248692368u },
    { 15699319729512770768u, 1650920409798953894u }, { 16248804598352126938u, 1320736327839163115u },
    {  7551343283653851484u, 2113178124542660985u }, {  6041074626923081187u, 1690542499634128788u },
    { 12211557331022285596u, 1352433999707303030u }, {  1091747655926105338u, 2163894399531684849u },
    {  4562746939482794594u, 1731115519625347879u }, {  7339546366328145998u, 1384892415700278303u },
    {  8053925371383123274u, 2215827865120445285u }, {  6443140297106498619u, 1772662292096356228u },
    { 12533209867169019542u, 1418129833677084982u }, {  5295740528502789974u, 2269007733883335972u },
    { 15304638867027962949u, 1815206187106668777u }, {  4865013464138549713u, 1452164949685335022u },
    { 14960057215536570740u, 1161731959748268017u }, {  9178696285890871890u, 1858771135597228828u },
    { 14721654658196518159u, 1487016908477783062u }, {  4398626097073393881u, 1189613526782226450u },
    {  7037801755317430209u, 1903381642851562320u }, {  5630241404253944167u, 1522705314281249856u },
    {   814844308661245011u, 1218164251424999885u }, {  1303750893857992017u, 1949062802279999816u },
    { 15800395974054034906u, 1559250241823999852u }, {  5261619149759407279u, 1247400193459199882u },
    { 12107939454356961969u, 1995840309534719811u }, {  5997002748743659252u, 1596672247627775849u },
    {  8486951013736837725u, 1277337798102220679u }, {  2511075177753209390u, 2043740476963553087u },
    { 13076906586428298482u, 1634992381570842469u }, { 14150874083884549109u, 1307993905256673975u },
    {  4194654460505726958u, 2092790248410678361u }, { 18113118827372222859u, 1674232198728542688u },
    {  3422448617672047318u, 1339385758982834151u }, { 16543964232501006678u, 2143017214372534641u },
    {  9545822571258895019u, 1714413771498027713u }, { 15015355686490936662u, 1371531017198422170u },
    {  5577825024675947042u, 2194449627517475473u }, { 11840957649224578280u, 1755559702013980378u },
    { 16851463748863483271u, 1404447761611184302u }, { 12204946739213931940u, 2247116418577894884u },
    { 13453306206113055875u, 1797693134862315907u }, {  3383947335406624054u, 1438154507889852726u },
    { 16482362180876329456u, 2301047212623764361u }, {  9496540929959153242u, 1840837770099011489u },
    { 11286581558709232917u, 1472670216079209191u }, {  5339916432225476010u, 1178136172863367353u },
    {  4854517476818851293u, 1885017876581387765u }, {  3883613981455081034u, 1508014301265110212u },
    { 14174937629389795797u, 1206411441012088169u }, { 11611853762797942306u, 1930258305619341071u },
    {  5600134195496443521u, 1544206644495472857u }, { 15548153800622885787u, 1235365315596378285u },
    {  6430302007287065643u, 1976584504954205257u }, { 16212288050055383484u, 1581267603963364205u },
    { 12969830440044306787u, 1265014083170691364u }, {  9683682259845159889u, 2024022533073106183u },
    { 15125643437359948558u, 1619218026458484946u }, {  8411165935146048523u, 1295374421166787957u },
    { 17147214310975587960u, 2072599073866860731u }, { 10028422634038560045u, 1658079259093488585u },
    {  8022738107230848036u, 1326463407274790868u }, {  9147032156827446534u, 2122341451639665389u },
    { 11006974540203867551u, 1697873161311732311u }, {  5116230817421183718u, 1358298529049385849u },
    { 15564666937357714594u, 2173277646479017358u }, {  1383687105660440706u, 1738622117183213887u },
    { 12174996128754083534u, 1390897693746571109u }, {  8411947361780802685u, 2225436309994513775u },
    {  6729557889424642148u, 1780349047995611020u }, {  5383646311539713719u, 1424279238396488816u },
    {  1235136468979721303u, 2278846781434382106u }, { 15745504434151418335u, 1823077425147505684u },
    { 16285752362063044992u, 1458461940118004547u }, {  5649904260166615347u, 1166769552094403638u },
    {  5350498001524674232u, 1866831283351045821u }, {   591049586477829062u, 1493465026680836657u },
    { 11540886113407994219u, 1194772021344669325u }, {    18673707743239135u, 1911635234151470921u },
    { 14772334225162232601u, 1529308187321176736u }, {  8128518565387875758u, 1223446549856941389u },
    {  1937583260394870242u, 1957514479771106223u }, {  8928764237799716840u, 1566011583816884978u },
    { 14521709019723594119u, 1252809267053507982u }, {  8477339172590109297u, 2004494827285612772u },
    { 17849917782297818407u, 1603595861828490217u }, {  6901236596354434079u, 1282876689462792174u },
    { 18420676183650915173u, 2052602703140467478u }, {  3668494502695001169u, 1642082162512373983u },
    { 10313493231639821582u, 1313665730009899186u }, {  9122891541139893884u, 2101865168015838698u },
    { 14677010862395735754u, 1681492134412670958u }, {   673562245690857633u, 1345193707530136767u }
};
// Best case: use 128-bit type.
fn u64 mul_shift_64(const u64 m, const u64* const mul, const s32 j)
{
    const u128 b0 = ((u128) m) * mul[0];
    const u128 b2 = ((u128) m) * mul[1];
    return (u64) (((b0 >> 64) + b2) >> (j - 64));
}

fn u64 mul_shift_all_64(const u64 m, const u64* const mul, const s32 j, u64* const vp, u64* const vm, const u32 mmShift)
{
    *vp = mul_shift_64(4 * m + 2, mul, j);
    *vm = mul_shift_64(4 * m - 1 - mmShift, mul, j);
    return mul_shift_64(4 * m, mul, j);
}

// Returns e == 0 ? 1 : [log_2(5^e)]; requires 0 <= e <= 3528.
// fn s32 log2_pow5(const s32 e)
// {
//   // This approximation works up to the point that the multiplication overflows at e = 3529.
//   // If the multiplication were done in 64 bits, it would fail at 5^4004 which is just greater
//   // than 2^9297.
//   assert(e >= 0);
//   assert(e <= 3528);
//   return (s32) ((((u32) e) * 1217359) >> 19);
// }
// Returns floor(log_10(5^e)); requires 0 <= e <= 2620.
fn u32 log10_pow5(const s32 e) {
  // The first value this approximation fails for is 5^2621 which is just greater than 10^1832.
  assert(e >= 0);
  assert(e <= 2620);
  return (((u32) e) * 732923) >> 20;
}

global const u64 DOUBLE_POW5_SPLIT[DOUBLE_POW5_TABLE_SIZE][2] =
{
    {                    0u, 1152921504606846976u }, {                    0u, 1441151880758558720u },
    {                    0u, 1801439850948198400u }, {                    0u, 2251799813685248000u },
    {                    0u, 1407374883553280000u }, {                    0u, 1759218604441600000u },
    {                    0u, 2199023255552000000u }, {                    0u, 1374389534720000000u },
    {                    0u, 1717986918400000000u }, {                    0u, 2147483648000000000u },
    {                    0u, 1342177280000000000u }, {                    0u, 1677721600000000000u },
    {                    0u, 2097152000000000000u }, {                    0u, 1310720000000000000u },
    {                    0u, 1638400000000000000u }, {                    0u, 2048000000000000000u },
    {                    0u, 1280000000000000000u }, {                    0u, 1600000000000000000u },
    {                    0u, 2000000000000000000u }, {                    0u, 1250000000000000000u },
    {                    0u, 1562500000000000000u }, {                    0u, 1953125000000000000u },
    {                    0u, 1220703125000000000u }, {                    0u, 1525878906250000000u },
    {                    0u, 1907348632812500000u }, {                    0u, 1192092895507812500u },
    {                    0u, 1490116119384765625u }, {  4611686018427387904u, 1862645149230957031u },
    {  9799832789158199296u, 1164153218269348144u }, { 12249790986447749120u, 1455191522836685180u },
    { 15312238733059686400u, 1818989403545856475u }, { 14528612397897220096u, 2273736754432320594u },
    { 13692068767113150464u, 1421085471520200371u }, { 12503399940464050176u, 1776356839400250464u },
    { 15629249925580062720u, 2220446049250313080u }, {  9768281203487539200u, 1387778780781445675u },
    {  7598665485932036096u, 1734723475976807094u }, {   274959820560269312u, 2168404344971008868u },
    {  9395221924704944128u, 1355252715606880542u }, {  2520655369026404352u, 1694065894508600678u },
    { 12374191248137781248u, 2117582368135750847u }, { 14651398557727195136u, 1323488980084844279u },
    { 13702562178731606016u, 1654361225106055349u }, {  3293144668132343808u, 2067951531382569187u },
    { 18199116482078572544u, 1292469707114105741u }, {  8913837547316051968u, 1615587133892632177u },
    { 15753982952572452864u, 2019483917365790221u }, { 12152082354571476992u, 1262177448353618888u },
    { 15190102943214346240u, 1577721810442023610u }, {  9764256642163156992u, 1972152263052529513u },
    { 17631875447420442880u, 1232595164407830945u }, {  8204786253993389888u, 1540743955509788682u },
    {  1032610780636961552u, 1925929944387235853u }, {  2951224747111794922u, 1203706215242022408u },
    {  3689030933889743652u, 1504632769052528010u }, { 13834660704216955373u, 1880790961315660012u },
    { 17870034976990372916u, 1175494350822287507u }, { 17725857702810578241u, 1469367938527859384u },
    {  3710578054803671186u, 1836709923159824231u }, {    26536550077201078u, 2295887403949780289u },
    { 11545800389866720434u, 1434929627468612680u }, { 14432250487333400542u, 1793662034335765850u },
    {  8816941072311974870u, 2242077542919707313u }, { 17039803216263454053u, 1401298464324817070u },
    { 12076381983474541759u, 1751623080406021338u }, {  5872105442488401391u, 2189528850507526673u },
    { 15199280947623720629u, 1368455531567204170u }, {  9775729147674874978u, 1710569414459005213u },
    { 16831347453020981627u, 2138211768073756516u }, {  1296220121283337709u, 1336382355046097823u },
    { 15455333206886335848u, 1670477943807622278u }, { 10095794471753144002u, 2088097429759527848u },
    {  6309871544845715001u, 1305060893599704905u }, { 12499025449484531656u, 1631326116999631131u },
    { 11012095793428276666u, 2039157646249538914u }, { 11494245889320060820u, 1274473528905961821u },
    {   532749306367912313u, 1593091911132452277u }, {  5277622651387278295u, 1991364888915565346u },
    {  7910200175544436838u, 1244603055572228341u }, { 14499436237857933952u, 1555753819465285426u },
    {  8900923260467641632u, 1944692274331606783u }, { 12480606065433357876u, 1215432671457254239u },
    { 10989071563364309441u, 1519290839321567799u }, {  9124653435777998898u, 1899113549151959749u },
    {  8008751406574943263u, 1186945968219974843u }, {  5399253239791291175u, 1483682460274968554u },
    { 15972438586593889776u, 1854603075343710692u }, {   759402079766405302u, 1159126922089819183u },
    { 14784310654990170340u, 1448908652612273978u }, {  9257016281882937117u, 1811135815765342473u },
    { 16182956370781059300u, 2263919769706678091u }, {  7808504722524468110u, 1414949856066673807u },
    {  5148944884728197234u, 1768687320083342259u }, {  1824495087482858639u, 2210859150104177824u },
    {  1140309429676786649u, 1381786968815111140u }, {  1425386787095983311u, 1727233711018888925u },
    {  6393419502297367043u, 2159042138773611156u }, { 13219259225790630210u, 1349401336733506972u },
    { 16524074032238287762u, 1686751670916883715u }, { 16043406521870471799u, 2108439588646104644u },
    {   803757039314269066u, 1317774742903815403u }, { 14839754354425000045u, 1647218428629769253u },
    {  4714634887749086344u, 2059023035787211567u }, {  9864175832484260821u, 1286889397367007229u },
    { 16941905809032713930u, 1608611746708759036u }, {  2730638187581340797u, 2010764683385948796u },
    { 10930020904093113806u, 1256727927116217997u }, { 18274212148543780162u, 1570909908895272496u },
    {  4396021111970173586u, 1963637386119090621u }, {  5053356204195052443u, 1227273366324431638u },
    { 15540067292098591362u, 1534091707905539547u }, { 14813398096695851299u, 1917614634881924434u },
    { 13870059828862294966u, 1198509146801202771u }, { 12725888767650480803u, 1498136433501503464u },
    { 15907360959563101004u, 1872670541876879330u }, { 14553786618154326031u, 1170419088673049581u },
    {  4357175217410743827u, 1463023860841311977u }, { 10058155040190817688u, 1828779826051639971u },
    {  7961007781811134206u, 2285974782564549964u }, { 14199001900486734687u, 1428734239102843727u },
    { 13137066357181030455u, 1785917798878554659u }, { 11809646928048900164u, 2232397248598193324u },
    { 16604401366885338411u, 1395248280373870827u }, { 16143815690179285109u, 1744060350467338534u },
    { 10956397575869330579u, 2180075438084173168u }, {  6847748484918331612u, 1362547148802608230u },
    { 17783057643002690323u, 1703183936003260287u }, { 17617136035325974999u, 2128979920004075359u },
    { 17928239049719816230u, 1330612450002547099u }, { 17798612793722382384u, 1663265562503183874u },
    { 13024893955298202172u, 2079081953128979843u }, {  5834715712847682405u, 1299426220705612402u },
    { 16516766677914378815u, 1624282775882015502u }, { 11422586310538197711u, 2030353469852519378u },
    { 11750802462513761473u, 1268970918657824611u }, { 10076817059714813937u, 1586213648322280764u },
    { 12596021324643517422u, 1982767060402850955u }, {  5566670318688504437u, 1239229412751781847u },
    {  2346651879933242642u, 1549036765939727309u }, {  7545000868343941206u, 1936295957424659136u },
    {  4715625542714963254u, 1210184973390411960u }, {  5894531928393704067u, 1512731216738014950u },
    { 16591536947346905892u, 1890914020922518687u }, { 17287239619732898039u, 1181821263076574179u },
    { 16997363506238734644u, 1477276578845717724u }, {  2799960309088866689u, 1846595723557147156u },
    { 10973347230035317489u, 1154122327223216972u }, { 13716684037544146861u, 1442652909029021215u },
    { 12534169028502795672u, 1803316136286276519u }, { 11056025267201106687u, 2254145170357845649u },
    { 18439230838069161439u, 1408840731473653530u }, { 13825666510731675991u, 1761050914342066913u },
    {  3447025083132431277u, 2201313642927583642u }, {  6766076695385157452u, 1375821026829739776u },
    {  8457595869231446815u, 1719776283537174720u }, { 10571994836539308519u, 2149720354421468400u },
    {  6607496772837067824u, 1343575221513417750u }, { 17482743002901110588u, 1679469026891772187u },
    { 17241742735199000331u, 2099336283614715234u }, { 15387775227926763111u, 1312085177259197021u },
    {  5399660979626290177u, 1640106471573996277u }, { 11361262242960250625u, 2050133089467495346u },
    { 11712474920277544544u, 1281333180917184591u }, { 10028907631919542777u, 1601666476146480739u },
    {  7924448521472040567u, 2002083095183100924u }, { 14176152362774801162u, 1251301934489438077u },
    {  3885132398186337741u, 1564127418111797597u }, {  9468101516160310080u, 1955159272639746996u },
    { 15140935484454969608u, 1221974545399841872u }, {   479425281859160394u, 1527468181749802341u },
    {  5210967620751338397u, 1909335227187252926u }, { 17091912818251750210u, 1193334516992033078u },
    { 12141518985959911954u, 1491668146240041348u }, { 15176898732449889943u, 1864585182800051685u },
    { 11791404716994875166u, 1165365739250032303u }, { 10127569877816206054u, 1456707174062540379u },
    {  8047776328842869663u, 1820883967578175474u }, {   836348374198811271u, 2276104959472719343u },
    {  7440246761515338900u, 1422565599670449589u }, { 13911994470321561530u, 1778206999588061986u },
    {  8166621051047176104u, 2222758749485077483u }, {  2798295147690791113u, 1389224218428173427u },
    { 17332926989895652603u, 1736530273035216783u }, { 17054472718942177850u, 2170662841294020979u },
    {  8353202440125167204u, 1356664275808763112u }, { 10441503050156459005u, 1695830344760953890u },
    {  3828506775840797949u, 2119787930951192363u }, {    86973725686804766u, 1324867456844495227u },
    { 13943775212390669669u, 1656084321055619033u }, {  3594660960206173375u, 2070105401319523792u },
    {  2246663100128858359u, 1293815875824702370u }, { 12031700912015848757u, 1617269844780877962u },
    {  5816254103165035138u, 2021587305976097453u }, {  5941001823691840913u, 1263492066235060908u },
    {  7426252279614801142u, 1579365082793826135u }, {  4671129331091113523u, 1974206353492282669u },
    {  5225298841145639904u, 1233878970932676668u }, {  6531623551432049880u, 1542348713665845835u },
    {  3552843420862674446u, 1927935892082307294u }, { 16055585193321335241u, 1204959932551442058u },
    { 10846109454796893243u, 1506199915689302573u }, { 18169322836923504458u, 1882749894611628216u },
    { 11355826773077190286u, 1176718684132267635u }, {  9583097447919099954u, 1470898355165334544u },
    { 11978871809898874942u, 1838622943956668180u }, { 14973589762373593678u, 2298278679945835225u },
    {  2440964573842414192u, 1436424174966147016u }, {  3051205717303017741u, 1795530218707683770u },
    { 13037379183483547984u, 2244412773384604712u }, {  8148361989677217490u, 1402757983365377945u },
    { 14797138505523909766u, 1753447479206722431u }, { 13884737113477499304u, 2191809349008403039u },
    { 15595489723564518921u, 1369880843130251899u }, { 14882676136028260747u, 1712351053912814874u },
    {  9379973133180550126u, 2140438817391018593u }, { 17391698254306313589u, 1337774260869386620u },
    {  3292878744173340370u, 1672217826086733276u }, {  4116098430216675462u, 2090272282608416595u },
    {   266718509671728212u, 1306420176630260372u }, {   333398137089660265u, 1633025220787825465u },
    {  5028433689789463235u, 2041281525984781831u }, { 10060300083759496378u, 1275800953740488644u },
    { 12575375104699370472u, 1594751192175610805u }, {  1884160825592049379u, 1993438990219513507u },
    { 17318501580490888525u, 1245899368887195941u }, {  7813068920331446945u, 1557374211108994927u },
    {  5154650131986920777u, 1946717763886243659u }, {   915813323278131534u, 1216698602428902287u },
    { 14979824709379828129u, 1520873253036127858u }, {  9501408849870009354u, 1901091566295159823u },
    { 12855909558809837702u, 1188182228934474889u }, {  2234828893230133415u, 1485227786168093612u },
    {  2793536116537666769u, 1856534732710117015u }, {  8663489100477123587u, 1160334207943823134u },
    {  1605989338741628675u, 1450417759929778918u }, { 11230858710281811652u, 1813022199912223647u },
    {  9426887369424876662u, 2266277749890279559u }, { 12809333633531629769u, 1416423593681424724u },
    { 16011667041914537212u, 1770529492101780905u }, {  6179525747111007803u, 2213161865127226132u },
    { 13085575628799155685u, 1383226165704516332u }, { 16356969535998944606u, 1729032707130645415u },
    { 15834525901571292854u, 2161290883913306769u }, {  2979049660840976177u, 1350806802445816731u },
    { 17558870131333383934u, 1688508503057270913u }, {  8113529608884566205u, 2110635628821588642u },
    {  9682642023980241782u, 1319147268013492901u }, { 16714988548402690132u, 1648934085016866126u },
    { 11670363648648586857u, 2061167606271082658u }, { 11905663298832754689u, 1288229753919426661u },
    {  1047021068258779650u, 1610287192399283327u }, { 15143834390605638274u, 2012858990499104158u },
    {  4853210475701136017u, 1258036869061940099u }, {  1454827076199032118u, 1572546086327425124u },
    {  1818533845248790147u, 1965682607909281405u }, {  3442426662494187794u, 1228551629943300878u },
    { 13526405364972510550u, 1535689537429126097u }, {  3072948650933474476u, 1919611921786407622u },
    { 15755650962115585259u, 1199757451116504763u }, { 15082877684217093670u, 1499696813895630954u },
    {  9630225068416591280u, 1874621017369538693u }, {  8324733676974063502u, 1171638135855961683u },
    {  5794231077790191473u, 1464547669819952104u }, {  7242788847237739342u, 1830684587274940130u },
    { 18276858095901949986u, 2288355734093675162u }, { 16034722328366106645u, 1430222333808546976u },
    {  1596658836748081690u, 1787777917260683721u }, {  6607509564362490017u, 2234722396575854651u },
    {  1823850468512862308u, 1396701497859909157u }, {  6891499104068465790u, 1745876872324886446u },
    { 17837745916940358045u, 2182346090406108057u }, {  4231062170446641922u, 1363966306503817536u },
    {  5288827713058302403u, 1704957883129771920u }, {  6611034641322878003u, 2131197353912214900u },
    { 13355268687681574560u, 1331998346195134312u }, { 16694085859601968200u, 1664997932743917890u },
    { 11644235287647684442u, 2081247415929897363u }, {  4971804045566108824u, 1300779634956185852u },
    {  6214755056957636030u, 1625974543695232315u }, {  3156757802769657134u, 2032468179619040394u },
    {  6584659645158423613u, 1270292612261900246u }, { 17454196593302805324u, 1587865765327375307u },
    { 17206059723201118751u, 1984832206659219134u }, {  6142101308573311315u, 1240520129162011959u },
    {  3065940617289251240u, 1550650161452514949u }, {  8444111790038951954u, 1938312701815643686u },
    {   665883850346957067u, 1211445438634777304u }, {   832354812933696334u, 1514306798293471630u },
    { 10263815553021896226u, 1892883497866839537u }, { 17944099766707154901u, 1183052186166774710u },
    { 13206752671529167818u, 1478815232708468388u }, { 16508440839411459773u, 1848519040885585485u },
    { 12623618533845856310u, 1155324400553490928u }, { 15779523167307320387u, 1444155500691863660u },
    {  1277659885424598868u, 1805194375864829576u }, {  1597074856780748586u, 2256492969831036970u },
    {  5609857803915355770u, 1410308106144398106u }, { 16235694291748970521u, 1762885132680497632u },
    {  1847873790976661535u, 2203606415850622041u }, { 12684136165428883219u, 1377254009906638775u },
    { 11243484188358716120u, 1721567512383298469u }, {   219297180166231438u, 2151959390479123087u },
    {  7054589765244976505u, 1344974619049451929u }, { 13429923224983608535u, 1681218273811814911u },
    { 12175718012802122765u, 2101522842264768639u }, { 14527352785642408584u, 1313451776415480399u },
    { 13547504963625622826u, 1641814720519350499u }, { 12322695186104640628u, 2052268400649188124u },
    { 16925056528170176201u, 1282667750405742577u }, {  7321262604930556539u, 1603334688007178222u },
    { 18374950293017971482u, 2004168360008972777u }, {  4566814905495150320u, 1252605225005607986u },
    { 14931890668723713708u, 1565756531257009982u }, {  9441491299049866327u, 1957195664071262478u },
    {  1289246043478778550u, 1223247290044539049u }, {  6223243572775861092u, 1529059112555673811u },
    {  3167368447542438461u, 1911323890694592264u }, {  1979605279714024038u, 1194577431684120165u },
    {  7086192618069917952u, 1493221789605150206u }, { 18081112809442173248u, 1866527237006437757u },
    { 13606538515115052232u, 1166579523129023598u }, {  7784801107039039482u, 1458224403911279498u },
    {   507629346944023544u, 1822780504889099373u }, {  5246222702107417334u, 2278475631111374216u },
    {  3278889188817135834u, 1424047269444608885u }, {  8710297504448807696u, 1780059086805761106u }
};

fn uint32_t pow5_factor(uint64_t value)
{
  const uint64_t m_inv_5 = 14757395258967641293u; // 5 * m_inv_5 = 1 (mod 2^64)
  const uint64_t n_div_5 = 3689348814741910323u;  // #{ n | n = 0 (mod 2^64) } = 2^64 / 5
  uint32_t count = 0;
  for (;;) {
    assert(value != 0);
    value *= m_inv_5;
    if (value > n_div_5)
      break;
    ++count;
  }
  return count;
}

// Returns true if value is divisible by 5^p.
fn bool multiple_of_power_of_5(const uint64_t value, const uint32_t p)
{
  // I tried a case distinction on p, but there was no performance difference.
  return pow5_factor(value) >= p;
}

// Returns true if value is divisible by 2^p.
fn u8 multiple_of_power_of_2(const u64 value, const u32 p) {
  assert(value != 0);
  assert(p < 64);
  // __builtin_ctzll doesn't appear to be faster here.
  return (value & ((1ull << p) - 1)) == 0;
}

static inline uint64_t div5(const uint64_t x) {
  return x / 5;
}

static inline uint64_t div10(const uint64_t x) {
  return x / 10;
}

static inline uint64_t div100(const uint64_t x) {
  return x / 100;
}

static inline uint64_t div1e8(const uint64_t x) {
  return x / 100000000;
}

static inline uint64_t div1e9(const uint64_t x) {
  return x / 1000000000;
}

static inline uint32_t mod1e9(const uint64_t x) {
  return (uint32_t) (x - 1000000000 * div1e9(x));
}

STRUCT(Double)
{
    u64 mantissa;
    s32 exponent;
};

may_be_unused fn Double double_transform(u64 ieee_mantissa, u32 ieee_exponent)
{
    u64 m2;
    s32 e2;
    if (ieee_exponent)
    {
        m2 = ((u64)1 << double_mantissa_bits) | ieee_mantissa;
        e2 = (s32)(ieee_exponent) - double_bias - double_mantissa_bits - 2;
    }
    else
    {
        m2 = ieee_mantissa;
        e2 = 1 - double_bias - double_mantissa_bits - 2;
    }

    u8 is_even = (m2 & 1) == 0;
    auto accept_bounds = is_even;

    u64 mv = 4 * m2;
    u32 mm_shift = (ieee_mantissa != 0) | (ieee_exponent <= 1);

    u64 vr, vp, vm;
    s32 e10;
    u8 vm_is_trailing_zeroes = 0;
    u8 vr_is_trailing_zeroes = 0;

    if (e2 >= 0)
    {
        u32 q = log10_pow2(e2) - (e2 > 3);
        e10 = (s32)q;
        s32 k = double_pow5_inv_bitcount + pow5_bits((s32)q) - 1;
        s32 i = -e2 + (s32)q + k;
        vr = mul_shift_all_64(m2, DOUBLE_POW5_INV_SPLIT[q], i, &vp, &vm, mm_shift);
        if (q <= 21)
        {
            u32 mv_mod_5 = ((u32)mv) - 5 * ((u32)div5(mv));
            if (mv_mod_5 == 0)
            {
                vr_is_trailing_zeroes = multiple_of_power_of_5(mv, q);
            }
            else if (accept_bounds)
            {
                vm_is_trailing_zeroes = multiple_of_power_of_5(mv - 1 - mm_shift, q);
            }
            else
            {
                vp -= multiple_of_power_of_5(mv + 2, q);
            }
        }
    }
    else
    {
        u32 q = log10_pow5(-e2) - (-e2 > 1);
        e10 = (s32) q + e2;
        s32 i = -e2 - (s32)q;
        s32 k = pow5_bits(i) - double_pow5_bitcount;
        s32 j = (s32)q - k;
        vr = mul_shift_all_64(m2, DOUBLE_POW5_SPLIT[i], j, &vp, &vm, mm_shift);

        if (q <= 1)
        {
            vr_is_trailing_zeroes = 1;
            if (accept_bounds)
            {
                vm_is_trailing_zeroes = mm_shift == 1;
            }
            else
            {
                vp -= 1;
            }
        }
        else if (q < 63)
        {
            vr_is_trailing_zeroes = multiple_of_power_of_2(mv, q);
        }
    }

    s32 removed = 0;
    u64 output;
    u8 last_removed_digit = 0;

    if (vm_is_trailing_zeroes | vr_is_trailing_zeroes)
    {
        while (1)
        {
            u64 vp_div10 = div10(vp);
            u64 vm_div10 = div10(vm);

            if (vp_div10 <= vm_div10)
            {
                break;
            }

            u32 vm_mod10 = ((u32)vm) - 10 * ((u32)vm_div10);
            u32 vr_div10 = div10(vr);
            u32 vr_mod10 = ((u32)vr) - 10 * ((u32)vr_div10);
            vm_is_trailing_zeroes &= vm_mod10 == 0;
            vr_is_trailing_zeroes &= last_removed_digit == 0;
            last_removed_digit = (u8)vr_mod10;
            vr = vr_div10;
            vp = vp_div10;
            vm = vm_div10;
            removed += 1;
        }

        if (vm_is_trailing_zeroes)
        {
            while (1)
            {
                const uint64_t vm_div10 = div10(vm);
                const uint32_t vm_mod10 = ((uint32_t) vm) - 10 * ((uint32_t) vm_div10);

                if (vm_mod10 != 0)
                {
                    break;
                }

                const uint64_t vp_div10 = div10(vp);
                const uint64_t vr_div10 = div10(vr);
                const uint32_t vr_mod10 = ((uint32_t) vr) - 10 * ((uint32_t) vr_div10);
                vr_is_trailing_zeroes &= last_removed_digit == 0;
                last_removed_digit = (uint8_t) vr_mod10;
                vr = vr_div10;
                vp = vp_div10;
                vm = vm_div10;
                removed += 1;
            }
        }

        if (vr_is_trailing_zeroes && last_removed_digit == 5 && vr % 2 == 0)
        {
            // Round even if the exact number is .....50..0.
            last_removed_digit = 4;
        }
        // We need to take vr + 1 if vr is outside bounds or we need to round up.
        output = vr + ((vr == vm && (!accept_bounds || !vm_is_trailing_zeroes)) || last_removed_digit >= 5);
    }
    else
    {
        u8 round_up = 0;
        u64 vp_div100 = div100(vp);
        u64 vm_div100 = div100(vm);

        if (vp_div100 > vm_div100)
        {
            u64 vr_div100 = div100(vr);
            u32 vr_mod100 = ((u32)vr) - 100 * ((u32)vr_div100);
            round_up = vr_mod100 >= 50;
            vr = vr_div100;
            vp = vp_div100;
            vm = vm_div100;
            removed += 2;
        }

        while (1)
        {
            u64 vp_div10 = div10(vp);
            u64 vm_div10 = div10(vm);
            if (vp_div10 <= vm_div10)
            {
                break;
            }
            u64 vr_div10 = div10(vr);
            u32 vr_mod10 = ((u32)vr) - 10 * ((u32) vr_div10);
            round_up = vr_mod10 >= 5;
            vr = vr_div10;
            vp = vp_div10;
            vm = vm_div10;
            removed += 1;
        }

        output = vr + ((vr == vm) | round_up);
    }

    s32 exp = e10 + removed;

    return (Double)
    {
        .mantissa = output,
        .exponent = exp,
    };
}

may_be_unused fn SmallIntResult small_int(u64 ieee_mantissa, u32 ieee_exponent)
{
    SmallIntResult result = {};
    auto m2 = ((u64)1 << double_mantissa_bits) | ieee_mantissa;
    auto e2 = (s32)ieee_exponent - double_bias - double_mantissa_bits;

    if (e2 > 0)
    {
        return result;
    }

    if (e2 < -52)
    {
        return result;
    }

    u64 mask = ((u64)1 << -e2) - 1;
    u64 fraction = m2 & mask;
    if (fraction != 0)
    {
        return result;
    }

    result.mantissa = m2 >> -e2;
    result.exponent = 0;
    result.is_small_int = 1;

    return result;
}

fn u32 decimalLength17(const u64 v) {
    // This is slightly faster than a loop.
    // The average output length is 16.38 digits, so we check high-to-low.
    // Function precondition: v is not an 18, 19, or 20-digit number.
    // (17 digits are sufficient for round-tripping.)
    assert(v < 100000000000000000L);
    if (v >= 10000000000000000L) { return 17; }
    if (v >= 1000000000000000L) { return 16; }
    if (v >= 100000000000000L) { return 15; }
    if (v >= 10000000000000L) { return 14; }
    if (v >= 1000000000000L) { return 13; }
    if (v >= 100000000000L) { return 12; }
    if (v >= 10000000000L) { return 11; }
    if (v >= 1000000000L) { return 10; }
    if (v >= 100000000L) { return 9; }
    if (v >= 10000000L) { return 8; }
    if (v >= 1000000L) { return 7; }
    if (v >= 100000L) { return 6; }
    if (v >= 10000L) { return 5; }
    if (v >= 1000L) { return 4; }
    if (v >= 100L) { return 3; }
    if (v >= 10L) { return 2; }
    return 1;
}

// A floating decimal representing m * 10^e.
STRUCT(floating_decimal_64)
{
  uint64_t mantissa;
  // Decimal exponent's range is -324 to 308
  // inclusive, and can fit in a short if needed.
  int32_t exponent;
};

fn u8* digits2(u64 value)
{
    auto str = strlit("00010203040506070809101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899");
    return str.pointer + (value * 2);
}

fn void write_float_decimal(String buffer, u64* value, u64 count)
{
    u64 i = 0;

    while (i + 2 < count)
    {
        auto c = cast(u8, u64, *value % 100);
        *value /= 100;
        auto ptr = digits2(c);
        buffer.pointer[count - i - 1] = ptr[1];
        buffer.pointer[count - i - 2] = ptr[0];
        i += 2;
    }

    while (i < count)
    {
        auto c = cast(u8, u64, *value % 10);
        *value /= 10;
        buffer.pointer[count - i - 1] = '0' + c;

        i += 1;
    }
}

may_be_unused fn void print(const char* format, ...)
{
#ifndef SILENT
        u8 stack_buffer[4096];
        va_list args;
        va_start(args, format);
        String buffer = { .pointer = stack_buffer, .length = array_length(stack_buffer) };
        u8* it = (u8*)format;
        u64 buffer_i = 0;

        while (*it)
        {
            while (*it && *it != brace_open)
            {
                s_get(buffer, buffer_i) = *it;
                buffer_i += 1;
                it += 1;
            }

            if (*it == brace_open)
            {
                it += 1;
                auto next_ch = *it;

                if (next_ch == brace_open)
                {
                    trap();
                }
                else
                {
                    switch (next_ch)
                    {
                        case 'c':
                            {
                                int done = 0;
                                it += 1;
                                if (*it == 's')
                                {
                                    it += 1;
                                    if (*it == 't')
                                    {
                                        it += 1;
                                        if (*it == 'r')
                                        {
                                            it += 1;
                                            done = 1;
                                            auto* cstring = va_arg(args, const u8*);
                                            while (*cstring)
                                            {
                                                buffer.pointer[buffer_i] = *cstring;
                                                buffer_i += 1;
                                                cstring += 1;
                                            }
                                        }
                                    }
                                }
                                assert(done);
                            } break;
                        case 'f':
                            {
                                it += 1;
                                f64 value_double;
                                switch (*it)
                                {
                                    case '3':
                                        it += 1;
                                        if (*it != '2')
                                        {
                                            fail();
                                        }
                                        it += 1;
                                        fail();
                                        break;
                                    case '6':
                                        it += 1;
                                        if (*it != '4')
                                        {
                                            fail();
                                        }
                                        it += 1;
                                        value_double = va_arg(args, f64);
                                        break;
                                    default:
                                        fail();
                                }

                                {
                                    auto value_int = *(u64*)&value_double;

                                    const u8 ieee_sign = ((value_int >> (double_mantissa_bits + double_exponent_bits)) & 1) != 0;
                                    const auto ieee_mantissa = value_int & (((u64)1 << double_mantissa_bits) - 1);
                                    const auto ieee_exponent = (u32)((value_int >> double_mantissa_bits) & (((u32)1 << double_exponent_bits) - 1));

                                    if (ieee_exponent == (((u32)1 << double_exponent_bits) - 1) || (ieee_exponent == 0 && ieee_mantissa == 0))
                                    {
                                        if (ieee_mantissa)
                                        {
                                            auto nan = strlit("NaN");
                                            memcpy(&buffer.pointer[buffer_i], nan.pointer, nan.length);
                                            buffer_i += nan.length;
                                        }
                                        else
                                        {
                                            if (ieee_sign)
                                            {
                                                buffer.pointer[buffer_i] = '-';
                                                buffer_i += 1;
                                            }

                                            if (ieee_exponent)
                                            {
                                                auto inf = strlit("Infinity");
                                                memcpy(&buffer.pointer[buffer_i], inf.pointer, inf.length);
                                                buffer_i += inf.length;
                                            }
                                            else
                                            {
                                                auto e0 = strlit("0E0");
                                                memcpy(&buffer.pointer[buffer_i], e0.pointer, e0.length);
                                                buffer_i += e0.length;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        auto small_int_result = small_int(ieee_mantissa, ieee_exponent);
                                        Double result;
                                        if (small_int_result.is_small_int)
                                        {
                                            while (1)
                                            {
                                                u64 q = div10(small_int_result.mantissa);
                                                u32 r = ((u32)small_int_result.mantissa) - 10 * ((u32)q);

                                                if (r != 0)
                                                {
                                                    break;
                                                }

                                                small_int_result.mantissa = q;
                                                small_int_result.exponent += 1;
                                            }
                                        }
                                        else
                                        {
                                            result = double_transform(ieee_mantissa, ieee_exponent);
                                        }

                                        typedef enum FloatFormat
                                        {
                                            FLOAT_FORMAT_DECIMAL,
                                            FLOAT_FORMAT_SCIENTIFIC,
                                        } FloatFormat;

                                        FloatFormat format = FLOAT_FORMAT_DECIMAL;
                                        u64 output = result.mantissa;
                                        u32 olength = decimalLength17(output);

                                        // Sign
                                        buffer.pointer[buffer_i] = '-';
                                        buffer_i += ieee_sign;

                                        switch (format)
                                        {
                                            case FLOAT_FORMAT_SCIENTIFIC:
                                                {
                                                    u32 i = 0;

                                                    if ((output >> 32) != 0)
                                                    {
                                                        u64 q = div1e8(output);
                                                        u32 output2 = ((u32)output) - 100000000 * ((u32)q);
                                                        output = q;

                                                        u32 c = output % 10000;
                                                        output2 /= 10000;

                                                        const uint32_t d = output2 % 10000;
                                                        const uint32_t c0 = (c % 100) << 1;
                                                        const uint32_t c1 = (c / 100) << 1;
                                                        const uint32_t d0 = (d % 100) << 1;
                                                        const uint32_t d1 = (d / 100) << 1;

                                                        auto base_index = buffer_i + olength;
                                                        auto base = buffer.pointer + base_index;
                                                        memcpy(base - 1, DIGIT_TABLE + c0, 2);
                                                        memcpy(base - 3, DIGIT_TABLE + c1, 2);
                                                        memcpy(base - 5, DIGIT_TABLE + d0, 2);
                                                        memcpy(base - 7, DIGIT_TABLE + d1, 2);

                                                        i += 8;
                                                    }

                                                    auto output2 = (u32) output;

                                                    while (output2 >= 10000)
                                                    {
#ifdef __clang__
                                                        const u32 c = output2 - 10000 * (output2 / 10000);
#else
                                                        const uint32_t c = output2 % 10000;
#endif
                                                        output2 /= 10000;
                                                        const u32 c0 = (c % 100) << 1;
                                                        const u32 c1 = (c / 100) << 1;
                                                        auto base_index = buffer_i + olength - i;
                                                        memcpy(buffer.pointer + base_index - 1, DIGIT_TABLE + c0, 2);
                                                        memcpy(buffer.pointer + base_index - 3, DIGIT_TABLE + c1, 2);

                                                        i += 4;
                                                    }

                                                    if (output2 >= 100)
                                                    {
                                                        const u32 c = (output2 % 100) << 1;
                                                        output2 /= 100;
                                                        memcpy(buffer.pointer + buffer_i + olength - i - 1, DIGIT_TABLE + c, 2);
                                                        i += 2;
                                                    }

                                                    if (output2 >= 10)
                                                    {
                                                        const uint32_t c = output2 << 1;
                                                        // We can't use memcpy here: the decimal dot goes between these two digits.
                                                        buffer.pointer[buffer_i + olength - i] = DIGIT_TABLE[c + 1];
                                                        buffer.pointer[buffer_i] = DIGIT_TABLE[c];
                                                    }
                                                    else
                                                    {
                                                        buffer.pointer[buffer_i] = (u8)output2 + '0';
                                                    }

                                                    // Print decimal point if needed.
                                                    if (olength > 1)
                                                    {
                                                        buffer.pointer[buffer_i + 1] = '.';
                                                        buffer_i += olength + 1;
                                                    } else {
                                                        buffer_i += 1;
                                                    }

                                                    // Print the exponent.
                                                    buffer.pointer[buffer_i] = 'E';
                                                    buffer_i += 1;
                                                    int32_t exp = result.exponent + (int32_t) olength - 1;
                                                    if (exp < 0) {
                                                        buffer.pointer[buffer_i] = '-';
                                                        buffer_i += 1;
                                                        exp = -exp;
                                                    }

                                                    if (exp >= 100)
                                                    {
                                                        const int32_t c = exp % 10;
                                                        memcpy(buffer.pointer + buffer_i, DIGIT_TABLE + 2 * (exp / 10), 2);
                                                        buffer.pointer[buffer_i + 2] = (u8)c + '0';
                                                        buffer_i += 3;
                                                    }
                                                    else if (exp >= 10)
                                                    {
                                                        memcpy(buffer.pointer + buffer_i, DIGIT_TABLE + 2 * exp, 2);
                                                        buffer_i += 2;
                                                    }
                                                    else
                                                    {
                                                        buffer.pointer[buffer_i] = (u8)exp + '0';
                                                        buffer_i += 1;
                                                    }
                                                } break;
                                            case FLOAT_FORMAT_DECIMAL:
                                                {
                                                    auto dp_offset = result.exponent + cast(s32, u32, olength);

                                                    if (dp_offset <= 0)
                                                    {
                                                        buffer.pointer[buffer_i] = '0';
                                                        buffer.pointer[buffer_i + 1] = '.';
                                                        buffer_i += 2;

                                                        // auto dp_index = buffer_i;

                                                        auto dp_poffset = (u32)(-dp_offset);
                                                        memset(buffer.pointer + buffer_i, '0', dp_poffset);
                                                        buffer_i += dp_poffset;
                                                        write_float_decimal(s_get_slice(u8, buffer, buffer_i, buffer.length), &output, olength);
                                                        buffer_i += olength;
                                                    }
                                                    else
                                                    {
                                                        auto dp_uoffset = (u64)dp_offset;
                                                        if (dp_uoffset >= olength)
                                                        {
                                                            write_float_decimal(s_get_slice(u8, buffer, buffer_i, buffer.length), &output, olength);
                                                            buffer_i += olength;
                                                            auto length = dp_uoffset - olength;
                                                            auto memset_slice = s_get_slice(u8, buffer, buffer_i, buffer_i + length);
                                                            memset(memset_slice.pointer, 0, length);
                                                            buffer_i += length;
                                                        }
                                                        else
                                                        {
                                                            write_float_decimal(s_get_slice(u8, buffer, buffer_i + dp_uoffset + 1, buffer.length), &output, olength - dp_uoffset);
                                                            buffer.pointer[buffer_i + dp_uoffset] = '.';
                                                            // auto dp_index = buffer_i + dp_uoffset + 1;
                                                            write_float_decimal(s_get_slice(u8, buffer, buffer_i, buffer.length), &output, dp_uoffset);
                                                            buffer_i += olength + 1;
                                                        }
                                                    }
                                                } break;
                                        }
                                    }

                                }
                            } break;
                        case 's':
                            {
                                it += 1;

                                if (is_decimal_digit(*it))
                                {
                                    trap();
                                }
                                else
                                {
                                    String string = va_arg(args, String);
                                    memcpy(buffer.pointer + buffer_i, string.pointer, string.length);
                                    buffer_i += string.length;
                                }

                            } break;
                        case 'u':
                            {
                                it += 1;

                                u8* bit_count_start = it;
                                while (is_decimal_digit(*it))
                                {
                                    it += 1;
                                }

                                u8* bit_count_end = it;
                                u64 bit_count = parse_decimal(slice_from_pointer_range(u8, (u8*)bit_count_start, (u8*)bit_count_end));

                                typedef enum IntegerFormat : u8
                                {
                                    INTEGER_FORMAT_HEXADECIMAL,
                                    INTEGER_FORMAT_DECIMAL,
                                    INTEGER_FORMAT_OCTAL,
                                    INTEGER_FORMAT_BINARY,
                                } IntegerFormat;

                                IntegerFormat format = INTEGER_FORMAT_DECIMAL;

                                if (*it == ':')
                                {
                                    it += 1;
                                    switch (*it)
                                    {
                                        case 'x':
                                            format = INTEGER_FORMAT_HEXADECIMAL;
                                            break;
                                        case 'd':
                                            format = INTEGER_FORMAT_DECIMAL;
                                            break;
                                        case 'o':
                                            format = INTEGER_FORMAT_OCTAL;
                                            break;
                                        case 'b':
                                            format = INTEGER_FORMAT_BINARY;
                                            break;
                                        default:
                                            trap();
                                    }

                                    it += 1;
                                }

                                u64 original_value;
                                switch (bit_count)
                                {
                                    case 8:
                                    case 16:
                                    case 32:
                                        original_value = va_arg(args, u32);
                                        break;
                                    case 64:
                                        original_value = va_arg(args, u64);
                                        break;
                                    default:
                                        trap();
                                }

                                auto buffer_slice = s_get_slice(u8, buffer, buffer_i, buffer.length);

                                switch (format)
                                {
                                    case INTEGER_FORMAT_HEXADECIMAL:
                                        {
                                            auto written_characters = format_hexadecimal(buffer_slice, original_value);
                                            buffer_i += written_characters;
                                        } break;
                                    case INTEGER_FORMAT_DECIMAL:
                                        {
                                            auto written_characters = format_decimal(buffer_slice, original_value);
                                            buffer_i += written_characters;
                                        } break;
                                    case INTEGER_FORMAT_OCTAL:
                                    case INTEGER_FORMAT_BINARY:
                                        trap();
                                }
                            } break;
                        default:
                            buffer.pointer[buffer_i] = '{';
                            buffer_i += 1;
                            continue;
                    }

                    if (*it != brace_close)
                    {
                        fail();
                    }

                    it += 1;
                }
            }
        }

        String final_string = s_get_slice(u8, buffer, 0, buffer_i);
        os_file_write(stdout_get(), final_string);
#endif
}

global u64 minimum_granularity = page_size;
// global u64 middle_granularity = MB(2);
global u64 default_size = GB(4);

STRUCT(Arena)
{
    u64 reserved_size;
    u64 committed;
    u64 commit_position;
    u64 granularity;
    u8 reserved[4 * 8];
};

static_assert(sizeof(Arena) == 64);

fn Arena* arena_init(u64 reserved_size, u64 granularity, u64 initial_size)
{
    Arena* arena = (Arena*)os_reserve(0, reserved_size,
            (OSReserveProtectionFlags) {
                .read = 1,
                .write = 1,
            },
            (OSReserveMapFlags) {
                .priv = 1,
                .anon = 1,
                .noreserve = 1,
            });
    commit(arena, initial_size);
    *arena = (Arena){
        .reserved_size = reserved_size,
        .committed = initial_size,
        .commit_position = sizeof(Arena),
        .granularity = granularity,
    };
    return arena;
}

fn Arena* arena_init_default(u64 initial_size)
{
    return arena_init(default_size, minimum_granularity, initial_size);
}

fn u8* arena_allocate_bytes(Arena* arena, u64 size, u64 alignment)
{
    u64 aligned_offset = align_forward(arena->commit_position, alignment);
    u64 aligned_size_after = aligned_offset + size;

    if (aligned_size_after > arena->committed)
    {
        u64 committed_size = align_forward(aligned_size_after, arena->granularity);
        u64 size_to_commit = committed_size - arena->committed;
        void* commit_pointer = (u8*)arena + arena->committed;
        commit(commit_pointer, size_to_commit);
        arena->committed = committed_size;
    }

    auto* result = (u8*)arena + aligned_offset;
    arena->commit_position = aligned_size_after;
    assert(arena->commit_position <= arena->committed);
    return result;
}

fn String arena_join_string(Arena* arena, Slice(String) pieces)
{
    u64 size = 0;
    for (u64 i = 0; i < pieces.length; i += 1)
    {
        String piece = pieces.pointer[i];
        size += piece.length;
    }

    u8* pointer = arena_allocate_bytes(arena, size + 1, 1);
    auto* it = pointer;
    for (u64 i = 0; i < pieces.length; i += 1)
    {
        String piece = pieces.pointer[i];
        memcpy(it, piece.pointer, piece.length);
        it += piece.length;
    }
    assert((u64)(it - pointer) == size);
    *it = 0;

    return (String) { .pointer = pointer, .length = size };
}

#define arena_allocate(arena, T, count) (T*)(arena_allocate_bytes(arena, sizeof(T) * count, alignof(T)))
#define arena_allocate_slice(arena, T, count) (Slice(T)){ .pointer = arena_allocate(arena, T, count), .length = count }

may_be_unused fn void arena_reset(Arena* arena)
{
    arena->commit_position = sizeof(Arena);
    memset(arena + 1, 0, arena->committed - sizeof(Arena));
}

#define transmute(D, source) *(D*)&source

fn String file_read(Arena* arena, String path)
{
    String result = {};
    auto file_descriptor = os_file_open(path, (OSFileOpenFlags) {
        .truncate = 0,
        .executable = 0,
        .write = 0,
        .read = 1,
        .create = 0,
    });

    if (os_file_descriptor_is_valid(file_descriptor))
    {
        auto file_size = os_file_get_size(file_descriptor);
        if (file_size > 0)
        {
            result = (String){
                .pointer = arena_allocate_bytes(arena, file_size, 64),
                    .length = file_size,
            };

            // TODO: big files
            // TODO: result codes
            os_file_read(file_descriptor, result, file_size);
        }
        else
        {
            result.pointer = (u8*)&result;
        }

        // TODO: check result
        os_file_close(file_descriptor);
    }


    return result;
}


may_be_unused fn void run_command(Arena* arena, CStringSlice arguments, char* envp[])
{
    print("Running command:\n");
    assert(arguments.length > 0);
    assert(arguments.pointer[arguments.length - 1] == 0);
    for (u32 i = 0; i < arguments.length - 1; i += 1)
    {
        char* argument = arguments.pointer[i];
        print("{cstr} ", argument);
    }
    print("\n");

#if _WIN32
    auto start_timestamp = timestamp();

    u32 length = 0;
    for (u32 i = 0; i < arguments.length; i += 1)
    {
        auto argument = arguments.pointer[i];
        if (argument)
        {
            auto string_len = strlen(argument);
            length += cast(u32, u64, string_len + 1);
        }
    }

    char* bytes = (char*)arena_allocate_bytes(arena, length, 1);
    u32 byte_i = 0;
    for (u32 i = 0; i < arguments.length; i += 1)
    {
        auto argument = arguments.pointer[i];
        if (argument)
        {
            auto len = strlen(argument);
            memcpy(&bytes[byte_i], argument, len);
            byte_i += len;
            bytes[byte_i] = ' ';
            byte_i += 1;
        }
    }
    bytes[byte_i - 1] = 0;
    auto end_timestamp = timestamp();

    PROCESS_INFORMATION process_information = {};
    STARTUPINFOA startup_info = {};
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    auto handle_inheritance = 1;
    if (CreateProcessA(0, bytes, 0, 0, handle_inheritance, 0, 0, 0, &startup_info, &process_information))
    {
        WaitForSingleObject(process_information.hProcess, INFINITE);
        DWORD exit_code;
        if (GetExitCodeProcess(process_information.hProcess, &exit_code))
        {
            if (exit_code != 0)
            {
                fail();
            }
        }
        else
        {
            fail();
        }

        CloseHandle(process_information.hProcess);
        CloseHandle(process_information.hThread);
    }
    else
    {
        print("Failure\n");
        auto err = GetLastError();
        LPSTR lpMsgBuf;
        DWORD bufSize = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                err,
                LANG_NEUTRAL, // Use default language
                (LPSTR)&lpMsgBuf,
                0,
                NULL
                );
        unused(bufSize);
        todo();
    }
    unused(start_timestamp);
    unused(end_timestamp);
    unused(envp);
#else
    unused(arena);
    pid_t pid = syscall_fork();

    if (pid == -1)
    {
        trap();
    }

    auto start_timestamp = timestamp();

    if (pid == 0)
    {
        // close(pipes[0]);
        // fcntl(pipes[1], F_SETFD, FD_CLOEXEC);
        auto result = syscall_execve(arguments.pointer[0], arguments.pointer, envp);
#if LINK_LIBC
        print("Execve failed! Error: {cstr}\n", strerror(errno));
#else
        trap();
#endif
        unused(result);
        trap();
    }
    else
    {
        int status = 0;
        int options = 0;
        pid_t result = syscall_waitpid(pid, &status, options);
        auto end_timestamp = timestamp();
        int success = 0;
        if (result == pid)
        {
            if (WIFEXITED(status))
            {
                auto exit_code = WEXITSTATUS(status);
                success = exit_code == 0;
                print("Program exited with code {u32}\n", exit_code);
            }
            else if (WIFSIGNALED(status))
            {
                auto signal_code = WTERMSIG(status);
                print("Program was signaled: {u32}\n", signal_code);
            }
            else if (WIFSTOPPED(status))
            {
                auto stopped_code = WSTOPSIG(status);
                print("Program was stopped: {u32}\n", stopped_code);
            }
            else
            {
                print("Program terminated unexpectedly with status {u32}\n", status);
            }
        }
        else
        {
            trap();
        }

        if (!success)
        {
            print("Program failed to run!\n");
            fail();
        }
        auto ms = resolve_timestamp(start_timestamp, end_timestamp, TIME_UNIT_MILLISECONDS);
        auto ticks =
#if LINK_LIBC
            0
#else
            cpu_frequency != 0
#endif
            ;
        print("Command run successfully in {f64} {cstr}\n", ms, ticks ? "ticks" : "ms");
    }
#endif
}

#define VirtualBuffer(T) VirtualBuffer_ ## T
#define VirtualBufferP(T) VirtualBufferPointerTo_ ## T

#define decl_vb_ex(T, StructName) \
struct StructName \
{\
    T* pointer;\
    u32 length;\
    u32 capacity;\
};\
typedef struct StructName StructName

#define decl_vb(T) decl_vb_ex(T, VirtualBuffer(T))
#define decl_vbp(T) decl_vb_ex(T*, VirtualBufferP(T))

decl_vb(u8);
decl_vbp(u8);
decl_vb(s32);
decl_vb(u32);
decl_vb(String);

fn void vb_generic_ensure_capacity(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    u32 old_capacity = vb->capacity;
    u32 wanted_capacity = vb->length + item_count;

    if (old_capacity < wanted_capacity)
    {
        if (old_capacity == 0)
        {
            vb->pointer = os_reserve(0, item_size * UINT32_MAX, (OSReserveProtectionFlags) {}, (OSReserveMapFlags) { .priv = 1, .anon = 1, .noreserve = 1 });
        }

        u32 old_page_capacity = cast(u32, u64, align_forward(old_capacity * item_size, minimum_granularity));
        u32 new_page_capacity = cast(u32, u64, align_forward(wanted_capacity * item_size, minimum_granularity));

        u32 commit_size = new_page_capacity - old_page_capacity;
        void* commit_pointer = vb->pointer + old_page_capacity;

        commit(commit_pointer, commit_size);

        u32 new_capacity = new_page_capacity / item_size;
        vb->capacity = new_capacity;
    }
}

fn u8* vb_generic_add_assume_capacity(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    u32 index = vb->length;
    assert(vb->capacity >= index + item_count);
    vb->length = index + item_count;
    return vb->pointer + (index * item_size);
}

// fn u8* vb_generic_append_assume_capacity(VirtualBuffer(u8)* vb, void* item_pointer, u32 item_size, u32 item_count)
// {
//     u8* new_memory = vb_generic_add_assume_capacity(vb, item_size, item_count);
//     memcpy(new_memory, item_pointer, item_size * item_count);
//     return new_memory;
// }

fn u8* vb_generic_add(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    vb_generic_ensure_capacity(vb, item_size, item_count);
    return vb_generic_add_assume_capacity(vb, item_size, item_count);
}

may_be_unused fn u8* vb_append_bytes(VirtualBuffer(u8*) vb, Slice(u8) bytes)
{
    auto len = cast(u32, u64, bytes.length);
    vb_generic_ensure_capacity(vb, sizeof(u8), len);
    auto* pointer = vb_generic_add_assume_capacity(vb, sizeof(u8), len);
    memcpy(pointer, bytes.pointer, len);
    return pointer;
}

#define vb_size_of_element(vb) sizeof(*((vb)->pointer))
#define vb_add(vb, count) (typeof((vb)->pointer)) vb_generic_add((VirtualBuffer(u8)*)(vb), (vb_size_of_element(vb)), (count))
#define vb_add_scalar(vb, S) (S*) vb_generic_add(vb, 1, sizeof(S))
#define vb_copy_scalar(vb, s) *vb_add_scalar(vb, typeof(s)) = s
#define vb_append_struct(vb, T, s) *(vb_add_struct(vb, T)) = s
#define vb_append_one(vb, item) (typeof((vb)->pointer)) vb_generic_append((VirtualBuffer(u8)*)(vb), &(item), (vb_size_of_element(vb)), 1)
#define vb_to_bytes(vb) (Slice(u8)) { .pointer = (u8*)((vb).pointer), .length = (vb_size_of_element(vb)) * (vb).length, }
#define vb_ensure_capacity(vb, count) vb_generic_ensure_capacity((VirtualBuffer(u8)*)(vb), vb_size_of_element(vb), (count))
#define vb_copy_array(vb, arr) memcpy(vb_add(vb, array_length(arr)), arr, sizeof(arr))
#define vb_add_any_array(vb, E, count) (E*)vb_generic_add(vb, vb_size_of_element(vb), sizeof(E) * count)
#define vb_copy_any_array(vb, arr) memcpy(vb_generic_add(vb, vb_size_of_element(vb), sizeof(arr)), (arr), sizeof(arr))
#define vb_copy_any_slice(vb, slice) memcpy(vb_generic_add(vb, vb_size_of_element(vb), sizeof(*((slice).pointer)) * (slice).length), (slice).pointer, sizeof(*((slice).pointer)) * (slice).length)

fn void vb_copy_string(VirtualBuffer(u8)* buffer, String string)
{
    auto length = cast(u32, u64, string.length);
    auto* pointer = vb_add(buffer, length);
    memcpy(pointer, string.pointer, length);
}

fn u64 vb_copy_string_zero_terminated(VirtualBuffer(u8)* buffer, String string)
{
    assert(string.pointer[string.length] == 0);
    string.length += 1;

    vb_copy_string(buffer, string);

    return string.length;
}

may_be_unused fn Hash32 hash32_fib_end(Hash32 hash)
{
    auto result = truncate(Hash32, ((hash + 1) * 11400714819323198485ull) >> 32);
    return result;
}

may_be_unused fn Hash32 hash64_fib_end(Hash64 hash)
{
    auto result = truncate(Hash32, ((hash + 1) * 11400714819323198485ull) >> 32);
    return result;
}

STRUCT(MD5Context)
{
    u32 buffer[4];
    u8 input[64];
    u64 size;
};

// Took from: https://github.com/Zunawe/md5-c

#define MD5_A 0x67452301
#define MD5_B 0xefcdab89
#define MD5_C 0x98badcfe
#define MD5_D 0x10325476

#define MD5_F(X, Y, Z) ((X & Y) | (~X & Z))
#define MD5_G(X, Y, Z) ((X & Z) | (Y & ~Z))
#define MD5_H(X, Y, Z) (X ^ Y ^ Z)
#define MD5_I(X, Y, Z) (Y ^ (X | ~Z))

global u32 md5_s[] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                       5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

global u32 md5_k[] = {0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
                       0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                       0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
                       0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                       0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
                       0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                       0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
                       0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                       0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
                       0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                       0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
                       0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                       0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
                       0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                       0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
                       0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

/*
 * Padding used to make the size (in bits) of the input congruent to 448 mod 512
 */
global u8 md5_padding[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


may_be_unused fn MD5Context md5_init()
{
    return (MD5Context) {
        .buffer = { MD5_A, MD5_B, MD5_C, MD5_D },
    };
}

fn u32 rotate_left_u32(u32 x, u32 n)
{
    return (x << n) | (x >> (32 - n));
}

may_be_unused fn void md5_step(u32* buffer, u32* input)
{
    u32 aa = buffer[0];
    u32 bb = buffer[1];
    u32 cc = buffer[2];
    u32 dd = buffer[3];

    for (u32 i = 0; i < 64; i += 1)
    {
        u32 j;
        u32 e;
        switch (i / 16)
        {
            case 0:
                {
                    e = MD5_F(bb, cc, dd);
                    j = i;
                } break;
            case 1:
                {
                    e = MD5_G(bb, cc, dd);
                    j = ((i * 5) + 1) % 16;
                } break;
            case 2:
                {
                    e = MD5_H(bb, cc, dd);
                    j = ((i * 3) + 5) % 16;
                } break;
            default:
                {
                    e = MD5_I(bb, cc, dd);
                    j = (i * 7) % 16;
                } break;
        }

        u32 old_dd = dd;
        dd = cc;
        cc = bb;
        bb = bb + rotate_left_u32(aa + e + md5_k[i] + input[j], md5_s[i]);
        aa = old_dd;
    }

    buffer[0] += aa;
    buffer[1] += bb;
    buffer[2] += cc;
    buffer[3] += dd;
}

may_be_unused fn void md5_update(MD5Context* context, String input_argument)
{
    u32 input_local[16];
    auto offset = context->size % 64;
    context->size += input_argument.length;

    for (u64 i = 0; i < input_argument.length; i += 1)
    {
        context->input[offset] = input_argument.pointer[i];
        offset += 1;

        if (offset % 64 == 0)
        {
            // TODO: convert to little-endian in case we are big-endian?
            for (u16 i = 0; i < 16; i += 1)
            {
                auto existing = *(u32*)&input_argument.pointer[i * 4];
                input_local[i] = existing;
            }
            md5_step(context->buffer, input_local);
            offset = 0;
        }
    }
}

STRUCT(MD5Result)
{
    u8 hash[16];
};

may_be_unused fn MD5Result md5_end(MD5Context* context)
{
    u32 input[16];
    auto offset = context->size % 64;
    auto padding_length = offset < 56 ? 56 - offset : (56 + 64) - offset;
    
    md5_update(context, (String) { .pointer = md5_padding, .length = padding_length });
    context->size -= (u64)padding_length;

    for (u32 i = 0; i < 14; i += 1)
    {
        input[i] = *(u32*)&context->input[i * 4];
    }
    input[14] = (u32)(context->size * 8);
    input[15] = (u32)((context->size * 8) >> 32);

    md5_step(context->buffer, input);

    MD5Result result;
    for (u32 i = 0; i < 4; i += 1)
    {
        result.hash[(i * 4) + 0] = (u8)((context->buffer[i] & 0x000000ff) >> 0);
        result.hash[(i * 4) + 1] = (u8)((context->buffer[i] & 0x0000ff00) >> 8);
        result.hash[(i * 4) + 2] = (u8)((context->buffer[i] & 0x00ff0000) >> 16);
        result.hash[(i * 4) + 3] = (u8)((context->buffer[i] & 0xff000000) >> 24);
    }

    return result;
}

// https://github.com/jasinb/sha1.git
// STRUCT(Sha1Digest)
// {
//     u32 digest[5];
// };

// static uint32_t rotl32(uint32_t x, int b)
// {
//     return (x << b) | (x >> (32-b));
// }
//
// switch endianness
// fn u32 sha1_get32(u8* p)
// {
//     return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
// }

// fn u32 sha1_f(int t, u32 b, u32 c, u32 d)
// {
//     assert(0 <= t && t < 80);
//
//     if (t < 20)
//     {
//         return (b & c) | ((~b) & d);
//     }
//     else if (t < 40)
//     {
//         return b ^ c ^ d;
//     }
//     else if (t < 60)
//     {
//         return (b & c) | (b & d) | (c & d);
//     }
//     else
//     //if (t < 80)
//     {
//         return b ^ c ^ d;
//     }
// }

// STRUCT(Sha1Context)
// {
//     u8 block[64];
//     u32 h[5];
//     u64 bytes;
//     u32 cur;
// };

// fn void sha1_reset(Sha1Context* ctx)
// {
//     ctx->h[0] = 0x67452301;
//     ctx->h[1] = 0xefcdab89;
//     ctx->h[2] = 0x98badcfe;
//     ctx->h[3] = 0x10325476;
//     ctx->h[4] = 0xc3d2e1f0;
//     ctx->bytes = 0;
//     ctx->cur = 0;
// }

// fn void sha1_process_block(Sha1Context* ctx)
// {
//     global const u32 k[4] =
//     {
//         0x5A827999,
//         0x6ED9EBA1,
//         0x8F1BBCDC,
//         0xCA62C1D6
//     };
//
//     u32 w[16];
//     u32 a = ctx->h[0];
//     u32 b = ctx->h[1];
//     u32 c = ctx->h[2];
//     u32 d = ctx->h[3];
//     u32 e = ctx->h[4];
//     u32 t;
//
//     for (t = 0; t < 16; t++)
//         w[t] = sha1_get32((u8*)(&((uint32_t*)ctx->block)[t]));
//
//     for (t = 0; t < 80; t++)
//     {
//         auto s = t & 0xf;
//         u32 temp;
//         if (t >= 16)
//             w[s] = rotate_left_u32(w[(s + 13) & 0xf] ^ w[(s + 8) & 0xf] ^ w[(s + 2) & 0xf] ^ w[s], 1);
//
//          temp = rotate_left_u32(a, 5) + sha1_f(t, b,c,d) + e + w[s] + k[t/20];
//
//          e = d; d = c; c = rotate_left_u32(b, 30); b = a; a = temp;
//     }
//
//     ctx->h[0] += a;
//     ctx->h[1] += b;
//     ctx->h[2] += c;
//     ctx->h[3] += d;
//     ctx->h[4] += e;
// }

// fn void sha1_write(Sha1Context* ctx, String bytes)
// {
//     auto length = bytes.length;
//     ctx->bytes += length;
//
//     const uint8_t* src = bytes.pointer;
//     while (length--)
//     {
//         // TODO: could optimize the first and last few bytes, and then copy
//         // 128 bit blocks with SIMD in between
//         ctx->block[ctx->cur++] = *src++;
//         if (ctx->cur == 64)
//         {
//             sha1_process_block(ctx);
//             ctx->cur = 0;
//         }
//     }
// }

// fn Sha1Digest sha1_get_digest(Sha1Context* ctx)
// {
//     // append separator
//     ctx->block[ctx->cur++] = 0x80;
//     if (ctx->cur > 56)
//     {
//         // no space in block for the 64-bit message length, flush
//         memset(&ctx->block[ctx->cur], 0, 64 - ctx->cur);
//         sha1_process_block(ctx);
//         ctx->cur = 0;
//     }
//
//     memset(&ctx->block[ctx->cur], 0, 56 - ctx->cur);
//     uint64_t bits = ctx->bytes * 8;
//
//     // TODO a few instructions could be shaven
//     ctx->block[56] = (uint8_t)(bits >> 56 & 0xff);
//     ctx->block[57] = (uint8_t)(bits >> 48 & 0xff);
//     ctx->block[58] = (uint8_t)(bits >> 40 & 0xff);
//     ctx->block[59] = (uint8_t)(bits >> 32 & 0xff);
//     ctx->block[60] = (uint8_t)(bits >> 24 & 0xff);
//     ctx->block[61] = (uint8_t)(bits >> 16 & 0xff);
//     ctx->block[62] = (uint8_t)(bits >> 8  & 0xff);
//     ctx->block[63] = (uint8_t)(bits >> 0  & 0xff);
//     sha1_process_block(ctx);
//
//     {
//         Sha1Digest ret;
//         int i;
//         for (i = 0; i < 5; i++)
//             ret.digest[i] = sha1_get32((u8*)&ctx->h[i]);
//         sha1_reset(ctx);
//         return ret;
//     }
// }

// fn Sha1Digest sha1_compute(String bytes)
// {
//     Sha1Context ctx;
//     sha1_reset(&ctx);
//     sha1_write(&ctx, bytes);
//     return sha1_get_digest(&ctx);
// }

fn void entry_point(int argc, char* argv[], char* envp[]);

#if LINK_LIBC
int main(int argc, char* argv[], char* envp[])
{
    entry_point(argc, argv, envp);
    return 0;
}
#else
[[gnu::naked]] [[noreturn]] void _start()
{
    __asm__ __volatile__(
            "\nxor %ebp, %ebp"
            "\npopq %rdi"
            "\nmov %rsp, %rsi"
            "\nand $~0xf, %rsp"
            "\npushq %rsp"
            "\npushq $0"
            "\ncallq static_entry_point"
            "\nud2\n"
       );
}

void static_entry_point(int argc, char* argv[])
{
    char** envp = (char**)&argv[argc + 1];
    calibrate_cpu_timer();
    entry_point(argc, argv, envp);
    syscall_exit(0);
}

#endif
