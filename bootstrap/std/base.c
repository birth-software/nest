#include <std/base.h>
#include <std/os.h>

u8 cast_u32_to_u8(u32 source, const char* name, int line)
{
#if BB_DEBUG
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

u16 cast_u32_to_u16(u32 source, const char* name, int line)
{
#if BB_DEBUG
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

s16 cast_u32_to_s16(u32 source, const char* name, int line)
{
#if BB_DEBUG
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

s32 cast_u32_to_s32(u32 source, const char* name, int line)
{
#if BB_DEBUG
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

u8 cast_u64_to_u8(u64 source, const char* name, int line)
{
#if BB_DEBUG
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

u16 cast_u64_to_u16(u64 source, const char* name, int line)
{
#if BB_DEBUG
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

u32 cast_u64_to_u32(u64 source, const char* name, int line)
{
#if BB_DEBUG
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

s32 cast_u64_to_s32(u64 source, const char* name, int line)
{
#if BB_DEBUG
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

s64 cast_u64_to_s64(u64 source, const char* name, int line)
{
#if BB_DEBUG
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

u8 cast_s32_to_u8(s32 source, const char* name, int line)
{
#if BB_DEBUG
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

u16 cast_s32_to_u16(s32 source, const char* name, int line)
{
#if BB_DEBUG
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

u32 cast_s32_to_u32(s32 source, const char* name, int line)
{
#if BB_DEBUG
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

u64 cast_s32_to_u64(s32 source, const char* name, int line)
{
#if BB_DEBUG
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

s16 cast_s32_to_s16(s32 source, const char* name, int line)
{
#if BB_DEBUG
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

u16 cast_s64_to_u16(s64 source, const char* name, int line)
{
#if BB_DEBUG
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

u32 cast_s64_to_u32(s64 source, const char* name, int line)
{
#if BB_DEBUG
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

u64 cast_s64_to_u64(s64 source, const char* name, int line)
{
#if BB_DEBUG
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

s32 cast_s64_to_s32(s64 source, const char* name, int line)
{
#if BB_DEBUG
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

u8 log2_alignment(u64 alignment)
{
    assert(alignment != 0);
    assert((alignment & (alignment - 1)) == 0);
    u64 left = (sizeof(alignment) * 8) - 1;
    auto right = cast_to(u64, s32, __builtin_clzll(alignment));
    auto result = cast_to(u8, u64, left - right);
    return result;
}

// Lehmer's generator
// https://lemire.me/blog/2019/03/19/the-fastest-conventional-random-number-generator-that-can-pass-big-crush/
may_be_unused global_variable u128 rn_state;
may_be_unused fn u64 generate_random_number()
{
    rn_state *= 0xda942042e4dd58b5;
    return rn_state >> 64;
}

u64 round_up_to_next_power_of_2(u64 n)
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
    return n < 0 ? cast_to(u64, s64, -n) : cast_to(u64, s64, n);
}

u64 parse_decimal(String string)
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

u8 get_next_ch_safe(String string, u64 index)
{
    u64 next_index = index + 1;
    u64 is_in_range = next_index < string.length;
    u64 safe_index = safe_flag(next_index, is_in_range);
    u8 unsafe_result = string.pointer[safe_index];
    u64 safe_result = safe_flag(unsafe_result, is_in_range);
    assert(safe_result < 256);
    return (u8)safe_result;
}

u32 is_space(u8 ch, u8 next_ch)
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

u64 is_alphabetic(u8 ch)
{
    return is_lower(ch) | is_upper(ch);
}

u64 is_decimal_digit(u8 ch)
{
    return (ch >= '0') & (ch <= '9');
}

fn u64 is_hex_digit(u8 ch)
{
    return (is_decimal_digit(ch) | ((ch == 'a' | ch == 'A') | (ch == 'b' | ch == 'B'))) | (((ch == 'c' | ch == 'C') | (ch == 'd' | ch == 'D')) | ((ch == 'e' | ch == 'E') | (ch == 'f' | ch == 'F')));
}


u64 is_identifier_start(u8 ch)
{
    u64 alphabetic = is_alphabetic(ch);
    u64 is_underscore = ch == '_';
    return alphabetic | is_underscore;
}

u64 is_identifier_ch(u8 ch)
{
    u64 identifier_start = is_identifier_start(ch);
    u64 decimal = is_decimal_digit(ch);
    return identifier_start | decimal;
}

Hash64 hash_byte(Hash64 source, u8 ch)
{
    source ^= ch;
    source *= fnv_prime;
    return source;
}

Hash64 hash_bytes(String bytes)
{
    u64 result = fnv_offset;
    for (u64 i = 0; i < bytes.length; i += 1)
    {
        result = hash_byte(result, bytes.pointer[i]);
    }

    return result;
}

Hash32 hash64_to_hash32(Hash64 hash64)
{
    Hash32 low = hash64 & 0xffff;
    Hash32 high = (hash64 >> 32) & 0xffff;
    Hash32 result = (high << 16) | low;
    return result;
}

u64 align_forward(u64 value, u64 alignment)
{
    u64 mask = alignment - 1;
    u64 result = (value + mask) & ~mask;
    return result;
}

u64 align_backward(u64 value, u64 alignment)
{
    u64 result = value & ~(alignment - 1);
    return result;
}

u8 is_power_of_two(u64 value)
{
    return (value & (value - 1)) == 0;
}

u8 first_bit_set_32(u32 value)
{
    auto result = (u8)__builtin_ffs((s32)value);
    result -= result != 0;
    return result;
}

u64 first_bit_set_64(u64 value)
{
    auto result = (u8) __builtin_ffs((s64)value);
    result -= result != 0;
    return result;
}

Hash32 hash32_fib_end(Hash32 hash)
{
    auto result = truncate_value(Hash32, ((hash + 1) * 11400714819323198485ull) >> 32);
    return result;
}

Hash32 hash64_fib_end(Hash64 hash)
{
    auto result = truncate_value(Hash32, ((hash + 1) * 11400714819323198485ull) >> 32);
    return result;
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
void* memcpy(void* const restrict dst, const void* const restrict src, usize size)
{
    auto* destination = (u8*)dst;
    auto* source = (u8*)src;

    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = source[i];
    }

    return dst;
}

void* memmove(void* const dst, const void* const src, usize n)
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

void* memset(void* dst, int n, usize size)
{
    u8 ch = cast_to(u8, s32, n);
    auto* destination = (u8*)dst;
    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = ch;
    }

    return dst;
}

int memcmp(const void* a, const void* b, usize n)
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

usize strlen(const char* c_string)
{
    auto* it = c_string;
    while (*it)
    {
        it += 1;
    }
    return (u64)(it - c_string);
}
#endif
