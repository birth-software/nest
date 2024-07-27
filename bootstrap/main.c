#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>

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

typedef u64 Hash;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define fn static
#define method __attribute__((visibility("internal")))
#define global static
#define forceinline __attribute__((always_inline))
#define expect(x, b) __builtin_expect(x, b)
#define breakpoint() __builtin_debugtrap()
#define fail() trap()
#define trap() bad_ex(__FILE__, __LINE__)
#define array_length(arr) sizeof(arr) / sizeof((arr)[0])
#define KB(n) ((n) * 1024)
#define MB(n) ((n) * 1024 * 1024)
#define GB(n) ((u64)(n) * 1024 * 1024 * 1024)
#define TB(n) ((u64)(n) * 1024 * 1024 * 1024 * 1024)
#define unused(x) (void)(x)
#define may_be_unused __attribute__((unused))
#if LINK_LIBC
#define assert(x) if (__builtin_expect(!(x), 0)) { print("Assert failed! ERRNO: "); print(strerror(errno)); print("\n"); trap(); }
#else
#define assert(x) if (__builtin_expect(!(x), 0)) { print("Assert failed!\n"); trap(); }
#endif
#define static_assert(x) _Static_assert((x), "Static assert failed!")
#define alignof(x) _Alignof(x)
#define auto __auto_type

#define bad_ex(file, line) do { print("Bad exit at {cstr}:{u32}\n", file, line); __builtin_trap(); } while(0)
#define todo() do { print("TODO at {cstr}:{u32}\n", __FILE__, __LINE__); __builtin_trap(); } while(0)

may_be_unused fn void print(const char* format, ...);

#if __APPLE__
    const global u64 page_size = KB(16);
#else
    const global u64 page_size = KB(4);
#endif


const global u8 brace_open = '{';
const global u8 brace_close = '}';

const global u8 parenthesis_open = '(';
const global u8 parenthesis_close = ')';

const global u8 bracket_open = '[';
const global u8 bracket_close = ']';

fn u8 log2_alignment(u64 alignment)
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
__uint128_t rn_state;
fn u64 generate_random_number()
{
    rn_state *= 0xda942042e4dd58b5;
    return rn_state >> 64;
}

fn u64 round_up_to_next_power_of_2(u64 n)
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

#define strlit(s) (String){ .pointer = (u8*)s, .length = sizeof(s) - 1, }
#define ch_to_str(ch) (String){ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = (arr), .length = array_length(arr) }
#define pointer_to_bytes(p) (String) { .pointer = (u8*)(p), .length = sizeof(*p) }
#define struct_to_bytes(s) pointer_to_bytes(&(s))

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
#define s_get_slice(T, s, start, end) (Slice(T)){ .pointer = s.pointer + start, .length = end - start }
#define s_equal(a, b) ((a).length == (b).length && memcmp((a).pointer, (b).pointer, sizeof(*((a).pointer))) == 0)

declare_slice(u8);
typedef Slice(u8) String;
// Array of strings
declare_slice(String);

fn s32 string_first_ch(String string, u8 ch)
{
    s32 result = -1;
    for (u64 i = 0; i < string.length; i += 1)
    {
        if (string.pointer[i] == ch)
        {
            result = i;
            break;
        }
    }

    return result;
}

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

fn String string_dir(String string)
{
    String result = {};
    auto index = string_last_ch(string, '/');
    if (index != -1)
    {
        result = s_get_slice(u8, string, 0, index);
    }

    return result;
}

fn String string_base(String string)
{
    String result = {};
    auto index = string_last_ch(string, '/');
    if (index != -1)
    {
        result = s_get_slice(u8, string, index + 1, string.length - 2);
    }

    return result;
}

fn String string_no_extension(String string)
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

fn u8 get_next_ch_safe(String string, u64 index)
{
    u64 next_index = index + 1;
    u64 is_in_range = next_index < string.length;
    u64 safe_index = safe_flag(next_index, is_in_range);
    u8 unsafe_result = string.pointer[safe_index];
    u64 safe_result = safe_flag(unsafe_result, is_in_range);
    assert(safe_result < 256);
    return (u8)safe_result;
}

fn u32 is_space(u8 ch, u8 next_ch)
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

fn u64 is_hex_digit(u8 ch)
{
    return (is_decimal_digit(ch) | ((ch == 'a' | ch == 'A') | (ch == 'b' | ch == 'B'))) | (((ch == 'c' | ch == 'C') | (ch == 'd' | ch == 'D')) | ((ch == 'e' | ch == 'E') | (ch == 'f' | ch == 'F')));
}


fn u64 is_identifier_start(u8 ch)
{
    u64 alphabetic = is_alphabetic(ch);
    u64 is_underscore = ch == '_';
    return alphabetic | is_underscore;
}

fn u64 is_identifier_ch(u8 ch)
{
    u64 identifier_start = is_identifier_start(ch);
    u64 decimal = is_decimal_digit(ch);
    return identifier_start | decimal;
}

global const Hash fnv_offset = 14695981039346656037ull;
global const u64 fnv_prime = 1099511628211ull;

fn Hash hash_byte(Hash source, u8 ch)
{
    source ^= ch;
    source *= fnv_prime;
    return source;
}

