#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

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
#endif

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

typedef u32 Hash32;
typedef u64 Hash64;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define fn static
#define method __attribute__((visibility("internal")))
#define global static
#define forceinline __attribute__((always_inline))
#define expect(x, b) __builtin_expect(x, b)
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

#if _DEBUG
#define assert(x) if (__builtin_expect(!(x), 0)) { bad_exit("Assert failed", __FILE__, __LINE__); }
#else
#define assert(x) __builtin_expect(!(x), 0)
#endif

#ifdef unreachable
#undef unreachable
#endif

#if _DEBUG
#define unreachable() bad_exit("Unreachable triggered", __FILE__, __LINE__)
#else
#define unreachable() __builtin_unreachable()
#endif

#define static_assert(x) _Static_assert((x), "Static assert failed!")
#define alignof(x) _Alignof(x)
#define auto __auto_type

#define bad_exit(message, file, line) do { print(message " at {cstr}:{u32}\n", file, line); __builtin_trap(); } while(0)
#define todo() do { print("TODO at {cstr}:{u32}\n", __FILE__, __LINE__); __builtin_trap(); } while(0)

may_be_unused fn void print(const char* format, ...);

#if __APPLE__
    const global u64 page_size = KB(16);
#else
    const global u64 page_size = KB(4);
#endif


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
    u64 right = __builtin_clzl(alignment);
    u8 result = left - right;
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

#if LINK_LIBC == 0
void* memcpy(void* __restrict dst, void* __restrict src, u64 size)
{
    auto* destination = (u8*)dst;
    auto* source = (u8*)src;

    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = source[i];
    }

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

fn int memcmp(const void* left, const void* right, u64 n)
{
	const u8 *l=(const u8*)left, *r=(const u8*)right;
	for (; n && *l == *r; n--, l++, r++);
	return n ? *l - *r : 0;
}

fn u64 strlen (const char* c_string)
{
    auto* it = c_string;
    while (*it)
    {
        it += 1;
    }
    return it - c_string;
}
#endif
#define slice_from_pointer_range(T, start, end) (Slice(T)) { .pointer = start, .length = (u64)(end - start), }

#define strlit_len(s) (sizeof(s) - 1)
#define strlit(s) (String){ .pointer = (u8*)(s), .length = strlit_len(s), }
#define ch_to_str(ch) (String){ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = (arr), .length = array_length(arr) }
#define pointer_to_bytes(p) (String) { .pointer = (u8*)(p), .length = sizeof(*p) }
#define struct_to_bytes(s) pointer_to_bytes(&(s))
#define string_to_c(s) ((char*)((s).pointer))
#define cstr(s) ((String) { .pointer = (u8*)(s), .length = strlen(s), } )

#define case_to_name(prefix, e) case prefix ## e: return strlit(#e)

#define Slice(T) Slice_ ## T
#define SliceP(T) SliceP_ ## T
#define declare_slice_ex(T, StructName) struct StructName  \
{\
    T* pointer;\
    u64 length;\
};\
typedef struct StructName StructName

#define declare_slice(T) declare_slice_ex(T, Slice(T))
#define declare_slice_p(T) declare_slice_ex(T*, SliceP(T))

#define s_get(s, i) (s).pointer[i]
#define s_get_pointer(s, i) &((s).pointer[i])
#define s_get_slice(T, s, start, end) (Slice(T)){ .pointer = ((s).pointer) + (start), .length = (end) - (start) }
#define s_equal(a, b) ((a).length == (b).length && memcmp((a).pointer, (b).pointer, sizeof(*((a).pointer))) == 0)

declare_slice(u8);
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