fn Hash hash_bytes(String bytes)
{
    u64 result = fnv_offset;
    for (u64 i = 0; i < bytes.length; i += 1)
    {
        result = hash_byte(result, bytes.pointer[i]);
    }

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

fn void* syscall_mmap(void* address, size_t length, int protection_flags, int map_flags, int fd, signed long offset)
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

fn int syscall_mprotect(void *address, size_t length, int protection_flags)
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

fn int syscall_open(const char *file_path, int flags, int mode)
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

fn int syscall_close(int fd)
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

fn ssize_t syscall_read(int fd, void* buffer, size_t bytes)
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

may_be_unused fn int syscall_execve(const char* path, char *const argv[], char *const envp[])
{
#if LINK_LIBC
    return execve(path, argv, envp);
#else
    return syscall3(syscall_x86_64_execve, (unsigned long)path, (unsigned long)argv, (unsigned long)envp);
#endif
}

[[noreturn]] [[gnu::cold]] fn void syscall_exit(int status)
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

fn void* reserve(u64 size)
{
    int protection_flags = PROT_NONE;
    int map_flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_NORESERVE;
    void* result = syscall_mmap(0, size, protection_flags, map_flags, -1, 0);
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
global u64 middle_granularity = MB(2);
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

fn void* arena_allocate_bytes(Arena* arena, u64 size, u64 alignment)
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

    void* result = (u8*)arena + aligned_offset;
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

fn void arena_reset(Arena* arena)
{
    arena->commit_position = sizeof(Arena);
    memset(arena + 1, 0, arena->committed - sizeof(Arena));
}

#define transmute(D, source) *(D*)&source

struct StringMapValue
{
    String string;
    u32 value;
};
typedef struct StringMapValue StringMapValue;

struct StringMap
{
    u32* pointer;
    u32 length;
    u32 capacity;
};
typedef struct StringMap StringMap;

fn s32 string_map_find_slot(StringMap* map, u32 original_index, String key, u32 value)
{
    auto it_index = original_index;
    auto existing_capacity = map->capacity;
    s32 result = -1;

    for (u32 i = 0; i < existing_capacity; i += 1)
    {
        auto index = it_index & (existing_capacity - 1);
        u32 existing_key = map->pointer[index];

        // Not set
        if (existing_key == 0)
        {
            result = index;
            break;
        }
        else
        {
            unused(value);
            unused(key);
            trap();
        }

        it_index += 1;
    }

    return result;
}

struct StringMapPut
{
    u32 value;
    u8 existing;
};
typedef struct StringMapPut StringMapPut;

fn StringMapValue* string_map_values(StringMap* map)
{
    assert(map->pointer);
    return (StringMapValue*)(map->pointer + map->capacity);
}

fn void string_map_ensure_capacity(StringMap* map, Arena* arena, u32 additional)
{
    auto current_capacity = map->capacity;
    auto half_capacity = current_capacity >> 1;
    auto destination_length = map->length + additional;

    if (destination_length > half_capacity)
    {
        u32 new_capacity = MAX(round_up_to_next_power_of_2(destination_length), 32);
        auto new_capacity_bytes = sizeof(u32) * new_capacity + new_capacity * sizeof(StringMapValue);

        void* ptr = arena_allocate_bytes(arena, new_capacity_bytes, MAX(alignof(u32), alignof(StringMapValue)));
        memset(ptr, 0, new_capacity_bytes);

        auto* keys = (u32*)ptr;
        auto* values = (StringMapValue*)(keys + new_capacity);

        auto* old_keys = map->pointer;
        auto old_capacity = map->capacity;
        auto* old_values = (StringMapValue*)(map->pointer + current_capacity);

        map->length = 0;
        map->pointer = keys;
        map->capacity = new_capacity;

        for (u32 i = 0; i < old_capacity; i += 1)
        {
            auto key = old_keys[i];
            if (key)
            {
                trap();
            }
        }

        for (u32 i = 0; i < old_capacity; i += 1)
        {
            trap();
        }
    }
}

fn StringMapPut string_map_put_at_assume_not_existent_assume_capacity(StringMap* map, u32 hash, String key, u32 value, u32 index)
{
    u32 existing_hash = map->pointer[index];
    map->pointer[index] = hash;
    auto* values = string_map_values(map);
    auto existing_value = values[index];
    values[index] = (StringMapValue) {
        .value = value,
        .string = key,
    };
    map->length += 1;
    assert(existing_hash ? s_equal(existing_value.string, key) : 1);

    return (StringMapPut)
    {
        .value = existing_value.value,
        .existing = existing_hash != 0,
    };
}

fn StringMapPut string_map_put_assume_not_existent_assume_capacity(StringMap* map, u32 hash, String key, u32 value)
{
    assert(map->length < map->capacity);
    auto index = hash & (map->capacity - 1);
    
    return string_map_put_at_assume_not_existent_assume_capacity(map, hash, key, value, index);
}

fn StringMapPut string_map_put_assume_not_existent(StringMap* map, Arena* arena, u32 hash, String key, u32 value)
{
    string_map_ensure_capacity(map, arena, 1);
    return string_map_put_assume_not_existent_assume_capacity(map, hash, key, value);
}

fn StringMapPut string_map_put(StringMap* map, Arena* arena, String key, u32 value)
{
    Hash long_hash = hash_bytes(key);
    auto hash = (u32)long_hash;
    assert(hash);
    auto index = hash & (map->capacity - 1);
    auto slot = string_map_find_slot(map, index, key, value);
    if (slot != -1)
    {
        trap();
    }
    else
    {
        if (map->length < map->capacity)
        {
            trap();
        }
        else if (map->length == map->capacity)
        {
            auto result = string_map_put_assume_not_existent(map, arena, hash, key, value);
            assert(!result.existing);
            return result;
        }
        else
        {
            trap();
        }
    }
    trap();
}

fn int file_write(String file_path, String file_data)
{
    int file_descriptor = syscall_open((char*)file_path.pointer, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(file_descriptor != -1);

    auto bytes = syscall_write(file_descriptor, file_data.pointer, file_data.length);
    assert(bytes >= 0);
    assert((u64)bytes == file_data.length);

    int close_result = syscall_close(file_descriptor);
    assert(close_result == 0);
    return 0;
}

fn String file_read(Arena* arena, String path)
{
    String result = {};
    int file_descriptor = syscall_open((char*)path.pointer, 0, 0);
    assert(file_descriptor != -1);

    struct stat stat_buffer;
    int stat_result = syscall_fstat(file_descriptor, &stat_buffer);
    assert(stat_result == 0);

    u64 file_size = stat_buffer.st_size;

    result = (String){
        .pointer = arena_allocate_bytes(arena, file_size, 64),
        .length = file_size,
    };

    // TODO: big files
    ssize_t read_result = syscall_read(file_descriptor, result.pointer, result.length);
    assert(read_result >= 0);
    assert((u64)read_result == file_size);

    auto close_result = syscall_close(file_descriptor);
    assert(close_result == 0);

    return result;
}

fn void print_string(String message)
{
#if SILENT == 0
        ssize_t result = syscall_write(1, message.pointer, message.length);
        assert(result >= 0);
        assert((u64)result == message.length);
#else
        unused(message);
#endif
}

typedef enum ELFSectionType : u32
{
    null = 0x00,
    program = 0x01,
    symbol_table = 0x02,
    string_table = 0x03,
    relocation_with_addends = 0x04,
    symbol_hash_table = 0x05,
    dynamic = 0x06,
    note = 0x07,
    bss = 0x08,
    relocation_no_addends = 0x09,
    lib = 0x0a, // reserved
    dynamic_symbol_table = 0x0b,
    init_array = 0x0e,
    fini_array = 0x0f,
    preinit_array = 0x10,
    group = 0x11,
    symbol_table_section_header_index = 0x12,
} ELFSectionType;

struct ELFSectionHeaderFlags
{
    u64 write:1;
    u64 alloc:1;
    u64 executable:1;
    u64 blank:1;
    u64 merge:1;
    u64 strings:1;
    u64 info_link:1;
    u64 link_order:1;
    u64 os_non_conforming:1;
    u64 group:1;
    u64 tls:1;
};
typedef struct ELFSectionHeaderFlags ELFSectionHeaderFlags;

struct ELFSectionHeader
{
    u32 name_offset;
    ELFSectionType type;
    ELFSectionHeaderFlags flags;
    u64 address;
    u64 offset;
    u64 size;
    u32 link;
    u32 info;
    u64 alignment;
    u64 entry_size;
};
typedef struct ELFSectionHeader ELFSectionHeader;
static_assert(sizeof(ELFSectionHeader) == 64);

typedef enum ELFBitCount : u8
{
    bits32 = 1,
    bits64 = 2,
} ELFBitCount;

typedef enum ELFEndianness : u8
{
    little = 1,
    big = 2,
} ELFEndianness;

typedef enum ELFAbi : u8
{
    system_v_abi = 0,
    linux_abi = 3,
} ELFAbi;

typedef enum ELFType : u16
{
    none = 0,
    relocatable = 1,
    executable = 2,
    shared = 3,
    core = 4,
} ELFType;

typedef enum ELFMachine : u16
{
    x86_64 = 0x3e,
    aarch64 = 0xb7,
} ELFMachine;

typedef enum ELFSectionIndex : u16
{
    UNDEFINED = 0,
    ABSOLUTE = 0xfff1,
    COMMON = 0xfff2,
} ELFSectionIndex;

struct ELFHeader
{
    u8 identifier[4];
    ELFBitCount bit_count;
    ELFEndianness endianness;
    u8 format_version;
    ELFAbi abi;
    u8 abi_version;
    u8 padding[7];
    ELFType type;
    ELFMachine machine;
    u32 version;
    u64 entry_point;
    u64 program_header_offset;
    u64 section_header_offset;
    u32 flags;
    u16 elf_header_size;
    u16 program_header_size;
    u16 program_header_count;
    u16 section_header_size;
    u16 section_header_count;
    u16 section_header_string_table_index;

};
typedef struct ELFHeader ELFHeader;
static_assert(sizeof(ELFHeader) == 0x40);

typedef enum ELFSymbolBinding : u8
{
    LOCAL = 0,
    GLOBAL = 1,
    WEAK = 2,
} ELFSymbolBinding;

typedef enum ELFSymbolType : u8
{
    ELF_SYMBOL_TYPE_NONE = 0,
    ELF_SYMBOL_TYPE_OBJECT = 1,
    ELF_SYMBOL_TYPE_FUNCTION = 2,
    ELF_SYMBOL_TYPE_SECTION = 3,
    ELF_SYMBOL_TYPE_FILE = 4,
    ELF_SYMBOL_TYPE_COMMON = 5,
    ELF_SYMBOL_TYPE_TLS = 6,
} ELFSymbolType;
struct ELFSymbol
{
    u32 name_offset;
    ELFSymbolType type:4;
    ELFSymbolBinding binding:4;
    u8 other;
    u16 section_index; // In the section header table
    u64 value;
    u64 size;
};
typedef struct ELFSymbol ELFSymbol;
static_assert(sizeof(ELFSymbol) == 24);

struct NameReference
{
    u32 offset;
    u32 length;
};

typedef struct NameReference NameReference;

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

fn void vb_generic_ensure_capacity(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    u32 old_capacity = vb->capacity;
    u32 wanted_capacity = vb->length + item_count;

    if (old_capacity < wanted_capacity)
    {
        if (old_capacity == 0)
        {
            vb->pointer = reserve(default_size);
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


// fn u8* vb_generic_append(VirtualBuffer(u8)* vb, void* item_pointer, u32 item_size, u32 item_count)
// {
//     vb_generic_ensure_capacity(vb, item_size, item_count);
//     return vb_generic_append_assume_capacity(vb, item_pointer, item_size, item_count);
// }

#define vb_add(a, count) (typeof((a)->pointer)) vb_generic_add((VirtualBuffer(u8)*)(a), sizeof(*((a)->pointer)), count)
#define vb_append_one(a, item) (typeof((a)->pointer)) vb_generic_append((VirtualBuffer(u8)*)(a), &(item), sizeof(*((a)->pointer)), 1)

typedef struct Thread Thread;

typedef enum TypeId : u32
{
    // Simple types
    TYPE_BOTTOM = 0,
    TYPE_TOP,
    TYPE_LIVE_CONTROL,
    TYPE_DEAD_CONTROL,
    // Not simple types
    TYPE_INTEGER,
    TYPE_TUPLE,

    TYPE_COUNT,
} TypeId;

typedef struct BackendType BackendType;
struct TypeIndex
{
    u32 index;
};
typedef struct TypeIndex TypeIndex;
#define index_equal(a, b) (a.index == b.index)
static_assert(sizeof(TypeIndex) == sizeof(u32));
declare_slice(TypeIndex);

#define RawIndex(T, i) (T ## Index) { .index = (i) }
#define Index(T, i) RawIndex(T, (i) + 1)
#define geti(i) ((i).index - 1)
#define validi(i) ((i).index != 0)
#define invalidi(T) RawIndex(T, 0)

struct TypeInteger
{
    u64 constant;
    u8 bit_count;
    u8 is_constant;
    u8 is_signed;
    u8 padding1;
    u32 padding;
};
typedef struct TypeInteger TypeInteger;
static_assert(sizeof(TypeInteger) == 16);

struct TypeTuple
{
    Slice(TypeIndex) types;
};
typedef struct TypeTuple TypeTuple;

struct Type
{
    union
    {
        TypeInteger integer;
        TypeTuple tuple;
    };
    Hash hash;
    TypeId id;
};
typedef struct Type Type;
decl_vb(Type);

typedef enum NodeId : u8
{
    NODE_START,
    NODE_STOP,
    NODE_CONTROL_PROJECTION,
    NODE_DEAD_CONTROL,
    NODE_SCOPE,
    NODE_PROJECTION,
    NODE_RETURN,
    NODE_REGION,
    NODE_REGION_LOOP,
    NODE_IF,
    NODE_PHI,

    NODE_INTEGER_ADD,
    NODE_INTEGER_SUBSTRACT,
    NODE_INTEGER_MULTIPLY,
    NODE_INTEGER_UNSIGNED_DIVIDE,
    NODE_INTEGER_SIGNED_DIVIDE,
    NODE_INTEGER_UNSIGNED_REMAINDER,
    NODE_INTEGER_SIGNED_REMAINDER,
    NODE_INTEGER_UNSIGNED_SHIFT_LEFT,
    NODE_INTEGER_SIGNED_SHIFT_LEFT,
    NODE_INTEGER_UNSIGNED_SHIFT_RIGHT,
    NODE_INTEGER_SIGNED_SHIFT_RIGHT,
    NODE_INTEGER_AND,
    NODE_INTEGER_OR,
    NODE_INTEGER_XOR,

    NODE_CONSTANT,

    NODE_COUNT,
} NodeId;

struct NodeIndex
{
    u32 index;
};
typedef struct NodeIndex NodeIndex;
declare_slice(NodeIndex);
decl_vb(NodeIndex);

struct NodeCFG
{
    s32 immediate_dominator_tree_depth;
    s32 loop_depth;
    s32 anti_dependency;
};
typedef struct NodeCFG NodeCFG;

struct NodeConstant
{
    TypeIndex type;
};
typedef struct NodeConstant NodeConstant;

struct NodeStart
{
    NodeCFG cfg;
    TypeIndex arguments;
};
typedef struct NodeStart NodeStart;

struct NodeStop
{
    NodeCFG cfg;
};
typedef struct NodeStop NodeStop;

struct ScopePair
{
    StringMap values;
    StringMap types;
};
typedef struct ScopePair ScopePair;

struct StackScope
{
    ScopePair* pointer;
    u32 length;
    u32 capacity;
};
typedef struct StackScope StackScope;

struct NodeScope
{
    StackScope stack;
};
typedef struct NodeScope NodeScope;

struct NodeProjection
{
    String label;
    u32 index;
};
typedef struct NodeProjection NodeProjection;

struct NodeControlProjection
{
    NodeProjection projection;
    NodeCFG cfg;
};
typedef struct NodeControlProjection NodeControlProjection;

struct NodeReturn
{
    NodeCFG cfg;
};
typedef struct NodeReturn NodeReturn;

struct NodeDeadControl
{
    NodeCFG cfg;
};
typedef struct NodeDeadControl NodeDeadControl;

struct Node
{
    Hash hash;
    u32 input_offset;
    u32 output_offset;
    u32 dependency_offset;
    u16 output_count;
    u16 input_count;
    u16 dependency_count;
    u16 input_capacity;
    u16 output_capacity;
    u16 dependency_capacity;
    u16 thread_id;
    TypeIndex type;
    NodeId id;
    union
    {
        NodeConstant constant;
        NodeStart start;
        NodeStop stop;
        NodeScope scope;
        NodeControlProjection control_projection;
        NodeReturn return_node;
        NodeDeadControl dead_control;
    };
};
typedef struct Node Node;
declare_slice_p(Node);

decl_vb(Node);
decl_vbp(Node);

struct ArrayReference
{
    u32 offset;
    u32 length;
};
typedef struct ArrayReference ArrayReference;
decl_vb(ArrayReference);

struct Function
{
    String name;
    NodeIndex start;
    NodeIndex stop;
    TypeIndex return_type;
};
typedef struct Function Function;
decl_vb(Function);

struct File
{
    String path;
    String source;
    StringMap values;
    StringMap types;
};
typedef struct File File;

struct FunctionBuilder
{
    Function* function;
    File* file;
    NodeIndex scope;
    NodeIndex dead_control;
};
typedef struct FunctionBuilder FunctionBuilder;

struct InternPool
{
    u32* pointer;
    u32 length;
    u32 capacity;
};
typedef struct InternPool InternPool;

typedef u64 BitsetElement;
decl_vb(BitsetElement);
declare_slice(BitsetElement);
struct Bitset
{
    VirtualBuffer(BitsetElement) arr;
    u32 length;
};
typedef struct Bitset Bitset;
const u64 element_bitsize = sizeof(u64) * 8;

fn u8 bitset_get(Bitset* bitset, u64 index)
{
    auto element_index = index / element_bitsize;
    if (element_index < bitset->arr.length)
    {
        auto bit_index = index % element_bitsize;
        auto result = (bitset->arr.pointer[element_index] & (1 << bit_index)) != 0;
        return result;
    }

    return 0;
}

fn void bitset_ensure_length(Bitset* bitset, u64 max)
{
    auto length = (max / (sizeof(u64) * 8)) + (max % (sizeof(u64) * 8) != 0);
    auto old_length = bitset->arr.length;
    if (old_length < length)
    {
        auto new_element_count = length - old_length;
        unused(vb_add(&bitset->arr, new_element_count));
    }
}

fn void bitset_set_value(Bitset* bitset, u64 index, u8 value)
{
    bitset_ensure_length(bitset, index + 1);
    auto element_index = index / element_bitsize;
    auto bit_index = index % element_bitsize;
    bitset->arr.pointer[element_index] |= (!!value) << bit_index;
}

fn void bitset_clear(Bitset* bitset)
{
    memset(bitset->arr.pointer, 0, bitset->arr.capacity);
    bitset->arr.length = 0;
    bitset->length = 0;
}

struct WorkList
{
    VirtualBuffer(NodeIndex) nodes;
    Bitset visited;
    Bitset bitset;
    u32 mid_assert:1;
};
typedef struct WorkList WorkList;

struct Thread
{
    Arena* arena;
    struct
    {
        VirtualBuffer(Type) types;
        VirtualBuffer(Node) nodes;
        VirtualBuffer(NodeIndex) uses;
        VirtualBuffer(u8) name_buffer;
        VirtualBuffer(ArrayReference) use_free_list;
        VirtualBuffer(Function) functions;
    } buffer;
    struct
    {
        InternPool types;
        InternPool nodes;
    } interned;
    struct
    {
        TypeIndex bottom;
        TypeIndex top;
        TypeIndex live_control;
        TypeIndex dead_control;
        struct
        {
            TypeIndex top;
            TypeIndex bottom;
            TypeIndex zero;
        } integer;
    } types;
    struct
    {
        u64 total;
        u64 nop;
    } iteration;
    WorkList worklist;
    s64 main_function;
};
typedef struct Thread Thread;

fn NodeIndex thread_worklist_push(Thread* thread, NodeIndex node_index)
{
    if (validi(node_index))
    {
        if (!bitset_get(&thread->worklist.bitset, geti(node_index)))
        {
            bitset_set_value(&thread->worklist.bitset, geti(node_index), 1);
            *vb_add(&thread->worklist.nodes, 1) = node_index;
        }
    }

    return node_index;
}

fn NodeIndex thread_worklist_pop(Thread* thread)
{
    auto result = invalidi(Node);

    auto len = thread->worklist.nodes.length;
    if (len)
    {
        auto index = len - 1;
        auto node_index = thread->worklist.nodes.pointer[index];
        thread->worklist.nodes.length = index;
        bitset_set_value(&thread->worklist.bitset, index, 0);
        result = node_index;
    }

    return result;
}

fn void thread_worklist_clear(Thread* thread)
{
    bitset_clear(&thread->worklist.visited);
    bitset_clear(&thread->worklist.bitset);
    thread->worklist.nodes.length = 0;
}

fn Type* thread_type_get(Thread* thread, TypeIndex type_index)
{
    assert(validi(type_index));
    auto* type = &thread->buffer.types.pointer[geti(type_index)];
    return type;
}

fn Node* thread_node_get(Thread* thread, NodeIndex node_index)
{
    assert(validi(node_index));
    auto* node = &thread->buffer.nodes.pointer[geti(node_index)];
    return node;
}

fn void thread_node_set_use(Thread* thread, u32 offset, u16 index, NodeIndex new_use)
{
    thread->buffer.uses.pointer[offset + index] = new_use;
}

fn NodeIndex thread_node_get_use(Thread* thread, u32 offset, u16 index)
{
    NodeIndex i = thread->buffer.uses.pointer[offset + index];
    return i;
}

fn NodeIndex node_input_get(Thread* thread, Node* node, u16 index)
{
    assert(index < node->input_count);
    NodeIndex result = thread_node_get_use(thread, node->input_offset, index);
    return result;
}

fn NodeIndex node_output_get(Thread* thread, Node* node, u16 index)
{
    assert(index < node->output_count);
    NodeIndex result = thread_node_get_use(thread, node->output_offset, index);
    return result;
}

fn NodeIndex scope_get_control(Thread* thread, Node* node)
{
    assert(node->id == NODE_SCOPE);
    auto control = node_input_get(thread, node, 0);
    return control;
}

fn NodeIndex builder_get_control_node_index(Thread* thread, FunctionBuilder* builder)
{
    auto* scope_node = thread_node_get(thread, builder->scope);
    auto result = scope_get_control(thread, scope_node);
    return result;
}

typedef struct NodeDualReference NodeDualReference;

struct UseReference
{
    NodeIndex* pointer;
    u32 index;
};
typedef struct UseReference UseReference;

fn UseReference thread_get_node_reference_array(Thread* thread, u16 count)
{
    u32 free_list_count = thread->buffer.use_free_list.length;
    for (u32 i = 0; i < free_list_count; i += 1)
    {
        if (thread->buffer.use_free_list.pointer[i].length >= count)
        {
            trap();
        }
    }

    u32 index = thread->buffer.uses.length;
    auto* node_indices = vb_add(&thread->buffer.uses, count);
    return (UseReference)
    {
        .pointer = node_indices,
        .index = index,
    };
}

fn void node_ensure_capacity(Thread* thread, u32* offset, u16* capacity, u16 current_length, u16 additional)
{
    auto current_offset = *offset;
    auto current_capacity = *capacity;
    auto desired_capacity = current_length + additional;

    if (desired_capacity > current_capacity)
    {
        auto* ptr = vb_add(&thread->buffer.uses, desired_capacity);
        u32 new_offset = ptr - thread->buffer.uses.pointer;
        memcpy(ptr, &thread->buffer.uses.pointer[current_offset], current_length * sizeof(NodeIndex));
        memset(ptr + current_length, 0, (desired_capacity - current_length) * sizeof(NodeIndex));
        *offset = new_offset;
        *capacity = desired_capacity;
    }
}

fn void node_add_one_assume_capacity(Thread* thread, NodeIndex node, u32 offset, u16 capacity, u16* length)
{
    auto index = *length;
    assert(index < capacity);
    thread->buffer.uses.pointer[offset + index] = node;
    *length = index + 1;
}

fn void node_add_one(Thread* thread, u32* offset, u16* capacity, u16* count, NodeIndex node_index)
{
    node_ensure_capacity(thread, offset, capacity, *count, 1);
    node_add_one_assume_capacity(thread, node_index, *offset, *capacity, count);
}

fn NodeIndex node_add_output(Thread* thread, NodeIndex node_index, NodeIndex output_index)
{
    auto* this_node = thread_node_get(thread, node_index);
    node_add_one(thread, &this_node->output_offset, &this_node->output_capacity, &this_node->output_count, output_index);

    return node_index;
}

fn NodeIndex intern_pool_remove_node(Thread* thread, NodeIndex node_index);
fn void node_unlock(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    if (node->hash)
    {
        auto old_node_index = intern_pool_remove_node(thread, node_index);
        assert(index_equal(old_node_index, node_index));
        node->hash = 0;
    }
}

fn s32 node_find(Slice(NodeIndex) nodes, NodeIndex node_index)
{
    s32 result = -1;
    for (u32 i = 0; i < nodes.length; i += 1)
    {
        if (index_equal(nodes.pointer[i], node_index))
        {
            result = i;
            break;
        }
    }
    return result;
}

fn void thread_node_remove_use(Thread* thread, u32 offset, u16* length, u16 index)
{
    auto current_length = *length;
    assert(index < current_length);
    auto item_to_remove = &thread->buffer.uses.pointer[offset + index];
    auto substitute = &thread->buffer.uses.pointer[offset + current_length - 1];
    *item_to_remove = *substitute;
    *length = current_length - 1;
}

fn Slice(NodeIndex) node_get_inputs(Thread* thread, Node* node)
{
    auto result = (Slice(NodeIndex)) {
        .pointer = &thread->buffer.uses.pointer[node->input_offset],
        .length = node->input_count,
    };
    return result;
}

fn Slice(NodeIndex) node_get_outputs(Thread* thread, Node* node)
{
    auto result = (Slice(NodeIndex)) {
        .pointer = &thread->buffer.uses.pointer[node->output_offset],
        .length = node->output_count,
    };
    return result;
}

fn u8 node_remove_output(Thread* thread, NodeIndex node_index, NodeIndex use_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto outputs = node_get_outputs(thread, node);
    auto index = node_find(outputs, use_index);
    assert(index != -1);
    thread_node_remove_use(thread, node->output_offset, &node->output_count, index);
    return node->output_count == 0;
}

fn void move_dependencies_to_worklist(Thread* thread, Node* node)
{
    assert(node->dependency_count == 0);
    for (u32 i = 0; i < node->dependency_count; i += 1)
    {
        trap();
    }
}

fn NodeIndex node_set_input(Thread* thread, NodeIndex node_index, u16 index, NodeIndex new_input)
{
    auto* node = thread_node_get(thread, node_index);
    assert(index < node->input_count);
    node_unlock(thread, node_index);
    auto old_input = node_input_get(thread, node, index);

    if (!index_equal(old_input, new_input))
    {
        if (validi(new_input))
        {
            node_add_output(thread, new_input, node_index);
        }

        thread_node_set_use(thread, node->input_offset, index, new_input);

        if (validi(old_input))
        {
            if (node_remove_output(thread, old_input, node_index))
            {
                trap();
            }
        }

        move_dependencies_to_worklist(thread, node);
    }

    return new_input;
}

fn NodeIndex builder_set_control(Thread* thread, FunctionBuilder* builder, NodeIndex node_index)
{
    return node_set_input(thread, builder->scope, 0, node_index);
}

fn NodeIndex node_add_input(Thread* thread, NodeIndex node_index, NodeIndex input_index)
{
    node_unlock(thread, node_index);
    Node* this_node = thread_node_get(thread, node_index);
    node_add_one(thread, &this_node->input_offset, &this_node->input_capacity, &this_node->input_count, input_index);
    if (validi(input_index))
    {
        node_add_output(thread, input_index, node_index);
    }

    return input_index;
}

fn NodeIndex builder_add_return(Thread* thread, FunctionBuilder* builder, NodeIndex node_index)
{
    return node_add_input(thread, builder->function->stop, node_index);
}

struct NodeCreate
{
    NodeId id;
    Slice(NodeIndex) inputs;
};
typedef struct NodeCreate NodeCreate;

fn String node_id_to_string(Node* node)
{
    switch (node->id)
    {
        case_to_name(NODE_, START);
        case_to_name(NODE_, STOP);
        case_to_name(NODE_, CONTROL_PROJECTION);
        case_to_name(NODE_, DEAD_CONTROL);
        case_to_name(NODE_, SCOPE);
        case_to_name(NODE_, PROJECTION);
        case_to_name(NODE_, RETURN);
        case_to_name(NODE_, REGION);
        case_to_name(NODE_, REGION_LOOP);
        case_to_name(NODE_, IF);
        case_to_name(NODE_, PHI);
        case_to_name(NODE_, INTEGER_ADD);
        case_to_name(NODE_, INTEGER_SUBSTRACT);
        case_to_name(NODE_, INTEGER_MULTIPLY);
        case_to_name(NODE_, INTEGER_UNSIGNED_DIVIDE);
        case_to_name(NODE_, INTEGER_SIGNED_DIVIDE);
        case_to_name(NODE_, INTEGER_UNSIGNED_REMAINDER);
        case_to_name(NODE_, INTEGER_SIGNED_REMAINDER);
        case_to_name(NODE_, INTEGER_UNSIGNED_SHIFT_LEFT);
        case_to_name(NODE_, INTEGER_SIGNED_SHIFT_LEFT);
        case_to_name(NODE_, INTEGER_UNSIGNED_SHIFT_RIGHT);
        case_to_name(NODE_, INTEGER_SIGNED_SHIFT_RIGHT);
        case_to_name(NODE_, INTEGER_AND);
        case_to_name(NODE_, INTEGER_OR);
        case_to_name(NODE_, INTEGER_XOR);
        case_to_name(NODE_, CONSTANT);
        case_to_name(NODE_, COUNT);
    }
}

fn NodeIndex thread_node_add(Thread* thread, NodeCreate data)
{
    auto input_result = thread_get_node_reference_array(thread, data.inputs.length);
    memcpy(input_result.pointer, data.inputs.pointer, sizeof(Node*) * data.inputs.length);

    auto* node = vb_add(&thread->buffer.nodes, 1);
    auto node_index = Index(Node, node - thread->buffer.nodes.pointer);
    memset(node, 0, sizeof(Node));
    node->id = data.id;
    node->input_offset = input_result.index;
    node->input_count = data.inputs.length;
    node->type = invalidi(Type);

    print("[NODE CREATION] #{u32} {s} | INPUTS: { ", node_index.index, node_id_to_string(node));

    for (u32 i = 0; i < data.inputs.length; i += 1)
    {
        NodeIndex input = data.inputs.pointer[i];
        print("{u32} ", input.index);
        if (validi(input))
        {
            node_add_output(thread, input, node_index);
        }
    }

    print("}\n");

    return node_index;
}

fn void scope_push(Thread* thread, FunctionBuilder* builder)
{
    auto* scope = thread_node_get(thread, builder->scope);
    auto current_length = scope->scope.stack.length;
    auto desired_length = current_length + 1;
    auto current_capacity = scope->scope.stack.capacity;

    if (current_capacity < desired_length)
    {
        auto optimal_capacity = MAX(round_up_to_next_power_of_2(desired_length), 8);
        auto* new_pointer = arena_allocate(thread->arena, ScopePair, optimal_capacity);
        memcpy(new_pointer, scope->scope.stack.pointer, current_length * sizeof(ScopePair));
        scope->scope.stack.capacity = optimal_capacity;
        scope->scope.stack.pointer = new_pointer;
    }

    memset(&scope->scope.stack.pointer[current_length], 0, sizeof(ScopePair));
    scope->scope.stack.length = current_length + 1;
}

fn void node_pop_inputs(Thread* thread, NodeIndex node_index, u16 input_count)
{
    node_unlock(thread, node_index);
    auto* node = thread_node_get(thread, node_index);
    auto inputs = node_get_inputs(thread, node);
    for (u16 i = 0; i < input_count; i += 1)
    {
        auto old_input = inputs.pointer[node->input_count - 1];
        node->input_count -= 1;
        if (validi(old_input))
        {
            if (node_remove_output(thread, old_input, node_index))
            {
                trap();
            }
        }
    }
}

fn void scope_pop(Thread* thread, FunctionBuilder* builder)
{
    auto scope_index = builder->scope;
    auto* scope = thread_node_get(thread, scope_index);
    auto index = scope->scope.stack.length - 1;
    auto popped_scope = scope->scope.stack.pointer[index];
    scope->scope.stack.length = index;
    auto input_count = popped_scope.values.length;
    node_pop_inputs(thread, scope_index, input_count);
}

fn ScopePair* scope_get_last(Node* node)
{
    assert(node->id == NODE_SCOPE);
    return &node->scope.stack.pointer[node->scope.stack.length - 1];
}

fn NodeIndex scope_define(Thread* thread, FunctionBuilder* builder, String name, TypeIndex type_index, NodeIndex node_index)
{
    auto scope_node_index = builder->scope;
    auto* scope_node = thread_node_get(thread, scope_node_index);
    auto* last = scope_get_last(scope_node);
    string_map_put(&last->types, thread->arena, name, geti(type_index));

    auto existing = string_map_put(&last->values, thread->arena, name, scope_node->input_count).existing;
    NodeIndex result;

    if (existing)
    {
        result = invalidi(Node);
    }
    else
    {
        result = node_add_input(thread, scope_node_index, node_index);
    }

    return result;
}

fn u8 type_equal(Thread* thread, Type* a, Type* b)
{
    u8 result = 0;
    if (a == b)
    {
        result = 1;
    }
    else
    {
        assert(a->hash);
        assert(b->hash);
        if ((a->hash == b->hash) & (a->id == b->id))
        {
            switch (a->id)
            {
                case TYPE_INTEGER:
                    {
                        result = 
                            ((a->integer.constant == b->integer.constant) & (a->integer.bit_count == b->integer.bit_count))
                            &
                            ((a->integer.is_signed == b->integer.is_signed) & (a->integer.is_constant == b->integer.is_constant));
                    } break;
                case TYPE_TUPLE:
                    {
                        result = a->tuple.types.length == b->tuple.types.length;

                        if (result)
                        {
                            for (u32 i = 0; i < a->tuple.types.length; i += 1)
                            {
                                if (!index_equal(a->tuple.types.pointer[i], b->tuple.types.pointer[i]))
                                {
                                    todo();
                                }
                            }
                        }
                    } break;
                default:
                    trap();
            }
        }
    }

    return result;
}
fn Hash hash_type(Thread* thread, Type* type);

fn Hash node_get_hash_default(Thread* thread, Node* node, NodeIndex node_index)
{
    return fnv_offset;
}

fn Hash node_get_hash_projection(Thread* thread, Node* node)
{
    trap();
}

fn Hash node_get_hash_control_projection(Thread* thread, Node* node, NodeIndex node_index)
{
    auto projection_index = node->control_projection.projection.index;
    auto proj_index_bytes = struct_to_bytes(projection_index);
    return hash_bytes(proj_index_bytes);
}

fn Hash node_get_hash_constant(Thread* thread, Node* node, NodeIndex node_index)
{
    auto type_index = node->type;
    auto* type = thread_type_get(thread, node->type);
    auto type_hash = hash_type(thread, type);
    // print("Hashing node #{u32} (constant) (type: #{u32}) (hash: {u64:x})\n", node_index.index, type_index.index, type_hash);
    return type_hash;
}

struct TypeGetOrPut
{
    TypeIndex index;
    u8 existing;
};

typedef struct TypeGetOrPut TypeGetOrPut;

fn TypeGetOrPut intern_pool_get_or_put_new_type(Thread* thread, Type* type);

typedef NodeIndex NodeIdealize(Thread* thread, NodeIndex node_index);
typedef TypeIndex NodeComputeType(Thread* thread, NodeIndex node_index);
typedef Hash TypeGetHash(Thread* thread, Type* type);
typedef Hash NodeGetHash(Thread* thread, Node* node, NodeIndex node_index);

fn TypeIndex thread_get_integer_type(Thread* thread, TypeInteger type_integer)
{
    Type type;
    memset(&type, 0, sizeof(Type));
    type.integer = type_integer;
    type.id = TYPE_INTEGER;

    auto result = intern_pool_get_or_put_new_type(thread, &type);
    return result.index;
}

fn NodeIndex peephole(Thread* thread, Function* function, NodeIndex node_index);
fn NodeIndex constant_int_create_with_type(Thread* thread, Function* function, TypeIndex type_index)
{
    auto node_index = thread_node_add(thread, (NodeCreate){
        .id = NODE_CONSTANT,
        .inputs = array_to_slice(((NodeIndex []) {
            function->start,
        }))
    });
    auto* node = thread_node_get(thread, node_index);

    node->constant = (NodeConstant) {
        .type = type_index,
    };

    print("Creating constant integer node #{u32} with value: {u64:x}\n", node_index.index, thread_type_get(thread, type_index)->integer.constant);

    auto result = peephole(thread, function, node_index);
    return result;
}

fn NodeIndex constant_int_create(Thread* thread, Function* function, u64 value)
{
    auto type_index = thread_get_integer_type(thread, (TypeInteger){
        .constant = value,
        .bit_count = 0,
        .is_constant = 1,
        .is_signed = 0,
    });

    auto constant_int = constant_int_create_with_type(thread, function, type_index);
    return constant_int;
}

struct NodeVirtualTable
{
    NodeComputeType* const compute_type;
    NodeIdealize* const idealize;
    NodeGetHash* const get_hash;
};
typedef struct NodeVirtualTable NodeVirtualTable;

struct TypeVirtualTable
{
    TypeGetHash* const get_hash;
};
typedef struct TypeVirtualTable TypeVirtualTable;
fn Hash hash_type(Thread* thread, Type* type);

fn NodeIndex idealize_null(Thread* thread, NodeIndex node_index)
{
    return invalidi(Node);
}

fn TypeIndex compute_type_constant(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    assert(node->id == NODE_CONSTANT);
    return node->constant.type;
}

fn Hash type_get_hash_default(Thread* thread, Type* type)
{
    assert(!type->hash);
    Hash hash = fnv_offset;


    u32 i = 0;
    for (auto* it = (u8*)type; it < (u8*)(type + 1); it += 1)
    {
        hash = hash_byte(hash, *it);
        if (type->id == TYPE_INTEGER)
        {
            // print("Byte [{u32}] = 0x{u32:x}\n", i, (u32)*it);
            i += 1;
        }
    }

    return hash;
}

fn Hash type_get_hash_tuple(Thread* thread, Type* type)
{
    Hash hash = fnv_offset;
    for (u64 i = 0; i < type->tuple.types.length; i += 1)
    {
        auto* tuple_type = thread_type_get(thread,type->tuple.types.pointer[i]); 
        auto type_hash = hash_type(thread, tuple_type);
        for (u8* it = (u8*)&type_hash; it < (u8*)(&type_hash + 1); it += 1)
        {
            hash = hash_byte(hash, *it);
        }
    }

    return hash;
}

fn u8 is_projection(Node* n)
{
    return (n->id == NODE_CONTROL_PROJECTION) | (n->id == NODE_PROJECTION);
}

fn NodeIndex projection_get_control(Thread* thread, Node* node)
{
    assert(is_projection(node));
    auto node_index = node_input_get(thread, node, 0);
    return node_index;
}

fn s32 projection_get_index(Thread* thread, Node* node)
{
    assert(is_projection(node));

    switch (node->id)
    {
        case NODE_CONTROL_PROJECTION:
            return node->control_projection.projection.index;
        default:
            trap();
    }
}

fn TypeIndex compute_type_projection(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    assert(is_projection(node));
    auto control_node_index = projection_get_control(thread, node);
    auto* control_node = thread_node_get(thread, control_node_index);
    auto* control_type = thread_type_get(thread, control_node->type);

    if (control_type->id == TYPE_TUPLE)
    {
        auto index = projection_get_index(thread, node);
        auto type_index = control_type->tuple.types.pointer[index];
        return type_index;
    }
    else
    {
        return thread->types.bottom;
    }
}

fn NodeIndex idealize_control_projection(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    assert(node->id == NODE_CONTROL_PROJECTION);
    auto control_node_index = projection_get_control(thread, node);
    auto* control_node = thread_node_get(thread, control_node_index);
    auto* control_type = thread_type_get(thread, control_node->type);
    auto index = node->control_projection.projection.index;

    if (control_type->id == TYPE_TUPLE)
    {
        if (index_equal(control_type->tuple.types.pointer[index], thread->types.dead_control))
        {
            trap();
        }
        if (control_node->id == NODE_IF)
        {
            trap();
        }
    }

    if (control_node->id == NODE_IF)
    {
        trap();
    }

    return invalidi(Node);
}

fn NodeIndex return_get_control(Thread* thread, Node* node)
{
    return node_input_get(thread, node, 0);
}

fn NodeIndex return_get_value(Thread* thread, Node* node)
{
    return node_input_get(thread, node, 1);
}

fn void intern_pool_ensure_capacity(InternPool* pool, Arena* arena, u32 additional)
{
    auto current_capacity = pool->capacity;
    auto half_capacity = current_capacity >> 1;
    auto destination_length = pool->length + additional;

    if (destination_length > half_capacity)
    {
        u32 new_capacity = MAX(round_up_to_next_power_of_2(destination_length), 32);
        u32* new_array = arena_allocate(arena, u32, new_capacity);
        memset(new_array, 0, sizeof(u32) * new_capacity);

        auto* old_pointer = pool->pointer;
        auto old_capacity = pool->capacity;

        pool->length = 0;
        pool->pointer = new_array;
        pool->capacity = new_capacity;

        for (u32 i = 0; i < old_capacity; i += 1)
        {
           auto key = old_pointer[i];
           if (key)
           {
               trap();
           }
        }

        for (u32 i = 0; i < old_capacity; i += 1)
        {
            trap();
        }
    }
}

fn TypeIndex intern_pool_put_new_type_at_assume_not_existent_assume_capacity(Thread* thread, Type* type, u32 index)
{
    auto* result = vb_add(&thread->buffer.types, 1);
    auto buffer_index = result - thread->buffer.types.pointer;
    auto type_index = Index(Type, buffer_index);
    if (type->id == TYPE_INTEGER)
    {
        auto diff = (u8*)(&type->hash) - (u8*)(&type->integer.is_signed + 1);
        for (u32 i = 0; i < diff; i += 1)
        {
            assert(*(&type->integer.is_signed + 1 + i) == 0);
        }
    }
    *result = *type;

    thread->interned.types.pointer[index] = *(u32*)&type_index;
    thread->interned.types.length += 1;

    return type_index;
}

fn TypeIndex intern_pool_put_new_type_assume_not_existent_assume_capacity(Thread* thread, Hash hash, Type* type)
{
    assert(thread->interned.types.length < thread->interned.types.capacity);
    auto index = hash & (thread->interned.types.capacity - 1);

    return intern_pool_put_new_type_at_assume_not_existent_assume_capacity(thread, type, index);
}

fn TypeIndex intern_pool_put_new_type_assume_not_existent(Thread* thread, Hash hash, Type* type)
{
    intern_pool_ensure_capacity(&thread->interned.types, thread->arena, 1);
    return intern_pool_put_new_type_assume_not_existent_assume_capacity(thread, hash, type);
}

fn s32 intern_pool_find_type_slot(Thread* thread, u32 original_index, Type* type)
{
    auto it_index = original_index;
    auto existing_capacity = thread->interned.types.capacity;
    s32 result = -1;

    for (u32 i = 0; i < existing_capacity; i += 1)
    {
        auto index = it_index & (existing_capacity - 1);
        u32 key = thread->interned.types.pointer[index];

        // Not set
        if (key == 0)
        {
            result = index;
            break;
        }
        else
        {
            TypeIndex existing_type_index = *(TypeIndex*)&key;
            Type* existing_type = thread_type_get(thread, existing_type_index);
            if (type_equal(thread, existing_type, type))
            {
                result = index;
                break;
            }
        }

        it_index += 1;
    }

    return result;
}

fn TypeGetOrPut intern_pool_get_or_put_new_type(Thread* thread, Type* type)
{
    auto existing_capacity = thread->interned.types.capacity;
    auto hash = hash_type(thread, type);
    auto original_index = hash & (existing_capacity - 1);
    
    auto slot = intern_pool_find_type_slot(thread, original_index, type);
    if (slot != -1)
    {
        u32 index = slot;
        TypeIndex type_index = *(TypeIndex*)&thread->interned.types.pointer[index];
        u8 existing = validi(type_index);
        if (!existing)
        {
            type_index = intern_pool_put_new_type_at_assume_not_existent_assume_capacity(thread, type, index);
        }

        return (TypeGetOrPut) {
            .index = type_index,
            .existing = existing,
        };
    }
    else
    {
        if (thread->interned.types.length < existing_capacity)
        {
            trap();
        }
        else if (thread->interned.types.length == existing_capacity)
        {
            auto result = intern_pool_put_new_type_assume_not_existent(thread, hash, type);
            return (TypeGetOrPut) {
                .index = result,
                .existing = 0,
            };
        }
        else
        {
            trap();
        }
    }
}


fn TypeGetOrPut type_make_tuple(Thread* thread, Slice(TypeIndex) types)
{
    Type type;
    memset(&type, 0, sizeof(Type));
    type.tuple = (TypeTuple){
        .types = types,
    };
    type.id = TYPE_TUPLE;
    auto result = intern_pool_get_or_put_new_type(thread, &type);
    return result;
}

fn TypeIndex type_make_tuple_allocate(Thread* thread, Slice(TypeIndex) types)
{
    auto gop = type_make_tuple(thread, types);
    // Need to reallocate the type array
    if (!gop.existing)
    {
        auto* type = thread_type_get(thread, gop.index);
        assert(type->tuple.types.pointer == types.pointer);
        assert(type->tuple.types.length == types.length);
        type->tuple.types = arena_allocate_slice(thread->arena, TypeIndex, types.length);
        memcpy(type->tuple.types.pointer, types.pointer, sizeof(TypeIndex) * types.length);
    }

    return gop.index;
}

fn TypeIndex compute_type_return(Thread* thread, NodeIndex node_index)
{
    Node* node = thread_node_get(thread, node_index);
    auto control_type = thread_node_get(thread, return_get_control(thread, node))->type;
    auto return_type = thread_node_get(thread, return_get_value(thread, node))->type;
    Slice(TypeIndex) types = array_to_slice(((TypeIndex[]) {
        control_type,
        return_type,
    }));
    auto result = type_make_tuple_allocate(thread, types);
    return result;
}

fn NodeIndex idealize_return(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto control_node_index = return_get_control(thread, node);
    auto* control_node = thread_node_get(thread, control_node_index);
    if (index_equal(control_node->type, thread->types.dead_control))
    {
        return control_node_index;
    }
    else
    {
        return invalidi(Node);
    }

}

fn TypeIndex compute_type_dead_control(Thread* thread, NodeIndex node_index)
{
    unused(node_index);
    return thread->types.dead_control;
}

fn TypeIndex compute_type_bottom(Thread* thread, NodeIndex node_index)
{
    unused(node_index);
    return thread->types.bottom;
}

fn NodeIndex idealize_stop(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto original_input_count = node->input_count;
    for (u16 i = 0; i < node->input_count; i += 1)
    {
        auto input_node_index = node_input_get(thread, node, i);
        auto* input_node = thread_node_get(thread, input_node_index);
        if (index_equal(input_node->type, thread->types.dead_control))
        {
            trap();
        }
    }

    if (node->input_count != original_input_count)
    {
        return node_index;
    }
    else
    {
        return invalidi(Node);
    }
}

fn TypeIndex compute_type_start(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    return node->start.arguments;
}

fn u8 type_is_constant(Type* type)
{
    switch (type->id)
    {
        case TYPE_INTEGER:
            return type->integer.is_constant;
        default:
            return 0;
    }
}

fn u8 node_is_unused(Node* node)
{
    return node->output_count == 0;
}

fn u8 node_is_dead(Node* node)
{
    return node_is_unused(node) & ((node->input_count == 0) & (!validi(node->type)));
}

fn TypeIndex compute_type_integer_binary(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto inputs = node_get_inputs(thread, node);
    auto* left = thread_node_get(thread, inputs.pointer[1]);
    auto* right = thread_node_get(thread, inputs.pointer[2]);
    assert(!node_is_dead(left));
    assert(!node_is_dead(right));
    auto* left_type = thread_type_get(thread, left->type);
    auto* right_type = thread_type_get(thread, right->type);

    if (((left_type->id == TYPE_INTEGER) & (right_type->id == TYPE_INTEGER)) & (type_is_constant(left_type) & type_is_constant(right_type)))
    {
        auto left_value = left_type->integer.constant;
        auto right_value = right_type->integer.constant;
        assert(left_type->integer.bit_count == 0);
        assert(right_type->integer.bit_count == 0);
        assert(!left_type->integer.is_signed);
        assert(!right_type->integer.is_signed);

        u64 result;
        TypeInteger type_integer = left_type->integer;

        switch (node->id)
        {
            case NODE_INTEGER_ADD:
                result = left_value + right_value;
                break;
            case NODE_INTEGER_SUBSTRACT:
                result = left_value - right_value;
                break;
            case NODE_INTEGER_AND:
                result = left_value & right_value;
                break;
            case NODE_INTEGER_OR:
                result = left_value | right_value;
                break;
            case NODE_INTEGER_XOR:
                result = left_value ^ right_value;
                break;
            default:
                trap();
        }
        type_integer.constant = result;
        auto new_type = thread_get_integer_type(thread, type_integer);
        return new_type;
    }
    else
    {
        trap();
    }
}

global const TypeVirtualTable type_functions[TYPE_COUNT] = {
    [TYPE_BOTTOM] = { .get_hash = &type_get_hash_default },
    [TYPE_TOP] = { .get_hash = &type_get_hash_default },
    [TYPE_LIVE_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_DEAD_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_INTEGER] = { .get_hash = &type_get_hash_default },
    [TYPE_TUPLE] = { .get_hash = &type_get_hash_tuple },
};

global const NodeVirtualTable node_functions[NODE_COUNT] = {
    [NODE_START] = {
        .compute_type = &compute_type_start,
        .idealize = &idealize_null,
        .get_hash = &node_get_hash_default,
    },
    [NODE_STOP] = {
        .compute_type = &compute_type_bottom,
        .idealize = &idealize_stop,
        .get_hash = &node_get_hash_default,
    },
    [NODE_CONTROL_PROJECTION] = {
        .compute_type = &compute_type_projection,
        .idealize = &idealize_control_projection,
        .get_hash = &node_get_hash_control_projection,
    },
    [NODE_DEAD_CONTROL] = {
        .compute_type = &compute_type_dead_control,
        .idealize = &idealize_null,
        .get_hash = &node_get_hash_default,
    },
    [NODE_RETURN] = {
        .compute_type = &compute_type_return,
        .idealize = &idealize_return,
        .get_hash = &node_get_hash_default,
    },

    // Integer operations
    [NODE_INTEGER_ADD] = {
        .compute_type = &compute_type_integer_binary,
    },
    [NODE_INTEGER_SUBSTRACT] = {
        .compute_type = &compute_type_integer_binary,
    },
    [NODE_INTEGER_AND] = {
        .compute_type = &compute_type_integer_binary,
    },
    [NODE_INTEGER_OR] = {
        .compute_type = &compute_type_integer_binary,
    },
    [NODE_INTEGER_XOR] = {
        .compute_type = &compute_type_integer_binary,
    },

    // Constant
    [NODE_CONSTANT] = {
        .compute_type = &compute_type_constant,
        .idealize = &idealize_null,
        .get_hash = &node_get_hash_constant,
    },
};

fn String type_id_to_string(Type* type)
{
    switch (type->id)
    {
        case_to_name(TYPE_, BOTTOM);
        case_to_name(TYPE_, TOP);
        case_to_name(TYPE_, LIVE_CONTROL);
        case_to_name(TYPE_, DEAD_CONTROL);
        case_to_name(TYPE_, INTEGER);
        case_to_name(TYPE_, TUPLE);
        case_to_name(TYPE_, COUNT);
    }
}

fn Hash hash_type(Thread* thread, Type* type)
{
    Hash hash = type->hash;

    if (!hash)
    {
        hash = type_functions[type->id].get_hash(thread, type);
        // print("Hashing type id {s}: {u64:x}\n", type_id_to_string(type), hash);
    }

    assert(hash != 0);
    type->hash = hash;

    return hash;
}

fn NodeIndex intern_pool_put_node_at_assume_not_existent_assume_capacity(Thread* thread, NodeIndex node, u32 index)
{
    thread->interned.nodes.pointer[index] = *(u32*)&node;
    thread->interned.nodes.length += 1;

    return node;
}

fn NodeIndex intern_pool_put_node_assume_not_existent_assume_capacity(Thread* thread, Hash hash, NodeIndex node)
{
    auto capacity = thread->interned.nodes.capacity;
    assert(thread->interned.nodes.length < capacity);
    auto index = hash & (capacity - 1);

    return intern_pool_put_node_at_assume_not_existent_assume_capacity(thread, node, index);
}

fn NodeIndex intern_pool_put_node_assume_not_existent(Thread* thread, Hash hash, NodeIndex node)
{
    intern_pool_ensure_capacity(&thread->interned.nodes, thread->arena, 1);
    return intern_pool_put_node_assume_not_existent_assume_capacity(thread, hash, node);
}

struct NodeGetOrPut
{
    NodeIndex index;
    u8 existing;
};
typedef struct NodeGetOrPut NodeGetOrPut;

// This assumes the indices are not equal
fn u8 node_equal(Thread* thread, Node* a, Node* b)
{
    u8 result = 0;
    assert(a != b);
    assert(a->hash);
    assert(b->hash);

    if (((a->id == b->id) & (a->hash == b->hash)) & (a->input_count == b->input_count))
    {
        auto inputs_a = node_get_inputs(thread, a);
        auto inputs_b = node_get_inputs(thread, b);
        result = 1;

        for (u16 i = 0; i < a->input_count; i += 1)
        {
            if (!index_equal(inputs_a.pointer[i], inputs_b.pointer[i]))
            {
                result = 0;
                break;
            }
        }

        if (result)
        {
            switch (a->id)
            {
                case NODE_CONSTANT:
                    result = index_equal(a->constant.type, b->constant.type);
                    break;
                default:
                    trap();
            }
        }
    }

    return result;
}


fn u8 node_index_equal(Thread* thread, NodeIndex a, NodeIndex b)
{
    u8 result = 0;
    if (index_equal(a, b))
    {
        result = 1;
    }
    else
    {
        auto* node_a = thread_node_get(thread, a);
        auto* node_b = thread_node_get(thread, b);
        assert(node_a != node_b);
        result = node_equal(thread, node_a, node_b);
    }

    return result;
}

fn s32 intern_pool_find_node_slot(Thread* thread, u32 original_index, NodeIndex node_index)
{
    assert(validi(node_index));
    auto it_index = original_index;
    auto existing_capacity = thread->interned.nodes.capacity;
    s32 result = -1;
    auto* node = thread_node_get(thread, node_index);

    for (u32 i = 0; i < existing_capacity; i += 1)
    {
        auto index = it_index & (existing_capacity - 1);
        u32 key = thread->interned.nodes.pointer[index];

        if (key == 0)
        {
            result = index;
            break;
        }
        else
        {
            NodeIndex existing_node_index = *(NodeIndex*)&key;
            // Exhaustive comparation, shortcircuit when possible
            if (node_index_equal(thread, existing_node_index, node_index))
            {
                result = index;
                break;
            }
        }

        it_index += 1;
    }

    return result;
}

fn Hash hash_node(Thread* thread, Node* node, NodeIndex node_index)
{
    auto hash = node->hash;
    if (!hash)
    {
        hash = node_functions[node->id].get_hash(thread, node, node_index);
        // print("[HASH #{u32}] Received hash from callback: {u64:x}\n", node_index.index, hash);

        hash = hash_byte(hash, node->id);


        auto inputs = node_get_inputs(thread, node);
        for (u32 i = 0; i < inputs.length; i += 1)
        {
            auto input_index = inputs.pointer[i];
            if (validi(input_index))
            {
                for (u8* it = (u8*)&input_index; it < (u8*)(&input_index + 1); it += 1)
                {
                    hash = hash_byte(hash, *it);
                }
            }
        }

        // print("[HASH] Node #{u32}, {s}: {u64:x}\n", node_index.index, node_id_to_string(node), hash);

        node->hash = hash;
    }

    assert(hash);

    return hash;
}

fn NodeGetOrPut intern_pool_get_or_put_node(Thread* thread, NodeIndex node_index)
{
    auto existing_capacity = thread->interned.nodes.capacity;
    auto* node = &thread->buffer.nodes.pointer[geti(node_index)];
    auto hash = hash_node(thread, node, node_index);
    auto original_index = hash & (existing_capacity - 1);
    
    auto slot = intern_pool_find_node_slot(thread, original_index, node_index);
    if (slot != -1)
    {
        u32 index = slot;
        auto* existing_ptr = &thread->interned.nodes.pointer[index];
        NodeIndex existing_value = *(NodeIndex*)existing_ptr;
        u8 existing = validi(existing_value);
        NodeIndex new_value = existing_value;
        if (!existing)
        {
            new_value = intern_pool_put_node_at_assume_not_existent_assume_capacity(thread, node_index, index);
            assert(!index_equal(new_value, existing_value));
            assert(index_equal(new_value, node_index));
        }
        return (NodeGetOrPut) {
            .index = new_value,
            .existing = existing,
        };
    }
    else
    {
        if (thread->interned.nodes.length < existing_capacity)
        {
            trap();
        }
        else if (thread->interned.nodes.length == existing_capacity)
        {
            auto result = intern_pool_put_node_assume_not_existent(thread, hash, node_index);
            return (NodeGetOrPut) {
                .index = result,
                .existing = 0,
            };
        }
        else
        {
            trap();
        }
    }
}

fn NodeIndex intern_pool_remove_node(Thread* thread, NodeIndex node_index)
{
    auto existing_capacity = thread->interned.nodes.capacity;
    auto* node = thread_node_get(thread, node_index);
    auto hash = hash_node(thread, node, node_index);
    
    auto original_index = hash & (existing_capacity - 1);
    auto slot = intern_pool_find_node_slot(thread, original_index, node_index);
    if (slot != -1)
    {
        auto slot_index = (u32)slot;
        auto* slot_pointer = &thread->interned.nodes.pointer[slot_index];
        auto old_node_index = *(NodeIndex*)slot_pointer;
        *slot_pointer = 0;

        auto index = slot_index;

        while (1)
        {
            index = (index + 1) & (existing_capacity - 1);
            auto existing = thread->interned.nodes.pointer[index];
            if (existing == 0)
            {
                break;
            }
            auto existing_node_index = *(NodeIndex*)&existing;
            auto* existing_node = thread_node_get(thread, existing_node_index);
            auto existing_node_hash = hash_node(thread, existing_node, existing_node_index);
            auto existing_index = existing_node_hash & (existing_capacity - 1);

            if (slot_index <= existing_index)
            {
                if ((slot_index < existing_index) & (existing_index <= index))
                {
                    continue;
                }
            }
            else
            {
                if ((slot_index <= index) | (slot_index < existing_index))
                {
                    continue;
                }
            }

            trap();
        }

        return old_node_index;
    }
    else
    {
        trap();
    }
}


global String test_files[] = {
    strlit("tests/first/main.nat"),
};

struct Parser
{
    u64 i;
    u32 line;
    u32 column;
};
typedef struct Parser Parser;

[[gnu::hot]] fn void skip_space(Parser* parser, String src)
{
    u64 original_i = parser->i;

    if (original_i != src.length)
    {
        if (is_space(src.pointer[original_i], get_next_ch_safe(src, original_i)))
        {
            while (parser->i < src.length)
            {
                u64 index = parser->i;
                u8 ch = src.pointer[index];
                u64 new_line = ch == '\n';
                parser->line += new_line;

                if (new_line)
                {
                    parser->column = index + 1;
                }

                if (!is_space(ch, get_next_ch_safe(src, parser->i)))
                {
                    break;
                }

                u32 is_comment = src.pointer[index] == '/';
                parser->i += is_comment + is_comment;
                if (is_comment)
                {
                    while (parser->i < src.length)
                    {
                        if (src.pointer[parser->i] == '\n')
                        {
                            break;
                        }

                        parser->i += 1;
                    }

                    continue;
                }

                parser->i += 1;
            }
        }
    }
}

[[gnu::hot]] fn void expect_character(Parser* parser, String src, u8 expected_ch)
{
    u64 index = parser->i;
    if (expect(index < src.length, 1))
    {
        u8 ch = src.pointer[index];
        u64 matches = ch == expected_ch;
        expect(matches, 1);
        parser->i += matches;
        if (!matches)
        {
            print_string(strlit("expected character '"));
            print_string(ch_to_str(expected_ch));
            print_string(strlit("', but found '"));
            print_string(ch_to_str(ch));
            print_string(strlit("'\n"));
            fail();
        }
    }
    else
    {
        print_string(strlit("expected character '"));
        print_string(ch_to_str(expected_ch));
        print_string(strlit("', but found end of file\n"));
        fail();
    }
}

[[gnu::hot]] fn String parse_identifier(Parser* parser, String src)
{
    u64 identifier_start_index = parser->i;
    u64 is_string_literal = src.pointer[identifier_start_index] == '"';
    parser->i += is_string_literal;
    u8 identifier_start_ch = src.pointer[parser->i];
    u64 is_valid_identifier_start = is_identifier_start(identifier_start_ch);
    parser->i += is_valid_identifier_start;

    if (expect(is_valid_identifier_start, 1))
    {
        while (parser->i < src.length)
        {
            u8 ch = src.pointer[parser->i];
            u64 is_identifier = is_identifier_ch(ch);
            expect(is_identifier, 1);
            parser->i += is_identifier;

            if (!is_identifier)
            {
                if (expect(is_string_literal, 0))
                {
                    expect_character(parser, src, '"');
                }

                String result = s_get_slice(u8, src, identifier_start_index, parser->i - is_string_literal);
                return result;
            }
        }

        fail();
    }
    else
    {
        fail();
    }
}

typedef struct Parser Parser;

#define array_start '['
#define array_end ']'

#define argument_start '('
#define argument_end ')'

#define block_start '{'
#define block_end '}'

#define pointer_sign '*'

fn void thread_add_job(Thread* thread, NodeIndex node_index)
{
    trap();
}

fn void thread_add_jobs(Thread* thread, Slice(NodeIndex) nodes)
{
    for (u32 i = 0; i < nodes.length; i += 1)
    {
        NodeIndex node_index = nodes.pointer[i];
        thread_add_job(thread, node_index);
    }
}

fn u8 type_is_simple(Type* type)
{
    return type->id <= TYPE_DEAD_CONTROL;
}

fn TypeIndex type_meet(Thread* thread, TypeIndex a, TypeIndex b)
{
    TypeIndex result = invalidi(Type);
    if (index_equal(a, b))
    {
        result = a;
    }
    else
    {
        Type* a_type = thread_type_get(thread, a);
        Type* b_type = thread_type_get(thread, b);
        TypeIndex left = invalidi(Type);
        TypeIndex right = invalidi(Type);

        assert(a_type != b_type);
        if (a_type->id == b_type->id)
        {
            left = a;
            right = b;
        }
        else if (type_is_simple(a_type))
        {
            left = a;
            right = b;
        }
        else if (type_is_simple(b_type))
        {
            trap();
        }
        else
        {
            result = thread->types.bottom;
        }

        assert(!!validi(left) == !!validi(right));
        assert((validi(left) & validi(right)) | (validi(result)));

        if (validi(left))
        {
            assert(!validi(result));
            auto* left_type = thread_type_get(thread, left);
            auto* right_type = thread_type_get(thread, left);
            switch (left_type->id)
            {
                case TYPE_INTEGER:
                    {
                        auto integer_bot = thread->types.integer.bottom;
                        auto integer_top = thread->types.integer.top;
                        if (index_equal(left, integer_bot))
                        {
                            result = left; 
                        }
                        else if (index_equal(right, integer_bot))
                        {
                            result = right; 
                        }
                        else if (index_equal(right, integer_top))
                        {
                            result = left; 
                        }
                        else if (index_equal(left, integer_top))
                        {
                            result = right; 
                        }
                        else
                        {
                            result = integer_bot;
                        }
                    } break;
                case TYPE_BOTTOM:
                    {
                        assert(type_is_simple(left_type));
                        if ((left_type->id == TYPE_BOTTOM) | (right_type->id == TYPE_TOP))
                        {
                            result = left;
                        }
                        else if ((left_type->id == TYPE_TOP) | (right_type->id == TYPE_BOTTOM))
                        {
                            result = right;
                        }
                        else if (!type_is_simple(right_type))
                        {
                            result = thread->types.bottom;
                        }
                        else if (left_type->id == TYPE_LIVE_CONTROL)
                        {
                            result = thread->types.live_control;
                        }
                        else
                        {
                            result = thread->types.dead_control;
                        }
                    } break;
                default:
                    trap();
            }
        }
    }

    assert(validi(result));

    return result;
}

fn u8 type_is_a(Thread* thread, TypeIndex a, TypeIndex b)
{
    auto m = type_meet(thread, a, b);
    return index_equal(m, b);
}
union NodePair
{
    struct
    {
        NodeIndex old;
        NodeIndex new;
    };
    NodeIndex nodes[2];
};
typedef union NodePair NodePair;

fn NodeIndex node_keep(Thread* thread, NodeIndex node_index)
{
    return node_add_output(thread, node_index, invalidi(Node));
}

fn NodeIndex node_unkeep(Thread* thread, NodeIndex node_index)
{
    node_remove_output(thread, node_index, invalidi(Node));
    return node_index;
}

fn void node_kill(Thread* thread, NodeIndex node_index)
{
    node_unlock(thread, node_index);
    auto* node = thread_node_get(thread, node_index);
    print("[NODE KILLING] (#{u32}, {s}) START\n", node_index.index, node_id_to_string(node));
    assert(node_is_unused(node));
    node->type = invalidi(Type);

    auto inputs = node_get_inputs(thread, node);
    while (node->input_count > 0)
    {
        auto input_index = node->input_count - 1;
        node->input_count = input_index;
        auto old_input_index = inputs.pointer[input_index];

        print("[NODE KILLING] (#{u32}, {s}) Removing input #{u32} at slot {u32}\n", node_index.index, node_id_to_string(node), old_input_index.index, input_index);
        if (validi(old_input_index))
        {
            thread_worklist_push(thread, old_input_index);
            u8 no_more_outputs = node_remove_output(thread, old_input_index, node_index);
            if (no_more_outputs)
            {
                print("[NODE KILLING] (#{u32}, {s}) (NO MORE OUTPUTS - KILLING) Input #{u32}\n", node_index.index, node_id_to_string(node), old_input_index.index);
                node_kill(thread, old_input_index);
            }
        }
    }

    assert(node_is_dead(node));
    // print("[NODE KILLING] (#{u32}, {s}) END\n", node_index.index, node_id_to_string(node));
}

fn NodeIndex dead_code_elimination(Thread* thread, NodePair nodes)
{
    NodeIndex old = nodes.old;
    NodeIndex new = nodes.new;

    if (!index_equal(old, new))
    {
        print("[DCE] old: #{u32} != new: #{u32}. Proceeding to eliminate\n", old.index, new.index);
        auto* old_node = thread_node_get(thread, old);
        if (node_is_unused(old_node) & !node_is_dead(old_node))
        {
            node_keep(thread, new);
            node_kill(thread, old);
            node_unkeep(thread, new);
        }
    }

    return new;
}

fn u8 type_is_high_or_const(Thread* thread, TypeIndex type_index)
{
    u8 result = index_equal(type_index, thread->types.top) | index_equal(type_index, thread->types.dead_control);
    if (!result)
    {
        Type* type = thread_type_get(thread, type_index);
        switch (type->id)
        {
            case TYPE_INTEGER:
                result = type->integer.is_constant | (type->integer.constant == 0);
                break;
            default:
                break;
        }
    }

    return result;
}

fn TypeIndex type_join(Thread* thread, TypeIndex a, TypeIndex b)
{
    TypeIndex result;
    if (index_equal(a, b))
    {
        result = a;
    }
    else
    {
        trap();
    }

    return result;
}

fn void node_set_type(Thread* thread, Node* node, TypeIndex new_type)
{
    auto old_type = node->type;
    assert(!validi(old_type) || type_is_a(thread, new_type, old_type));
    if (!index_equal(old_type, new_type))
    {
        node->type = new_type;
        auto outputs = node_get_outputs(thread, node);
        thread_add_jobs(thread, outputs);
        move_dependencies_to_worklist(thread, node);
    }
}

global auto enable_peephole = 1;

fn NodeIndex peephole_optimize(Thread* thread, Function* function, NodeIndex node_index)
{
    assert(enable_peephole);
    auto result = node_index;
    auto* node = thread_node_get(thread, node_index);
    print("Peepholing node #{u32} ({s})\n", node_index.index, node_id_to_string(node));
    auto old_type = node->type;
    auto new_type = node_functions[node->id].compute_type(thread, node_index);

    if (enable_peephole)
    {
        thread->iteration.total += 1;
        node_set_type(thread, node, new_type);

        if (node->id != NODE_CONSTANT && node->id != NODE_DEAD_CONTROL && type_is_high_or_const(thread, node->type))
        {
            if (index_equal(node->type, thread->types.dead_control))
            {
                trap();
            }
            else
            {
                auto constant_node = constant_int_create_with_type(thread, function, node->type);
                return constant_node;
            }
        }

        auto idealize = 1;
        if (!node->hash)
        {
            auto gop = intern_pool_get_or_put_node(thread, node_index);
            idealize = !gop.existing;

            if (gop.existing) 
            {
                auto interned_node_index = gop.index;
                auto* interned_node = thread_node_get(thread, interned_node_index);
                auto new_type = type_join(thread, interned_node->type, node->type);
                node_set_type(thread, interned_node, new_type);
                node->hash = 0;
                print("[peephole_optimize] Eliminating #{u32} because an existing node was found: #{u32}\n", node_index.index, interned_node_index.index);
                auto dce_node = dead_code_elimination(thread, (NodePair) {
                    .old = node_index,
                    .new = interned_node_index,
                });

                result = dce_node;
            }
        }

        if (idealize)
        {
            auto idealized_node = node_functions[node->id].idealize(thread, node_index);
            if (validi(idealized_node))
            {
                result = idealized_node;
            }
            else
            {
                u64 are_types_equal = index_equal(new_type, old_type);
                thread->iteration.nop += are_types_equal;
                
                result = are_types_equal ? invalidi(Node) : node_index;
            }
        }
    }
    else
    {
        node->type = new_type;
    }

    return result;
}

fn NodeIndex peephole(Thread* thread, Function* function, NodeIndex node_index)
{
    NodeIndex result;
    if (enable_peephole)
    {
        NodeIndex new_node = peephole_optimize(thread, function, node_index);
        if (validi(new_node))
        {
            NodeIndex peephole_new_node = peephole(thread, function, new_node);
            print("[peephole] Eliminating #{u32} because a better node was found: #{u32}\n", node_index.index, new_node.index);
            auto dce_node = dead_code_elimination(thread, (NodePair)
            {
                .old = node_index,
                .new = peephole_new_node,
            });

            result = dce_node;
        }
        else
        {
            result = node_index;
        }
    }
    else
    {
        auto* node = thread_node_get(thread, node_index);
        auto new_type = node_functions[node->id].compute_type(thread, node_index);
        node->type = new_type;
        result = node_index;
    }

    return result;
}


fn TypeIndex analyze_type(Thread* thread, Parser* parser, String src)
{
    u64 start_index = parser->i;
    u8 start_ch = src.pointer[start_index];
    u32 is_array_start = start_ch == array_start;
    u32 u_start = start_ch == 'u';
    u32 s_start = start_ch == 's';
    u32 float_start = start_ch == 'f';
    u32 void_start = start_ch == 'v';
    u32 pointer_start = start_ch == pointer_sign;
    u32 integer_start = u_start | s_start;
    u32 number_start = integer_start | float_start;

    if (void_start)
    {
        trap();
    }
    else if (is_array_start)
    {
        trap();
    }
    else if (pointer_start)
    {
        trap();
    }
    else if (number_start)
    {
        u64 expected_digit_start = start_index + 1;
        u64 i = expected_digit_start;
        u32 decimal_digit_count = 0;
        u64 top = i + 5;

        while (i < top)
        {
            u8 ch = src.pointer[i];
            u32 is_digit = is_decimal_digit(ch);
            decimal_digit_count += is_digit;
            if (!is_digit)
            {
                u32 is_alpha = is_alphabetic(ch);
                if (is_alpha)
                {
                    decimal_digit_count = 0;
                }
                break;
            }

            i += 1;
        }


        if (decimal_digit_count)
        {
            parser->i += 1;

            if (integer_start)
            {
                u64 signedness = s_start;
                u64 bit_size;
                u64 current_i = parser->i;
                assert(src.pointer[current_i] >= '0' & src.pointer[current_i] <= '9');
                switch (decimal_digit_count) {
                    case 0:
                        fail();
                    case 1:
                        bit_size = src.pointer[current_i] - '0';
                        break;
                    case 2:
                        bit_size = (src.pointer[current_i] - '0') * 10 + (src.pointer[current_i + 1] - '0');
                        break;
                    default:
                        fail();
                }
                parser->i += decimal_digit_count;

                assert(!is_decimal_digit(src.pointer[parser->i]));

                if (bit_size)
                {
                    auto type_index = thread_get_integer_type(thread, (TypeInteger) {
                        .constant = 0,
                        .bit_count = bit_size,
                        .is_constant = 0,
                        .is_signed = signedness,
                    });
                    return type_index;
                }
                else
                {
                    fail();
                }
            }
            else if (float_start)
            {
                trap();
            }
            else
            {
                trap();
            }
        }
        else
        {
            fail();
        }
    }

    trap();
}

fn NodeIndex analyze_addition(Thread* thread, Parser* parser, Function* function, String src);
fn NodeIndex analyze_multiplication(Thread* thread, Parser* parser, Function* function, String src);

fn NodeIndex analyze_primary_expression(Thread* thread, Parser* parser, Function* function, String src)
{
    u8 starting_ch = src.pointer[parser->i];
    u64 is_digit = is_decimal_digit(starting_ch);
    if (is_digit)
    {
        typedef enum IntegerPrefix {
            INTEGER_PREFIX_HEXADECIMAL,
            INTEGER_PREFIX_DECIMAL,
            INTEGER_PREFIX_OCTAL,
            INTEGER_PREFIX_BINARY,
        } IntegerPrefix;
        IntegerPrefix prefix = INTEGER_PREFIX_DECIMAL;
        u64 value = 0;

        if (starting_ch == '0')
        {
            auto follow_up_character = src.pointer[parser->i + 1];
            auto is_hex_start = follow_up_character == 'x';
            auto is_octal_start = follow_up_character == 'o';
            auto is_bin_start = follow_up_character == 'b';
            auto is_prefixed_start = is_hex_start | is_octal_start | is_bin_start;
            auto follow_up_alpha = is_alphabetic(follow_up_character);
            auto follow_up_digit = is_decimal_digit(follow_up_character);
            auto is_valid_after_zero = is_space(follow_up_character, get_next_ch_safe(src, follow_up_character)) | (!follow_up_digit & !follow_up_alpha);

            if (is_prefixed_start) {
                switch (follow_up_character) {
                    case 'x': prefix = INTEGER_PREFIX_HEXADECIMAL; break;
                    case 'o': prefix = INTEGER_PREFIX_OCTAL; break;
                    case 'd': prefix = INTEGER_PREFIX_DECIMAL; break;
                    case 'b': prefix = INTEGER_PREFIX_BINARY; break;
                    default: fail();
                };

                parser->i += 2;

            } else if (!is_valid_after_zero) {
                fail();
            }
        }

        auto start = parser->i;

        switch (prefix) {
            case INTEGER_PREFIX_HEXADECIMAL:
                {
                    // while (is_hex_digit(src[parser->i])) {
                    //     parser->i += 1;
                    // }

                    trap();
                    // auto slice = src.slice(start, parser->i);
                    // value = parse_hex(slice);
                } break;
            case INTEGER_PREFIX_DECIMAL:
                {
                    while (is_decimal_digit(src.pointer[parser->i]))
                    {
                        parser->i += 1;
                    }

                    value = parse_decimal(s_get_slice(u8, src, start, parser->i));
                } break;
            case INTEGER_PREFIX_OCTAL:
                trap();
            case INTEGER_PREFIX_BINARY:
                trap();
        }

        auto node_index = constant_int_create(thread, function, value);
        return node_index;
    }
    else
    {
        trap();
    }
}

fn NodeIndex analyze_unary(Thread* thread, Parser* parser, Function* function, String src)
{
    // TODO: postfix
    typedef enum PrefixOperator
    {
        PREFIX_OPERATOR_NONE = 0,
        PREFIX_OPERATOR_NEGATION,
        PREFIX_OPERATOR_LOGICAL_NOT,
        PREFIX_OPERATOR_BITWISE_NOT,
        PREFIX_OPERATOR_ADDRESS_OF,
    } PrefixOperator;

    PrefixOperator prefix_operator;
    NodeIndex node_index;

    switch (src.pointer[parser->i])
    {
        case '-':
            todo();
        case '!':
            todo();
        case '~':
            todo();
        case '&':
            todo();
        default:
            {
                node_index = analyze_primary_expression(thread, parser, function, src);
                prefix_operator = PREFIX_OPERATOR_NONE;
            } break;
    }

    typedef enum SuffixOperator
    {
        SUFFIX_OPERATOR_NONE = 0,
        SUFFIX_OPERATOR_CALL,
        SUFFIX_OPERATOR_ARRAY,
        SUFFIX_OPERATOR_FIELD,
        SUFFIX_OPERATOR_POINTER_DEREFERENCE,
    } SuffixOperator;

    SuffixOperator suffix_operator;

    skip_space(parser, src);

    switch (src.pointer[parser->i])
    {
        case argument_start:
            todo();
        case array_start:
            todo();
        case '.':
            todo();
        default:
            break;
    }

    if (prefix_operator != PREFIX_OPERATOR_NONE)
    {
        todo();
    }

    return node_index;
}

fn NodeIndex analyze_multiplication(Thread* thread, Parser* parser, Function* function, String src)
{
    auto left = analyze_unary(thread, parser, function, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        auto skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '*':
                node_id = NODE_INTEGER_ADD;
                break;
            case '/':
                node_id = NODE_INTEGER_SUBSTRACT;
                break;
            case '%':
                todo();
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT) 
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_multiplication(thread, parser, function, src);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, function, new_node_index);
    }

    print("Analyze addition returned node #{u32}\n", left.index);

    return left;
}

fn NodeIndex analyze_addition(Thread* thread, Parser* parser, Function* function, String src)
{
    auto left = analyze_multiplication(thread, parser, function, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;

        switch (src.pointer[parser->i])
        {
            case '+':
                node_id = NODE_INTEGER_ADD;
                break;
            case '-':
                node_id = NODE_INTEGER_SUBSTRACT;
                break;
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT) 
        {
            break;
        }

        parser->i += 1;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_multiplication(thread, parser, function, src);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, function, new_node_index);
    }

    print("Analyze addition returned node #{u32}\n", left.index);

    return left;
}

fn NodeIndex analyze_shift(Thread* thread, Parser* parser, Function* function, String src)
{
    auto left = analyze_addition(thread, parser, function, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;

        if ((src.pointer[parser->i] == '<') & (src.pointer[parser->i + 1] == '<'))
        {
            node_id = NODE_INTEGER_SIGNED_SHIFT_LEFT;
        }
        else if ((src.pointer[parser->i] == '>') & (src.pointer[parser->i + 1] == '>'))
        {
            node_id = NODE_INTEGER_SIGNED_SHIFT_RIGHT;
        }
        else
        {
            break;
        }

        parser->i += 2;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_addition(thread, parser, function, src);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_bitwise_binary(Thread* thread, Parser* parser, Function* function, String src)
{
    auto left = analyze_shift(thread, parser, function, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        auto skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '&':
                node_id = NODE_INTEGER_AND;
                break;
            case '|':
                node_id = NODE_INTEGER_OR;
                break;
            case '^':
                node_id = NODE_INTEGER_XOR;
                break;
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT)
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_shift(thread, parser, function, src);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_comparison(Thread* thread, Parser* parser, Function* function, String src)
{
    auto left = analyze_bitwise_binary(thread, parser, function, src);

    while (1)
    {
        skip_space(parser, src);

        NodeId node_id;
        auto skip_count = 1;

        switch (src.pointer[parser->i])
        {
            case '=':
                todo();
            case '!':
                todo();
            case '<':
                todo();
            case '>':
                todo();
            default:
                node_id = NODE_COUNT;
                break;
        }

        if (node_id == NODE_COUNT)
        {
            break;
        }

        parser->i += skip_count;
        skip_space(parser, src);

        auto new_node_index = thread_node_add(thread, (NodeCreate) {
            .id = node_id,
            .inputs = array_to_slice(((NodeIndex[]) {
                invalidi(Node),
                left,
                invalidi(Node),
            })),
        });

        // print("Before right: LEFT is #{u32}\n", left.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));
        auto right = analyze_bitwise_binary(thread, parser, function, src);
        // print("Addition: left: #{u32}, right: #{u32}\n", left.index, right.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        node_set_input(thread, new_node_index, 2, right);

        // print("Addition new node #{u32}\n", new_node_index.index);
        // print("Left code:\n```\n{s}\n```\n", s_get_slice(u8, src, parser->i, src.length));

        left = peephole(thread, function, new_node_index);
    }

    return left;
}

fn NodeIndex analyze_expression(Thread* thread, Parser* parser, Function* function, String src, TypeIndex result_type)
{
    NodeIndex result = analyze_comparison(thread, parser, function, src);
    return result;
}

fn void analyze_block(Thread* thread, Parser* parser, FunctionBuilder* builder, String src)
{
    expect_character(parser, src, block_start);

    scope_push(thread, builder);

    Function* function = builder->function;

    while (1)
    {
        skip_space(parser, src);

        if (s_get(src, parser->i) == block_end)
        {
            break;
        }

        u64 statement_start_index = parser->i;
        u8 statement_start_ch = src.pointer[parser->i];
        if (is_identifier_start(statement_start_ch))
        {
            String statement_start_identifier = parse_identifier(parser, src); 
            if (s_equal(statement_start_identifier, (strlit("return"))))
            {
                skip_space(parser, src);
                NodeIndex return_value = analyze_expression(thread, parser, function, src, function->return_type);
                skip_space(parser, src);
                expect_character(parser, src, ';');

                auto return_node_index = thread_node_add(thread, (NodeCreate) {
                        .id = NODE_RETURN,
                        .inputs = array_to_slice(((NodeIndex[]) {
                                    builder_get_control_node_index(thread, builder),
                                    return_value,
                                    })),
                        });

                if (validi(builder->scope))
                {
                    // TODO:
                    // Look for memory slices
                    // todo();
                }

                return_node_index = peephole(thread, function, return_node_index);

                builder_add_return(thread, builder, return_node_index);

                builder_set_control(thread, builder, builder->dead_control);
            }
            else
            {
                trap();
            }
        }
        else
        {
            trap();
        }
    }

    expect_character(parser, src, block_end);

    scope_pop(thread, builder);
}

fn void analyze_file(Thread* thread, File* file)
{
    Parser p = {};
    Parser* parser = &p;
    String src = file->source;

    while (1)
    {
        skip_space(parser, src);

        if (parser->i == src.length)
        {
            break;
        }

        // Parse top level declaration
        u64 start_ch_index = parser->i;
        u8 start_ch = s_get(src, start_ch_index);

        u64 is_identifier = is_identifier_start(start_ch);

        if (is_identifier)
        {
            u8 next_ch = get_next_ch_safe(src, start_ch_index);
            u64 is_fn = (start_ch == 'f') & (next_ch == 'n');

            if (is_fn)
            {
                parser->i += 2;

                FunctionBuilder function_builder = {};
                FunctionBuilder* builder = &function_builder;
                builder->file = file;

                skip_space(parser, src);


                Function* function = vb_add(&thread->buffer.functions, 1);
                memset(function, 0, sizeof(Function));
                builder->function = function;

                function->name = parse_identifier(parser, src);
                if (s_equal(function->name, strlit("main")))
                {
                    thread->main_function = thread->buffer.functions.length - 1;
                }

                skip_space(parser, src);
                
                // Parse arguments
                u32 argument_count = 0;
                expect_character(parser, src, argument_start);
                // TODO: arguments
                skip_space(parser, src);
                expect_character(parser, src, argument_end);
                skip_space(parser, src);

                {
                    // Create the start node early since it is needed as a dependency for control and arguments
                    function->start = thread_node_add(thread, (NodeCreate) {
                        .id = NODE_START,
                        .inputs = {},
                    });
                    TypeIndex tuple = invalidi(Type);
                    if (argument_count)
                    {
                        Slice(TypeIndex) start_argument_types = arena_allocate_slice(thread->arena, TypeIndex, argument_count + 1);
                        u32 argument_i = 0;
                        start_argument_types.pointer[argument_i] = thread->types.live_control;
                        argument_i += 1;

                        for (u32 i = 0; i < argument_count; i += 1)
                        {
                            trap();
                        }
                    }
                    else
                    {
                        auto start_argument_types = (Slice(TypeIndex)) {
                            .pointer = &thread->types.live_control,
                                .length = 1,
                        };
                        tuple = type_make_tuple(thread, start_argument_types).index;
                    }

                    {
                        auto* start_node = thread_node_get(thread, function->start);
                        assert(validi(tuple));
                        start_node->type = tuple;
                        start_node->start.arguments = tuple;
                    }
                }

                // Create stop node
                {
                    function->stop = thread_node_add(thread, (NodeCreate) {
                        .id = NODE_STOP,
                        .inputs = {},
                    });
                }

                auto dead_control = thread_node_add(thread, (NodeCreate) {
                            .id = NODE_DEAD_CONTROL,
                            .inputs = { .pointer = &function->start, .length = 1 },
                        });
                dead_control = peephole(thread, function, dead_control);

                builder->dead_control = node_keep(thread, dead_control);

                // Create the function scope node
                {
                    auto scope_node_index = thread_node_add(thread, (NodeCreate)
                            {
                                .id = NODE_SCOPE,
                                .inputs = {},
                            });
                    auto* scope_node = thread_node_get(thread, scope_node_index);
                    scope_node->type = thread->types.bottom;
                    builder->scope = scope_node_index;

                    scope_push(thread, builder);
                    auto control_node_index = thread_node_add(thread, (NodeCreate){
                        .id = NODE_CONTROL_PROJECTION,
                        .inputs = {
                            .pointer = &function->start,
                            .length = 1,
                        },
                    });
                    auto* control_node = thread_node_get(thread, control_node_index);
                    auto control_name = strlit("$control");
                    control_node->control_projection.projection = (NodeProjection) {
                        .label = control_name,
                        .index = 0,
                    };
                    control_node_index = peephole(thread, function, control_node_index);
                    scope_define(thread, builder, control_name, thread->types.live_control, control_node_index);
                }

                function->return_type = analyze_type(thread, parser, src);

                skip_space(parser, src);

                analyze_block(thread, parser, builder, src);

                scope_pop(thread, builder);
                function->stop = peephole(thread, function, function->stop);
            }
            else
            {
                trap();
            }
        }
        else
        {
            trap();
        }
    }
}

typedef NodeIndex NodeCallback(Thread* thread, Function* function, NodeIndex node_index);

fn NodeIndex node_walk_internal(Thread* thread, Function* function, NodeIndex node_index, NodeCallback* callback)
{
    if (bitset_get(&thread->worklist.visited, geti(node_index)))
    {
        return invalidi(Node);
    }
    else
    {
        bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
        auto callback_result = callback(thread, function, node_index);
        if (validi(callback_result))
        {
            return callback_result;
        }

        auto* node = thread_node_get(thread, node_index);
        auto inputs = node_get_inputs(thread, node);
        auto outputs = node_get_outputs(thread, node);

        for (u64 i = 0; i < inputs.length; i += 1)
        {
            auto n = inputs.pointer[i];
            if (validi(n))
            {
                auto n_result = node_walk_internal(thread, function, n, callback);
                if (validi(n_result))
                {
                    return n_result;
                }
            }
        }

        for (u64 i = 0; i < outputs.length; i += 1)
        {
            auto n = outputs.pointer[i];
            if (validi(n))
            {
                auto n_result = node_walk_internal(thread, function, n, callback);
                if (validi(n_result))
                {
                    return n_result;
                }
            }
        }

        return invalidi(Node);
    }
}

fn NodeIndex node_walk(Thread* thread, Function* function, NodeIndex node_index, NodeCallback* callback)
{
    assert(thread->worklist.visited.length == 0);
    NodeIndex result = node_walk_internal(thread, function, node_index, callback);
    bitset_clear(&thread->worklist.visited);
    return result;
}

fn NodeIndex progress_on_list_callback(Thread* thread, Function* function, NodeIndex node_index)
{
    if (bitset_get(&thread->worklist.bitset, geti(node_index)))
    {
        return invalidi(Node);
    }
    else
    {
        NodeIndex new_node = peephole_optimize(thread, function, node_index);
        return new_node;
    }
}

fn u8 progress_on_list(Thread* thread, Function* function, NodeIndex stop_node)
{
    thread->worklist.mid_assert = 1;

    NodeIndex changed = node_walk(thread, function, stop_node, &progress_on_list_callback);

    thread->worklist.mid_assert = 0;

    return !validi(changed);
}

fn void iterate_peepholes(Thread* thread, Function* function, NodeIndex stop_node_index)
{
    assert(progress_on_list(thread, function, stop_node_index));
    if (thread->worklist.nodes.length > 0)
    {
        while (1)
        {
            auto node_index = thread_worklist_pop(thread);
            if (!validi(node_index))
            {
                break;
            }

            auto* node = thread_node_get(thread, node_index);
            if (!node_is_dead(node))
            {
                auto new_node_index = peephole_optimize(thread, function, node_index);
                if (validi(new_node_index))
                {
                    trap();
                }
            }
        }
    }

    thread_worklist_clear(thread);
}

fn u8 node_is_cfg(Node* node)
{
    switch (node->id)
    {
        case NODE_START:
        case NODE_DEAD_CONTROL:
        case NODE_CONTROL_PROJECTION:
        case NODE_RETURN:
        case NODE_STOP:
            return 1;
        case NODE_CONSTANT:
            return 0;
        default:
            trap();
    }
}

fn void rpo_cfg(Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    if (node_is_cfg(node) && !bitset_get(&thread->worklist.visited, geti(node_index)))
    {
        bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
        auto outputs = node_get_outputs(thread, node);
        for (u64 i = 0; i < outputs.length; i += 1)
        {
            auto output = outputs.pointer[i];
            if (validi(output))
            {
                rpo_cfg(thread, output);
            }
        }

        *vb_add(&thread->worklist.nodes, 1) = node_index;
    }
}

fn s32 node_loop_depth(Thread* thread, Node* node)
{
    assert(node_is_cfg(node));
    s32 loop_depth;
    switch (node->id)
    {
        case NODE_START:
            {
                loop_depth = node->start.cfg.loop_depth;
                if (!loop_depth)
                {
                    loop_depth = node->start.cfg.loop_depth = 1;
                }
            } break;
        case NODE_STOP:
            {
                loop_depth = node->stop.cfg.loop_depth;
                if (!loop_depth)
                {
                    loop_depth = node->stop.cfg.loop_depth = 1;
                }
            } break;
        case NODE_RETURN:
            {
                loop_depth = node->return_node.cfg.loop_depth;
                if (!loop_depth)
                {
                    auto input_index = node_input_get(thread, node, 0);
                    auto input = thread_node_get(thread, input_index);
                    node->return_node.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
                }
            } break;
        case NODE_CONTROL_PROJECTION:
            {
                loop_depth = node->control_projection.cfg.loop_depth;
                if (!loop_depth)
                {
                    auto input_index = node_input_get(thread, node, 0);
                    auto input = thread_node_get(thread, input_index);
                    node->control_projection.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
                }
            } break;
        case NODE_DEAD_CONTROL:
            {
                loop_depth = node->dead_control.cfg.loop_depth;
                if (!loop_depth)
                {
                    auto input_index = node_input_get(thread, node, 0);
                    auto input = thread_node_get(thread, input_index);
                    node->dead_control.cfg.loop_depth = loop_depth = node_loop_depth(thread, input);
                }
            } break;
        default:
            trap();
    }

    return loop_depth;
}

fn u8 node_is_region(Node* node)
{
    return (node->id == NODE_REGION) | (node->id == NODE_REGION_LOOP);
}

fn u8 node_is_pinned(Node* node)
{
    switch (node->id)
    {
        case NODE_START:
            return 1;
        case NODE_CONSTANT:
            return 0;
        default:
            trap();
    }
}

fn void schedule_early(Thread* thread, NodeIndex node_index, NodeIndex start_node)
{
    if (validi(node_index) && !bitset_get(&thread->worklist.visited, geti(node_index)))
    {
        bitset_set_value(&thread->worklist.visited, geti(node_index), 1);
        auto* node = thread_node_get(thread, node_index);
        auto inputs = node_get_inputs(thread, node);
        for (u64 i = 0; i < inputs.length; i += 1)
        {
            auto input = inputs.pointer[i];
            if (validi(input))
            {
                auto* input_node = thread_node_get(thread, input);
                if (!node_is_pinned(input_node))
                {
                    trap();
                }
            }
        }

        if (!node_is_pinned(node))
        {
            auto early = start_node;
            for (u64 i = 1; i < inputs.length; i += 1)
            {
                auto input_index = inputs.pointer[i];
                auto input_node = thread_node_get(thread, input_index);
                auto control_input_index = node_input_get(thread, input_node, 0);
                auto* control_input_node = thread_node_get(thread, control_input_index);
                assert(node_is_cfg(control_input_node));
                trap();
            }
            node_set_input(thread, node_index, 0, early);
        }
    }
}

fn u8 node_cfg_block_head(Node* node)
{
    assert(node_is_cfg(node));
    switch (node->id)
    {
        case NODE_START:
            return 1;
        default:
            trap();
    }
}

fn u8 is_forwards_edge(Thread* thread, NodeIndex output_index, NodeIndex input_index)
{
    u8 result = validi(output_index) & validi(input_index);
    if (result)
    {
        auto* output = thread_node_get(thread, output_index);
        auto* input = thread_node_get(thread, input_index);
        result = output->input_count > 2;
        if (result)
        {
            auto input_index2 = node_input_get(thread, output, 2);

            result = index_equal(input_index2, input_index);
            
            if (result)
            {
                trap();
            }
        }
    }

    return result;
}

fn void schedule_late(Thread* thread, NodeIndex node_index, Slice(NodeIndex) nodes, Slice(NodeIndex) late)
{
    if (!validi(late.pointer[geti(node_index)]))
    {
        auto* node = thread_node_get(thread, node_index);

        if (node_is_cfg(node))
        {
            late.pointer[geti(node_index)] = node_cfg_block_head(node) ? node_index : node_input_get(thread, node, 0);
        }

        if (node->id == NODE_PHI)
        {
            trap();
        }

        auto outputs = node_get_outputs(thread, node);

        for (u32 i = 0; i < outputs.length; i += 1)
        {
            NodeIndex output = outputs.pointer[i];
            if (is_forwards_edge(thread, output, node_index))
            {
                trap();
            }
        }

        for (u32 i = 0; i < outputs.length; i += 1)
        {
            NodeIndex output = outputs.pointer[i];
            if (is_forwards_edge(thread, output, node_index))
            {
                trap();
            }
        }

        if (!node_is_pinned(node))
        {
            trap();
        }
    }
}

fn void gcm_build_cfg(Thread* thread, NodeIndex start_node_index, NodeIndex stop_node_index)
{
    // Fix loops
    {
        // TODO:
    }

    // Schedule early
    rpo_cfg(thread, start_node_index);

    u32 i = thread->worklist.nodes.length;
    while (i > 0)
    {
        i -= 1;
        auto node_index = thread->worklist.nodes.pointer[i];
        auto* node = thread_node_get(thread, node_index);
        node_loop_depth(thread, node);
        auto inputs = node_get_inputs(thread, node);
        for (u64 i = 0; i < inputs.length; i += 1)
        {
            auto input = inputs.pointer[i];
            schedule_early(thread, input, start_node_index);
        }

        if (node_is_region(node))
        {
            trap();
        }
    }

    // Schedule late

    auto max_node_count = thread->buffer.nodes.length;
    auto* alloc = arena_allocate(thread->arena, NodeIndex, max_node_count * 2);
    auto late = (Slice(NodeIndex)) {
        .pointer = alloc,
        .length = max_node_count,
    };
    auto nodes = (Slice(NodeIndex)) {
        .pointer = alloc + max_node_count,
        .length = max_node_count,
    };

    schedule_late(thread, start_node_index, nodes, late);

    for (u32 i = 0; i < late.length; i += 1)
    {
        auto node_index = nodes.pointer[i];
        if (validi(node_index))
        {
            trap();
            auto late_node_index = late.pointer[i];
            node_set_input(thread, node_index, 0, late_node_index);
        }
    }
}

fn void print_function(Thread* thread, Function* function)
{
    print("fn {s}\n====\n", function->name);
    VirtualBuffer(NodeIndex) nodes = {};
    *vb_add(&nodes, 1) = function->stop;
    auto start = function->start;

    while (1)
    {
        auto node_index = nodes.pointer[nodes.length - 1];
        auto* node = thread_node_get(thread, node_index);

        if (node->input_count)
        {
            for (u32 i = 1; i < node->input_count; i += 1)
            {
                *vb_add(&nodes, 1) = node_input_get(thread, node, 1);
            }
            *vb_add(&nodes, 1) = node_input_get(thread, node, 0);
        }
        else
        {
            break;
        }
    }

    u32 i = nodes.length;
    while (i > 0)
    {
        i -= 1;

        auto node_index = nodes.pointer[i];
        auto* node = thread_node_get(thread, node_index);
        auto* type = thread_type_get(thread, node->type);
        print("%{u32} - {s} - {s} ", geti(node_index), type_id_to_string(type), node_id_to_string(node));
        auto inputs = node_get_inputs(thread, node);
        auto outputs = node_get_outputs(thread, node);
        
        print("(INPUTS: { ");
        for (u32 i = 0; i < inputs.length; i += 1)
        {
            auto input_index = inputs.pointer[i];
            print("%{u32} ", geti(input_index));
        }
        print("} OUTPUTS: { ");
        for (u32 i = 0; i < outputs.length; i += 1)
        {
            auto output_index = outputs.pointer[i];
            print("%{u32} ", geti(output_index));
        }
        print_string(strlit("})\n"));
    }


    print("====\n", function->name);
}

fn void c_lower_append_string(VirtualBuffer(u8)* buffer, String string)
{
    memcpy(vb_add(buffer, string.length), string.pointer, string.length);
}

fn void c_lower_append_ch(VirtualBuffer(u8)* buffer, u8 ch)
{
    *vb_add(buffer, 1) = ch;
}

fn void c_lower_append_ch_repeated(VirtualBuffer(u8)* buffer, u8 ch, u32 times)
{
    u8* pointer = vb_add(buffer, times);
    memset(pointer, ch, times);
}

fn void c_lower_append_space(VirtualBuffer(u8)* buffer)
{
    c_lower_append_ch(buffer, ' ');
}

fn void c_lower_append_space_margin(VirtualBuffer(u8)* buffer, u32 times)
{
    c_lower_append_ch_repeated(buffer, ' ', times * 4);
}

fn void c_lower_type(VirtualBuffer(u8)* buffer, Thread* thread, TypeIndex type_index)
{
    Type* type = thread_type_get(thread, type_index);
    switch (type->id)
    {
        case TYPE_INTEGER:
            {
                u8 ch[] = { 'u', 's' };
                auto integer = &type->integer;
                u8 signedness_ch = ch[type->integer.is_signed];
                c_lower_append_ch(buffer, signedness_ch);
                u8 upper_digit = integer->bit_count / 10;
                u8 lower_digit = integer->bit_count % 10;
                if (upper_digit)
                {
                    c_lower_append_ch(buffer, upper_digit + '0');
                }
                c_lower_append_ch(buffer, lower_digit + '0');
            } break;
        default:
            trap();
    }
}

fn void c_lower_node(VirtualBuffer(u8)* buffer, Thread* thread, NodeIndex node_index)
{
    auto* node = thread_node_get(thread, node_index);
    auto* type = thread_type_get(thread, node->type);
    switch (node->id)
    {
        case NODE_CONSTANT:
            {
                switch (type->id)
                {
                    case TYPE_INTEGER:
                        {
                            assert(type->integer.bit_count == 0);
                            assert(type->integer.is_constant);
                            assert(!type->integer.is_signed);
                            vb_generic_ensure_capacity(buffer, 1, 64);
                            auto current_length = buffer->length;
                            auto buffer_slice = (String){ .pointer = buffer->pointer + current_length, .length = buffer->capacity - current_length, };
                            auto written_characters = format_hexadecimal(buffer_slice, type->integer.constant);
                            buffer->length = current_length + written_characters;
                        } break;
                        trap();
                    default:
                        trap();
                }
            } break;
        default:
            trap();
    }
}

fn String c_lower(Thread* thread)
{
    VirtualBuffer(u8) buffer = {};
    auto program_epilogue = strlit("#include <stdint.h>\n"
            "typedef uint8_t u8;\n"
            "typedef uint16_t u16;\n"
            "typedef uint32_t u32;\n"
            "typedef uint64_t u64;\n"
            "typedef int8_t s8;\n"
            "typedef int16_t s16;\n"
            "typedef int32_t s32;\n"
            "typedef int64_t s64;\n"
            );
    c_lower_append_string(&buffer, program_epilogue);

    for (u32 function_i = 0; function_i < thread->buffer.functions.length; function_i += 1)
    {
        auto* function = &thread->buffer.functions.pointer[function_i];
        c_lower_type(&buffer, thread, function->return_type);
        c_lower_append_space(&buffer);

        c_lower_append_string(&buffer, function->name);
        c_lower_append_ch(&buffer, argument_start);
        c_lower_append_ch(&buffer, argument_end);
        c_lower_append_ch(&buffer, '\n');
        c_lower_append_ch(&buffer, block_start);
        c_lower_append_ch(&buffer, '\n');

        auto start_node_index = function->start;
        auto* start_node = thread_node_get(thread, start_node_index);
        assert(start_node->output_count > 0);
        auto stop_node_index = function->stop;

        auto proj_node_index = node_output_get(thread, start_node, 1);
        auto it_node_index = proj_node_index;
        auto current_statement_margin = 1;

        while (!index_equal(it_node_index, stop_node_index))
        {
            auto* it_node = thread_node_get(thread, it_node_index);
            auto outputs = node_get_outputs(thread, it_node);
            auto inputs = node_get_inputs(thread, it_node);

            switch (it_node->id)
            {
                case NODE_CONTROL_PROJECTION:
                    break;
                case NODE_RETURN:
                    {
                        c_lower_append_space_margin(&buffer, current_statement_margin);
                        c_lower_append_string(&buffer, strlit("return "));
                        assert(inputs.length > 1);
                        assert(inputs.length == 2);
                        auto input = inputs.pointer[1];
                        c_lower_node(&buffer, thread, input);
                        c_lower_append_ch(&buffer, ';');
                        c_lower_append_ch(&buffer, '\n');
                    } break;
                case NODE_STOP:
                    break;
                default:
                    trap();
            }

            assert(outputs.length == 1);
            it_node_index = outputs.pointer[0];
        }

        c_lower_append_ch(&buffer, block_end);
    }

    return (String) { .pointer = buffer.pointer, .length = buffer.length };
}

fn void thread_init(Thread* thread)
{
    memset(thread, 0, sizeof(Thread));
    thread->arena = arena_init_default(KB(64));
    thread->main_function = -1;

    Type top, bot, live_control, dead_control;
    memset(&top, 0, sizeof(Type));
    top.id = TYPE_TOP;
    memset(&bot, 0, sizeof(Type));
    bot.id = TYPE_BOTTOM;
    memset(&live_control, 0, sizeof(Type));
    live_control.id = TYPE_LIVE_CONTROL;
    memset(&dead_control, 0, sizeof(Type));
    dead_control.id = TYPE_DEAD_CONTROL;

    thread->types.top = intern_pool_get_or_put_new_type(thread, &top).index;
    thread->types.bottom = intern_pool_get_or_put_new_type(thread, &bot).index;
    thread->types.live_control = intern_pool_get_or_put_new_type(thread, &live_control).index;
    thread->types.dead_control = intern_pool_get_or_put_new_type(thread, &dead_control).index;

    thread->types.integer.top = thread_get_integer_type(thread, (TypeInteger) {
        .constant = 0,
        .is_constant = 0,
        .is_signed = 0,
        .bit_count = 0,
    });
    thread->types.integer.bottom = thread_get_integer_type(thread, (TypeInteger) {
        .constant = 1,
        .is_constant = 0,
        .is_signed = 0,
        .bit_count = 0,
    });
    thread->types.integer.zero = thread_get_integer_type(thread, (TypeInteger) {
        .constant = 0,
        .is_constant = 1,
        .is_signed = 0,
        .bit_count = 0,
    });
}

fn void thread_clear(Thread* thread)
{
    arena_reset(thread->arena);
}

#define DO_UNIT_TESTS 1
#if DO_UNIT_TESTS
fn void unit_tests()
{
    for (u64 power = 1, log2_i = 0; log2_i < 64; power <<= 1, log2_i += 1)
    {
        assert(log2_alignment(power) == log2_i);
    }
}
#endif

Slice(String) arguments;

typedef enum ExecutionEngine : u8
{
    EXECUTION_ENGINE_C = 'c',
    EXECUTION_ENGINE_INTERPRETER = 'i',
} ExecutionEngine;

struct Interpreter
{
    Function* function;
};
typedef struct Interpreter Interpreter;

fn Interpreter* interpreter_create(Thread* thread)
{
    auto* interpreter = arena_allocate(thread->arena, Interpreter, 1);
    *interpreter = (Interpreter){};
    return interpreter;
}

fn s32 interpreter_run(Interpreter* interpreter, Thread* thread)
{
    Function* function = interpreter->function;
    auto start_node_index = function->start;
    auto* start_node = thread_node_get(thread, start_node_index);
    assert(start_node->output_count > 0);
    auto stop_node_index = function->stop;

    auto proj_node_index = node_output_get(thread, start_node, 1);
    auto it_node_index = proj_node_index;
    auto current_statement_margin = 1;

    s32 result = -1;

    while (!index_equal(it_node_index, stop_node_index))
    {
        auto* it_node = thread_node_get(thread, it_node_index);
        auto outputs = node_get_outputs(thread, it_node);
        auto inputs = node_get_inputs(thread, it_node);

        switch (it_node->id)
        {
            case NODE_CONTROL_PROJECTION:
                break;
            case NODE_RETURN:
                {
                    auto return_value = thread_node_get(thread, inputs.pointer[1]);
                    if (return_value->id == NODE_CONSTANT)
                    {
                        auto constant_type_index = return_value->constant.type;
                        auto* constant_type = thread_type_get(thread, constant_type_index);
                        switch (constant_type->id)
                        {
                            case TYPE_INTEGER:
                                {
                                    assert(constant_type->integer.is_constant);
                                    result = constant_type->integer.constant;
                                } break;
                            default:
                                trap();
                        }
                    }
                    else
                    {
                        trap();
                    }
                } break;
            case NODE_STOP:
                break;
            default:
                trap();
        }

        assert(outputs.length == 1);
        it_node_index = outputs.pointer[0];
    }

    return result;
}

#if LINK_LIBC
int main(int argc, const char* argv[], char* envp[])
{
#else
void entry_point(int argc, const char* argv[])
{
    char** envp = (char**)&argv[argc + 1];
#endif
#if DO_UNIT_TESTS
    unit_tests();
#endif

    if (argc < 3)
    {
        fail();
    }

    Arena* global_arena = arena_init_default(KB(64));
    {
        arguments.pointer = arena_allocate(global_arena, String, argc);
        arguments.length = argc;

        for (int i = 0; i < argc; i += 1)
        {
            u64 len = strlen(argv[i]);
            arguments.pointer[i] = (String) {
                .pointer = (u8*)argv[i],
                .length = len,
            };
        }
    }

    String source_file_path = arguments.pointer[1];
    ExecutionEngine execution_engine = arguments.pointer[2].pointer[0];

    Thread* thread = arena_allocate(global_arena, Thread, 1);
    thread_init(thread);

    syscall_mkdir("nest", 0755);

    File file = {
        .path = source_file_path,
        .source = file_read(thread->arena, source_file_path),
    };
    analyze_file(thread, &file);
    auto test_dir = string_no_extension(file.path);
    auto test_name = string_base(test_dir);

    for (u32 function_i = 0; function_i < thread->buffer.functions.length; function_i += 1)
    {
        Function* function = &thread->buffer.functions.pointer[function_i];
        NodeIndex start_node_index = function->start;
        NodeIndex stop_node_index = function->stop;
        iterate_peepholes(thread, function, stop_node_index);
        // print_string(strlit("Before optimizations\n"));
        // print_function(thread, function);
        gcm_build_cfg(thread, start_node_index, stop_node_index);
        // print_string(strlit("After optimizations\n"));
        // print_function(thread, function);
    }

    if (thread->main_function == -1)
    {
        fail();
    }

    auto lowered_source = c_lower(thread);
    // print("Transpiled to C:\n```\n{s}\n```\n", lowered_source);

    auto c_source_path = arena_join_string(thread->arena, (Slice(String)) array_to_slice(((String[]) {
                    strlit("nest/"),
                    test_name,
                    strlit(".c"),
                    })));

    file_write(c_source_path, lowered_source);

    auto exe_path_view = s_get_slice(u8, c_source_path, 0, c_source_path.length - 2);
    auto exe_path = (char*)arena_allocate_bytes(thread->arena, exe_path_view.length + 1, 1);
    memcpy((char*)exe_path, exe_path_view.pointer, exe_path_view.length);
    exe_path[exe_path_view.length] = 0;

    auto command = (char*[]) {
        "/usr/bin/cc", "-g",
        "-o", exe_path,
        (char*)c_source_path.pointer,
        0,
    };

    switch (execution_engine)
    {
    case EXECUTION_ENGINE_C:
        {
            int res = syscall_execve("/usr/bin/cc", command, envp);
            assert(0);
        } break;
    case EXECUTION_ENGINE_INTERPRETER:
        {
            auto* main_function = &thread->buffer.functions.pointer[thread->main_function];
            auto* interpreter = interpreter_create(thread);
            interpreter->function = main_function;
            auto exit_code = interpreter_run(interpreter, thread);
            print("Interpreter exited with exit code: {u32}\n", exit_code);
            syscall_exit(exit_code);
        } break;
    }

    thread_clear(thread);
#if LINK_LIBC == 0
    syscall_exit(0);
#endif
}

#if LINK_LIBC == 0
[[gnu::naked]] [[noreturn]] void _start()
{
    __asm__ __volatile__(
            "\nxor %ebp, %ebp"
            "\npopq %rdi"
            "\nmov %rsp, %rsi"
            "\nand $~0xf, %rsp"
            "\npushq %rsp"
            "\npushq $0"
            "\ncallq entry_point"
            "\nud2\n"
       );
}
#endif