fn s32 string_last_ch(String string, u8 ch)
{
    s32 result = -1;
    u64 i = string.length;
    while (i > 0)
    {
        i -= 1;
        if (string.pointer[i] == ch)
        {
            result = i;
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
    auto index = string_last_ch(string, '/');
    if (index != -1)
    {
        result = s_get_slice(u8, string, index + 1, string.length);
    }

    return result;
}

may_be_unused fn String string_no_extension(String string)
{
    String result = {};
    auto index = string_last_ch(string, '.');
    if (index != -1)
    {
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
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n) : "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall1(long n, long a1)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1) : "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall2(long n, long a1, long a2)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2)
						  : "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall3(long n, long a1, long a2, long a3)
{
	unsigned long ret;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3) : "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall4(long n, long a1, long a2, long a3, long a4)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10): "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	unsigned long ret;
	register long r10 __asm__("r10") = a4;
	register long r8 __asm__("r8") = a5;
	__asm__ __volatile__ ("syscall" : "=a"(ret) : "a"(n), "D"(a1), "S"(a2),
						  "d"(a3), "r"(r10), "r"(r8) : "rcx", "r11", "memory");
	return ret;
}

may_be_unused fn forceinline long syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	unsigned long ret;
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

may_be_unused fn void* syscall_mmap(void* address, size_t length, int protection_flags, int map_flags, int fd, signed long offset)
{
#if LINK_LIBC
    return mmap(address, length, protection_flags, map_flags, fd, offset);
#else 
#ifdef __linux__
    return (void*) syscall6(syscall_x86_64_mmap, (unsigned long)address, length, protection_flags, map_flags, fd, offset);
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
    return syscall3(syscall_x86_64_mprotect, (unsigned long)address, length, protection_flags);
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
    return syscall3(syscall_x86_64_open, (unsigned long)file_path, flags, mode);
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
    return syscall1(syscall_x86_64_close, fd);
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
    return syscall2(syscall_x86_64_fstat, fd, (unsigned long)buffer);
#else
    return fstat(fd, buffer);
#endif
#endif
}

may_be_unused fn u64 file_get_size(int fd)
{
    struct stat stat_buffer;
    int stat_result = syscall_fstat(fd, &stat_buffer);
    assert(stat_result == 0);
    u64 size = stat_buffer.st_size;
    return size;
}

may_be_unused fn ssize_t syscall_read(int fd, void* buffer, size_t bytes)
{
#if LINK_LIBC
    return read(fd, buffer, bytes);
#else
#ifdef __linux__
    return syscall3(syscall_x86_64_read, fd, (unsigned long)buffer, bytes);
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
    return syscall3(syscall_x86_64_write, fd, (unsigned long)buffer, bytes);
#else
    return write(fd, buffer, bytes);
#endif
#endif
}

may_be_unused fn int syscall_mkdir(const char* path, u32 mode)
{
#if LINK_LIBC
    return mkdir(path, mode);
#else
    return syscall2(syscall_x86_64_mkdir, (unsigned long)path, mode);
#endif
}

may_be_unused fn int syscall_rmdir(const char* path)
{
#if LINK_LIBC
    return rmdir(path);
#else
    return syscall1(syscall_x86_64_rmdir, (unsigned long)path);
#endif
}

may_be_unused fn int syscall_unlink(const char* path)
{
#if LINK_LIBC
    return unlink(path);
#else
    return syscall1(syscall_x86_64_unlink, (unsigned long)path);
#endif
}

may_be_unused fn pid_t syscall_fork()
{
#if LINK_LIBC
    return fork();
#else
    return syscall0(syscall_x86_64_fork);
#endif

}

may_be_unused fn signed long syscall_execve(const char* path, char *const argv[], char *const envp[])
{
#if LINK_LIBC
    return execve(path, argv, envp);
#else
    return syscall3(syscall_x86_64_execve, (unsigned long)path, (unsigned long)argv, (unsigned long)envp);
#endif
}
may_be_unused fn pid_t syscall_waitpid(pid_t pid, int* status, int options)
{
#if LINK_LIBC
    return waitpid(pid, status, options);
#else
    return syscall4(syscall_x86_64_wait4, pid, (unsigned long)status, options, 0);
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

fn u8* reserve(u64 size)
{
    int protection_flags = PROT_NONE;
    int map_flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;
    u8* result = (u8*)syscall_mmap(0, size, protection_flags, map_flags, -1, 0);
    assert(result != MAP_FAILED);
    return result;
}

fn void commit(void* address, u64 size)
{
    int result = syscall_mprotect(address, size, PROT_READ | PROT_WRITE);
    assert(result == 0);
}

fn u64 align_forward(u64 value, u64 alignment)
{
    u64 mask = alignment - 1;
    u64 result = (value + mask) & ~mask;
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

#define SILENT (0)

may_be_unused fn void print(const char* format, ...)
{
#if SILENT == 0
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
                char next_ch = *it;

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
                                            auto* cstring = va_arg(args, const char*);
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
        syscall_write(1, final_string.pointer, final_string.length);
#endif
}

global u64 minimum_granularity = page_size;
// global u64 middle_granularity = MB(2);
global u64 default_size = GB(4);

struct Arena
{
    u64 reserved_size;
    u64 committed;
    u64 commit_position;
    u64 granularity;
    u8 reserved[4 * 8];
};

typedef struct Arena Arena;
static_assert(sizeof(Arena) == 64);

fn Arena* arena_init(u64 reserved_size, u64 granularity, u64 initial_size)
{
    Arena* arena = (Arena*)reserve(reserved_size);
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

fn void run_command(CStringSlice arguments, char* envp[])
{
    print("Running command:\n");
    assert(arguments.pointer[arguments.length - 1] == 0);
    for (u32 i = 0; i < arguments.length - 1; i += 1)
    {
        char* argument = arguments.pointer[i];
        print("{cstr} ", argument);
    }
    print("\n");
    pid_t pid = syscall_fork();

    if (pid == -1)
    {
        trap();
    }

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
        int success = 0;
        if (result == pid)
        {
            if (WIFEXITED(status))
            {
                auto exit_code = WEXITSTATUS(status);
                if (exit_code == 0)
                {
                    success = 1;
                }
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
    }
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

fn void vb_generic_ensure_capacity(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    u32 old_capacity = vb->capacity;
    u32 wanted_capacity = vb->length + item_count;

    if (old_capacity < wanted_capacity)
    {
        if (old_capacity == 0)
        {
            vb->pointer = reserve(item_size * UINT32_MAX);
        }

        u32 old_page_capacity = align_forward(old_capacity * item_size, minimum_granularity);
        u32 new_page_capacity = align_forward(wanted_capacity * item_size, minimum_granularity);

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
    vb_generic_ensure_capacity(vb, sizeof(u8), bytes.length);
    auto* pointer = vb_generic_add_assume_capacity(vb, sizeof(u8), bytes.length);
    memcpy(pointer, bytes.pointer, bytes.length);
    return pointer;
}

// fn u8* vb_generic_append(VirtualBuffer(u8)* vb, void* item_pointer, u32 item_size, u32 item_count)
// {
//     vb_generic_ensure_capacity(vb, item_size, item_count);
//     return vb_generic_append_assume_capacity(vb, item_pointer, item_size, item_count);
// }

#define vb_add(a, count) (typeof((a)->pointer)) vb_generic_add((VirtualBuffer(u8)*)(a), sizeof(*((a)->pointer)), count)
#define vb_append_one(a, item) (typeof((a)->pointer)) vb_generic_append((VirtualBuffer(u8)*)(a), &(item), sizeof(*((a)->pointer)), 1)
#define vb_to_bytes(vb) (Slice(u8)) { .pointer = (u8*)((vb).pointer), .length = sizeof(*((vb).pointer)) * (vb).length, }
