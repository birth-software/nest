#include <stdint.h>
#include <stdarg.h>

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
#define trap() __builtin_trap()
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
extern "C" void* memcpy(void* __restrict dst, void* __restrict src, u64 size)
{
    auto* destination = (u8*)dst;
    auto* source = (u8*)src;

    for (u64 i = 0; i < size; i += 1)
    {
        destination[i] = source[i];
    }

    return dst;
}

extern "C" void* memset(void* dst, u8 n, u64 size)
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
#endif
#define slice_from_pointer_range(T, start, end) (Slice(T)) { .pointer = start, .length = (u64)(end - start), }

#define strlit(s) (String){ .pointer = (u8*)s, .length = sizeof(s) - 1, }
#define ch_to_str(ch) (String){ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = (arr), .length = array_length(arr) }
#define pointer_to_bytes(p) Slice<u8>{ .pointer = (u8*)(p), .length = sizeof(*p) }
#define struct_to_bytes(s) pointer_to_bytes(&(s))

#define case_to_name(prefix, e) case prefix::e: return strlit(#e)

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

enum class SyscallX86_64 : u64 {
    read = 0,
    write = 1,
    open = 2,
    close = 3,
    stat = 4,
    fstat = 5,
    lstat = 6,
    poll = 7,
    lseek = 8,
    mmap = 9,
    mprotect = 10,
    munmap = 11,
    brk = 12,
    rt_sigaction = 13,
    rt_sigprocmask = 14,
    rt_sigreturn = 15,
    ioctl = 16,
    pread64 = 17,
    pwrite64 = 18,
    readv = 19,
    writev = 20,
    access = 21,
    pipe = 22,
    select = 23,
    sched_yield = 24,
    mremap = 25,
    msync = 26,
    mincore = 27,
    madvise = 28,
    shmget = 29,
    shmat = 30,
    shmctl = 31,
    dup = 32,
    dup2 = 33,
    pause = 34,
    nanosleep = 35,
    getitimer = 36,
    alarm = 37,
    setitimer = 38,
    getpid = 39,
    sendfile = 40,
    socket = 41,
    connect = 42,
    accept = 43,
    sendto = 44,
    recvfrom = 45,
    sendmsg = 46,
    recvmsg = 47,
    shutdown = 48,
    bind = 49,
    listen = 50,
    getsockname = 51,
    getpeername = 52,
    socketpair = 53,
    setsockopt = 54,
    getsockopt = 55,
    clone = 56,
    fork = 57,
    vfork = 58,
    execve = 59,
    exit = 60,
    wait4 = 61,
    kill = 62,
    uname = 63,
    semget = 64,
    semop = 65,
    semctl = 66,
    shmdt = 67,
    msgget = 68,
    msgsnd = 69,
    msgrcv = 70,
    msgctl = 71,
    fcntl = 72,
    flock = 73,
    fsync = 74,
    fdatasync = 75,
    truncate = 76,
    ftruncate = 77,
    getdents = 78,
    getcwd = 79,
    chdir = 80,
    fchdir = 81,
    rename = 82,
    mkdir = 83,
    rmdir = 84,
    creat = 85,
    link = 86,
    unlink = 87,
    symlink = 88,
    readlink = 89,
    chmod = 90,
    fchmod = 91,
    chown = 92,
    fchown = 93,
    lchown = 94,
    umask = 95,
    gettimeofday = 96,
    getrlimit = 97,
    getrusage = 98,
    sysinfo = 99,
    times = 100,
    ptrace = 101,
    getuid = 102,
    syslog = 103,
    getgid = 104,
    setuid = 105,
    setgid = 106,
    geteuid = 107,
    getegid = 108,
    setpgid = 109,
    getppid = 110,
    getpgrp = 111,
    setsid = 112,
    setreuid = 113,
    setregid = 114,
    getgroups = 115,
    setgroups = 116,
    setresuid = 117,
    getresuid = 118,
    setresgid = 119,
    getresgid = 120,
    getpgid = 121,
    setfsuid = 122,
    setfsgid = 123,
    getsid = 124,
    capget = 125,
    capset = 126,
    rt_sigpending = 127,
    rt_sigtimedwait = 128,
    rt_sigqueueinfo = 129,
    rt_sigsuspend = 130,
    sigaltstack = 131,
    utime = 132,
    mknod = 133,
    uselib = 134,
    personality = 135,
    ustat = 136,
    statfs = 137,
    fstatfs = 138,
    sysfs = 139,
    getpriority = 140,
    setpriority = 141,
    sched_setparam = 142,
    sched_getparam = 143,
    sched_setscheduler = 144,
    sched_getscheduler = 145,
    sched_get_priority_max = 146,
    sched_get_priority_min = 147,
    sched_rr_get_interval = 148,
    mlock = 149,
    munlock = 150,
    mlockall = 151,
    munlockall = 152,
    vhangup = 153,
    modify_ldt = 154,
    pivot_root = 155,
    _sysctl = 156,
    prctl = 157,
    arch_prctl = 158,
    adjtimex = 159,
    setrlimit = 160,
    chroot = 161,
    sync = 162,
    acct = 163,
    settimeofday = 164,
    mount = 165,
    umount2 = 166,
    swapon = 167,
    swapoff = 168,
    reboot = 169,
    sethostname = 170,
    setdomainname = 171,
    iopl = 172,
    ioperm = 173,
    create_module = 174,
    init_module = 175,
    delete_module = 176,
    get_kernel_syms = 177,
    query_module = 178,
    quotactl = 179,
    nfsservctl = 180,
    getpmsg = 181,
    putpmsg = 182,
    afs_syscall = 183,
    tuxcall = 184,
    security = 185,
    gettid = 186,
    readahead = 187,
    setxattr = 188,
    lsetxattr = 189,
    fsetxattr = 190,
    getxattr = 191,
    lgetxattr = 192,
    fgetxattr = 193,
    listxattr = 194,
    llistxattr = 195,
    flistxattr = 196,
    removexattr = 197,
    lremovexattr = 198,
    fremovexattr = 199,
    tkill = 200,
    time = 201,
    futex = 202,
    sched_setaffinity = 203,
    sched_getaffinity = 204,
    set_thread_area = 205,
    io_setup = 206,
    io_destroy = 207,
    io_getevents = 208,
    io_submit = 209,
    io_cancel = 210,
    get_thread_area = 211,
    lookup_dcookie = 212,
    epoll_create = 213,
    epoll_ctl_old = 214,
    epoll_wait_old = 215,
    remap_file_pages = 216,
    getdents64 = 217,
    set_tid_address = 218,
    restart_syscall = 219,
    semtimedop = 220,
    fadvise64 = 221,
    timer_create = 222,
    timer_settime = 223,
    timer_gettime = 224,
    timer_getoverrun = 225,
    timer_delete = 226,
    clock_settime = 227,
    clock_gettime = 228,
    clock_getres = 229,
    clock_nanosleep = 230,
    exit_group = 231,
    epoll_wait = 232,
    epoll_ctl = 233,
    tgkill = 234,
    utimes = 235,
    vserver = 236,
    mbind = 237,
    set_mempolicy = 238,
    get_mempolicy = 239,
    mq_open = 240,
    mq_unlink = 241,
    mq_timedsend = 242,
    mq_timedreceive = 243,
    mq_notify = 244,
    mq_getsetattr = 245,
    kexec_load = 246,
    waitid = 247,
    add_key = 248,
    request_key = 249,
    keyctl = 250,
    ioprio_set = 251,
    ioprio_get = 252,
    inotify_init = 253,
    inotify_add_watch = 254,
    inotify_rm_watch = 255,
    migrate_pages = 256,
    openat = 257,
    mkdirat = 258,
    mknodat = 259,
    fchownat = 260,
    futimesat = 261,
    fstatat64 = 262,
    unlinkat = 263,
    renameat = 264,
    linkat = 265,
    symlinkat = 266,
    readlinkat = 267,
    fchmodat = 268,
    faccessat = 269,
    pselect6 = 270,
    ppoll = 271,
    unshare = 272,
    set_robust_list = 273,
    get_robust_list = 274,
    splice = 275,
    tee = 276,
    sync_file_range = 277,
    vmsplice = 278,
    move_pages = 279,
    utimensat = 280,
    epoll_pwait = 281,
    signalfd = 282,
    timerfd_create = 283,
    eventfd = 284,
    fallocate = 285,
    timerfd_settime = 286,
    timerfd_gettime = 287,
    accept4 = 288,
    signalfd4 = 289,
    eventfd2 = 290,
    epoll_create1 = 291,
    dup3 = 292,
    pipe2 = 293,
    inotify_init1 = 294,
    preadv = 295,
    pwritev = 296,
    rt_tgsigqueueinfo = 297,
    perf_event_open = 298,
    recvmmsg = 299,
    fanotify_init = 300,
    fanotify_mark = 301,
    prlimit64 = 302,
    name_to_handle_at = 303,
    open_by_handle_at = 304,
    clock_adjtime = 305,
    syncfs = 306,
    sendmmsg = 307,
    setns = 308,
    getcpu = 309,
    process_vm_readv = 310,
    process_vm_writev = 311,
    kcmp = 312,
    finit_module = 313,
    sched_setattr = 314,
    sched_getattr = 315,
    renameat2 = 316,
    seccomp = 317,
    getrandom = 318,
    memfd_create = 319,
    kexec_file_load = 320,
    bpf = 321,
    execveat = 322,
    userfaultfd = 323,
    membarrier = 324,
    mlock2 = 325,
    copy_file_range = 326,
    preadv2 = 327,
    pwritev2 = 328,
    pkey_mprotect = 329,
    pkey_alloc = 330,
    pkey_free = 331,
    statx = 332,
    io_pgetevents = 333,
    rseq = 334,
    pidfd_send_signal = 424,
    io_uring_setup = 425,
    io_uring_enter = 426,
    io_uring_register = 427,
    open_tree = 428,
    move_mount = 429,
    fsopen = 430,
    fsconfig = 431,
    fsmount = 432,
    fspick = 433,
    pidfd_open = 434,
    clone3 = 435,
    close_range = 436,
    openat2 = 437,
    pidfd_getfd = 438,
    faccessat2 = 439,
    process_madvise = 440,
    epoll_pwait2 = 441,
    mount_setattr = 442,
    quotactl_fd = 443,
    landlock_create_ruleset = 444,
    landlock_add_rule = 445,
    landlock_restrict_self = 446,
    memfd_secret = 447,
    process_mrelease = 448,
    futex_waitv = 449,
    set_mempolicy_home_node = 450,
    cachestat = 451,
    fchmodat2 = 452,
    map_shadow_stack = 453,
    futex_wake = 454,
    futex_wait = 455,
    futex_requeue = 456,
};
#endif
#endif

fn void* syscall_mmap(void* address, size_t length, int protection_flags, int map_flags, int fd, signed long offset)
{
#if LINK_LIBC
    return mmap(address, length, protection_flags, map_flags, fd, offset);
#else 
#ifdef __linux__
    return (void*) syscall6(static_cast<long>(SyscallX86_64::mmap), (unsigned long)address, length, protection_flags, map_flags, fd, offset);
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
    return syscall3(static_cast<long>(SyscallX86_64::mprotect), (unsigned long)address, length, protection_flags);
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
    return syscall3(static_cast<long>(SyscallX86_64::open), (unsigned long)file_path, flags, mode);
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
    return syscall1(static_cast<long>(SyscallX86_64::close), fd);
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
    return syscall2(static_cast<long>(SyscallX86_64::fstat), fd, (unsigned long)buffer);
#else
    return fstat(fd, buffer);
#endif
#endif
}

fn u64 file_get_size(int fd)
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
    return syscall3(static_cast<long>(SyscallX86_64::read), fd, (unsigned long)buffer, bytes);
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
    return syscall3(static_cast<long>(SyscallX86_64::write), fd, (unsigned long)buffer, bytes);
#else
    return write(fd, buffer, bytes);
#endif
#endif
}

may_be_unused fn int syscall_rmdir(const char* path)
{
#if LINK_LIBC
    return rmdir(path);
#else
    return syscall1(static_cast<long>(SyscallX86_64::rmdir), (unsigned long)path);
#endif
}

may_be_unused fn int syscall_unlink(const char* path)
{
#if LINK_LIBC
    return unlink(path);
#else
    return syscall1(static_cast<long>(SyscallX86_64::unlink), (unsigned long)path);
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

[[noreturn]] [[gnu::cold]] fn void fail()
{
    trap();
    syscall_exit(1);
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

                                u64 value = original_value;
                                if (value)
                                {
                                    switch (format)
                                    {
                                        case INTEGER_FORMAT_HEXADECIMAL:
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

                                                while (reverse_index > 0)
                                                {
                                                    reverse_index -= 1;
                                                    s_get(buffer, buffer_i) = reverse_buffer[reverse_index];
                                                    buffer_i += 1;
                                                }
                                            } break;
                                        case INTEGER_FORMAT_DECIMAL:
                                            {
                                                // TODO: maybe print in one go?

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

                                                while (reverse_index > 0)
                                                {
                                                    reverse_index -= 1;
                                                    s_get(buffer, buffer_i) = reverse_buffer[reverse_index];
                                                    buffer_i += 1;
                                                }
                                            } break;
                                        case INTEGER_FORMAT_OCTAL:
                                        case INTEGER_FORMAT_BINARY:
                                            trap();
                                    }
                                }
                                else
                                {
                                    s_get(buffer, buffer_i) = '0';
                                    buffer_i += 1;
                                }
                            } break;
                        default:
                            trap();
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

#define arena_allocate(arena, T, count) arena_allocate_bytes(arena, sizeof(T) * count, alignof(T))
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
        u32 key = map->pointer[index];

        // Not set
        if (key == 0)
        {
            result = index;
            break;
        }
        else
        {
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
        .pointer = (u8*)arena_allocate_bytes(arena, file_size, 64),
        .length = file_size,
    };

    // TODO: big files
    ssize_t read_result = syscall_read(file_descriptor, result.pointer, result.length);
    assert(read_result >= 0);
    assert((u64)read_result == file_size);

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

fn u8* vb_generic_append_assume_capacity(VirtualBuffer(u8)* vb, void* item_pointer, u32 item_size, u32 item_count)
{
    u8* new_memory = vb_generic_add_assume_capacity(vb, item_size, item_count);
    memcpy(new_memory, item_pointer, item_size * item_count);
    return new_memory;
}

fn u8* vb_generic_add(VirtualBuffer(u8)* vb, u32 item_size, u32 item_count)
{
    vb_generic_ensure_capacity(vb, item_size, item_count);
    return vb_generic_add_assume_capacity(vb, item_size, item_count);
}


fn u8* vb_generic_append(VirtualBuffer(u8)* vb, void* item_pointer, u32 item_size, u32 item_count)
{
    vb_generic_ensure_capacity(vb, item_size, item_count);
    return vb_generic_append_assume_capacity(vb, item_pointer, item_size, item_count);
}

#define vb_add(a, count) (typeof((a)->pointer)) vb_generic_add((VirtualBuffer(u8)*)(a), sizeof(*((a)->pointer)), count)
#define vb_append_one(a, item) (typeof((a)->pointer)) vb_generic_append((VirtualBuffer(u8)*)(a), &(item), sizeof(*((a)->pointer)), 1)

typedef struct Thread Thread;

typedef enum TypeId : u32
{
    // Simple types
    TYPE_BOT = 0,
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
    u64 bit_count:7;
    u64 is_constant:1;
    u64 is_signed:1;
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

struct NodeConstant
{
    TypeIndex type;
};
typedef struct NodeConstant NodeConstant;

struct NodeStart
{
    TypeIndex arguments;
};
typedef struct NodeStart NodeStart;

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

struct NodeCFG
{
    s32 immediate_dominator_tree_depth;
    s32 loop_depth;
    s32 anti_dependency;
};
typedef struct NodeCFG NodeCFG;

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
        NodeScope scope;
        NodeControlProjection control_projection;
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
    NodeIndex start;
    NodeIndex stop;
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
};
typedef struct FunctionBuilder FunctionBuilder;

struct InternPool
{
    u32* pointer;
    u32 length;
    u32 capacity;
};
typedef struct InternPool InternPool;

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
};
typedef struct Thread Thread;

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

fn void node_unlock(Thread* thread, Node* node)
{
    if (node->hash)
    {
        trap();
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
    node_unlock(thread, node);
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
    Node* this_node = thread_node_get(thread, node_index);
    node_unlock(thread, this_node);
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

    for (u32 i = 0; i < data.inputs.length; i += 1)
    {
        NodeIndex input = data.inputs.pointer[i];
        if (validi(input))
        {
            node_add_output(thread, input, node_index);
        }
    }

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
    auto* node = thread_node_get(thread, node_index);
    node_unlock(thread, node);
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

fn NodeIndex node_keep(Thread* thread, NodeIndex node_index)
{
    return node_add_output(thread, node_index, invalidi(Node));
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
                            (a->integer.constant == b->integer.constant) &
                            (a->integer.bit_count == b->integer.bit_count) &
                            (a->integer.is_signed == b->integer.is_signed) &
                            (a->integer.is_constant == b->integer.is_constant);
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

typedef NodeIndex NodeIdealize(Thread* thread, NodeIndex node_index);
typedef TypeIndex NodeComputeType(Thread* thread, NodeIndex node_index);
typedef Hash TypeGetHash(Thread* thread, Type* type);
typedef Hash NodeGetHash(Thread* thread, Node* node);

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
    Hash hash = fnv_offset;

    for (auto* it = (u8*)type; it < (u8*)(type + 1); it += 1)
    {
        hash = hash_byte(hash, *it);
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

fn Hash node_get_hash_constant(Thread* thread, Node* node)
{
    trap();
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

struct TypeGetOrPut
{
    TypeIndex index;
    u8 existing;
};
typedef struct TypeGetOrPut TypeGetOrPut;

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

global const TypeVirtualTable type_functions[TYPE_COUNT] = {
    [TYPE_BOT] = { .get_hash = &type_get_hash_default },
    [TYPE_TOP] = { .get_hash = &type_get_hash_default },
    [TYPE_LIVE_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_DEAD_CONTROL] = { .get_hash = &type_get_hash_default },
    [TYPE_INTEGER] = { .get_hash = &type_get_hash_default },
    [TYPE_TUPLE] = { .get_hash = &type_get_hash_tuple },
};

global const NodeVirtualTable node_functions[NODE_COUNT] = {
    [NODE_STOP] = {
        .compute_type = &compute_type_bottom,
        .idealize = &idealize_stop,
    },
    [NODE_CONTROL_PROJECTION] = {
        .compute_type = &compute_type_projection,
        .idealize = &idealize_control_projection,
    },
    [NODE_DEAD_CONTROL] = {
        .compute_type = &compute_type_dead_control,
        .idealize = &idealize_null,
    },
    [NODE_RETURN] = {
        .compute_type = &compute_type_return,
        .idealize = &idealize_return,
    },
    [NODE_CONSTANT] = {
        .compute_type = &compute_type_constant,
        .idealize = &idealize_null,
        .get_hash = &node_get_hash_constant,
    },
};

fn Hash hash_type(Thread* thread, Type* type)
{
    Hash hash = type->hash;

    if (!hash)
    {
        hash = type_functions[type->id].get_hash(thread, type);
    }

    assert(hash);
    type->hash = hash;

    return hash;
}

fn NodeIndex intern_pool_put_node_at_assume_not_existent_assume_capacity(Thread* thread, NodeIndex node, u32 index)
{
    thread->interned.nodes.pointer[index] = geti(node);
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

        if ((key == 0) | (key == geti(node_index)))
        {
            result = index;
            break;
        }
        else
        {
            NodeIndex existing_node_index = *(NodeIndex*)&key;
            auto* existing_node = &thread->buffer.nodes.pointer[geti(existing_node_index)];
            if (existing_node->id == node->id)
            {
                trap();
            }
        }

        it_index += 1;
    }

    return result;
}

fn Hash hash_node(Node* node)
{
    auto hash = node->hash;
    if (!hash)
    {
        hash = fnv_offset;
        for (auto* it = (u8*)node; it < (u8*)(node + 1); it += 1)
        {
            hash = hash_byte(hash, *it);
        }

        node->hash = hash;
    }

    return hash;
}

fn NodeGetOrPut intern_pool_get_or_put_node(Thread* thread, NodeIndex node_index)
{
    auto existing_capacity = thread->interned.nodes.capacity;
    auto* node = &thread->buffer.nodes.pointer[geti(node_index)];
    auto hash = hash_node(node);
    auto original_index = hash & (existing_capacity - 1);
    
    auto slot = intern_pool_find_node_slot(thread, original_index, node_index);
    if (slot != -1)
    {
        u32 index = slot;
        u8 existing = thread->interned.nodes.pointer[index];
        auto result = intern_pool_put_node_at_assume_not_existent_assume_capacity(thread, node_index, index);
        return (NodeGetOrPut) {
            .index = result,
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
            trap();
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
                            result = a; 
                        }
                        else if (index_equal(right, integer_bot))
                        {
                            result = b; 
                        }
                        else if (index_equal(right, integer_top))
                        {
                            result = a; 
                        }
                        else if (index_equal(left, integer_top))
                        {
                            result = b; 
                        }
                        else
                        {
                            result = integer_bot;
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


fn void set_type(Thread* thread, Node* node, TypeIndex new_type)
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

fn u8 node_is_unused(Node* node)
{
    return node->output_count == 0;
}

fn u8 node_is_dead(Node* node)
{
    return node_is_unused(node) & ((node->input_count == 0) & (!validi(node->type)));
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

fn NodeIndex dead_code_elimination(Thread* thread, NodePair nodes)
{
    NodeIndex old = nodes.old;
    NodeIndex new = nodes.new;

    if (!index_equal(old, new))
    {
        auto* old_node = thread_node_get(thread, old);
        if (node_is_unused(old_node) & !node_is_dead(old_node))
        {
            trap();
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

global auto enable_peephole = 1;

fn NodeIndex peephole_optimize(Thread* thread, NodeIndex node_index)
{
    assert(enable_peephole);
    auto result = node_index;
    auto* node = thread_node_get(thread, node_index);
    auto old_type = node->type;
    auto new_type = node_functions[node->id].compute_type(thread, node_index);

    if (enable_peephole)
    {
        thread->iteration.total += 1;
        set_type(thread, node, new_type);

        if (node->id != NODE_CONSTANT && node->id != NODE_DEAD_CONTROL && type_is_high_or_const(thread, node->type))
        {
            trap();
        }

        auto idealize = 1;
        if (!node->hash)
        {
            auto gop = intern_pool_get_or_put_node(thread, node_index);
            if (gop.existing)
            {
                idealize = 0;
                trap();
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

fn NodeIndex peephole(Thread* thread, NodeIndex node_index)
{
    NodeIndex result;
    if (enable_peephole)
    {
        NodeIndex new_node = peephole_optimize(thread, node_index);
        if (validi(new_node))
        {
            NodeIndex peephole_new_node = peephole(thread, new_node);
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

fn TypeIndex thread_get_integer_type(Thread* thread, TypeInteger type_integer)
{
    Type type;
    memset(&type, 0, sizeof(Type));
    type.integer = type_integer;
    type.id = TYPE_INTEGER;

    auto result = intern_pool_get_or_put_new_type(thread, &type);
    return result.index;
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

fn NodeIndex constant_int_create(Thread* thread, Function* function, u64 value)
{
    auto node_index = thread_node_add(thread, (NodeCreate){
        .id = NODE_CONSTANT,
        .inputs = array_to_slice(((NodeIndex []) {
            function->start,
        }))
    });
    auto* node = thread_node_get(thread, node_index);

    auto type_index = thread_get_integer_type(thread, (TypeInteger){
        .constant = value,
        .bit_count = 0,
        .is_constant = 1,
        .is_signed = 0,
    });
    node->constant = (NodeConstant) {
        .type = type_index,
    };

    auto result = peephole(thread, node_index);
    return result;
}

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

fn NodeIndex analyze_expression(Thread* thread, Parser* parser, Function* function, String src, TypeIndex result_type)
{
    NodeIndex result = analyze_primary_expression(thread, parser, function, src);
    return result;
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

                String function_name = parse_identifier(parser, src);

                Function* function = vb_add(&thread->buffer.functions, 1);
                memset(function, 0, sizeof(Function));
                builder->function = function;

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
                dead_control = peephole(thread, dead_control);

                node_keep(thread, dead_control);

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
                    control_node_index = peephole(thread, control_node_index);
                    scope_define(thread, builder, control_name, thread->types.live_control, control_node_index);
                }

                TypeIndex return_type = analyze_type(thread, parser, src);

                skip_space(parser, src);
                expect_character(parser, src, block_start);

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
                            NodeIndex return_value = analyze_expression(thread, parser, function, src, return_type);
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
                                // TODO: make this happen
                                // trap();
                            }

                            return_node_index = peephole(thread, return_node_index);

                            builder_add_return(thread, builder, return_node_index);

                            builder_set_control(thread, builder, dead_control);
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

                parser->i += 1;

                scope_pop(thread, builder);
                function->stop = peephole(thread, function->stop);
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

fn void thread_init(Thread* thread)
{
    *thread = (Thread) {
        .arena = arena_init_default(KB(64)),
    };
    Type top, bot, live_control, dead_control;
    memset(&top, 0, sizeof(Type));
    top.id = TYPE_TOP;
    memset(&bot, 0, sizeof(Type));
    bot.id = TYPE_BOT;
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

//     Arena* arena = Arena::init_default(64*1024);
//
//     auto main_c_path = strlit("main.c");
//     {
//         auto main_c_content = strlit("int main()\n{\n    return 0;\n}");
//         int fd = syscall_open("main.c", O_WRONLY | O_CREAT | O_TRUNC, 0644);
//         assert(fd != -1);
//         auto result = syscall_write(fd, main_c_content.pointer, main_c_content.length);
//         assert(result >= 0);
//         assert((u64)result == main_c_content.length);
//         syscall_close(fd);
//     }
//     system("clang -c main.c -o main.o -Oz -fno-exceptions -fno-asynchronous-unwind-tables -fno-addrsig -fno-stack-protector -fno-ident");
//     system("objcopy --remove-section .note.GNU-stack main.o main2.o");
//     String main_o;
//     {
//
//         int fd = syscall_open("main2.o", 0, 0);
//         assert(fd != -1);
//         auto file_size = file_get_size(fd);
//
//         auto file = arena->allocate_slice<u8>(file_size);
//         auto read_bytes = syscall_read(fd, file.pointer, file.length);
//         assert(read_bytes == file_size);
//         main_o = file;
//         syscall_close(fd);
//         auto r1 = syscall_unlink("main.o");
//         assert(!r1);
//         auto r2 = syscall_unlink("main2.o");
//         assert(!r2);
//         auto r3 = syscall_unlink("main.c");
//         assert(!r3);
//     }
//
//     PinnedArray<u8> file = {};
//     auto elf_header = ELF::Header
//     {
//         .type = ELF::Type::relocatable,
//         .machine = ELF::Machine::x86_64,
//         .version = 1,
//         .entry_point = 0,
//         .program_header_offset = 0,
//         .section_header_offset = 192,
//         .flags = 0,
//         .program_header_size = 0,
//         .program_header_count = 0,
//         .section_header_size = sizeof(ELF::SectionHeader),
//         .section_header_count = 5,
//         .section_header_string_table_index = 4,
//     };
//     auto* original_elf_header = (ELF::Header*)&main_o[0];
//     file.append(struct_to_bytes(elf_header));
//     // .text
//     
//     // Code: 
//     // main:
//     //      xor eax, eax
//     //      ret
//     file.append_one(0x31);
//     file.append_one(0xc0);
//     file.append_one(0xc3);
//     for (int i = 0; i < 5; i += 1)
//     {
//         file.append_one(0);
//     }
//
//     // .symtab
//
//     ELF::Symbol null_symbol = {};
//     file.append(struct_to_bytes(null_symbol));
//
//     ELF::Symbol symbol1 = {
//         .name_offset = 1,
//         .type = ELF::Symbol::Type::FILE,
//         .binding = ELF::Symbol::Binding::LOCAL,
//         .section_index = (u16)ELF::SectionIndex::ABSOLUTE,
//         .value = 0,
//         .size = 0,
//     };
//     file.append(struct_to_bytes(symbol1));
//
//     ELF::Symbol symbol2 = {
//         .name_offset = 8,
//         .type = ELF::Symbol::Type::FUNCTION,
//         .binding = ELF::Symbol::Binding::GLOBAL,
//         .section_index = 1,
//         .value = 0,
//         .size = 3,
//     };
//     file.append(struct_to_bytes(symbol2));
//
//     // .strtab
//     // Null string
//     file.append_one(0);
//
//     file.append(strlit("main.c"));
//     file.append_one(0);
//
//     file.append(strlit("main"));
//     file.append_one(0);
//
//     // .shstrtab
//     // Null string
//     file.append_one(0);
//
//     file.append(strlit(".symtab"));
//     file.append_one(0);
//
//     file.append(strlit(".strtab"));
//     file.append_one(0);
//
//     file.append(strlit(".shstrtab"));
//     file.append_one(0);
//
//     file.append(strlit(".text"));
//     file.append_one(0);
//
//     // Align
//     file.append_one(0);
//     file.append_one(0);
//
//     assert(file.length == 0xc0);
//
//     // Section headers
//     ELF::SectionHeader null_section = {};
//     file.append(struct_to_bytes(null_section));
//
//     ELF::SectionHeader text_section = {
//         .name_offset = 27,
//         .type = ELF::SectionHeader::Type::program,
//         .flags = {
//             .alloc = 1,
//             .executable = 1,
//         },
//         .address = 0,
//         .offset = 64,
//         .size = 3,
//         .link = 0,
//         .info = 0,
//         .alignment = 4,
//         .entry_size = 0,
//     };
//     file.append(struct_to_bytes(text_section));
//
//     ELF::SectionHeader symtab_section = {
//         .name_offset = 1,
//         .type = ELF::SectionHeader::Type::symbol_table,
//         .flags = {},
//         .address = 0,
//         .offset = 72,
//         .size = 72,
//         .link = 3,
//         .info = 2,
//         .alignment = alignof(ELF::Symbol),
//         .entry_size = sizeof(ELF::Symbol),
//     };
//     file.append(struct_to_bytes(symtab_section));
//
//     ELF::SectionHeader strtab_section = {
//         .name_offset = 9,
//         .type = ELF::SectionHeader::Type::string_table,
//         .flags = {},
//         .address = 0,
//         .offset = 144,
//         .size = 13,
//         .link = 0,
//         .info = 0,
//         .alignment = 1,
//         .entry_size = 0,
//     };
//     file.append(struct_to_bytes(strtab_section));
//
//     ELF::SectionHeader shstrtab_section = {
//         .name_offset = 17,
//         .type = ELF::SectionHeader::Type::string_table,
//         .flags = {},
//         .address = 0,
//         .offset = 157,
//         .size = 33,
//         .link = 0,
//         .info = 0,
//         .alignment = 1,
//         .entry_size = 0,
//     };
//     file.append(struct_to_bytes(shstrtab_section));
//
//     auto mine = file.slice();
//     auto original = main_o.slice(0, file.length);
//     assert(mine.equal(original));
//     assert(file.length == main_o.length);
//
//     {
//         int fd = syscall_open("main.o", O_WRONLY | O_CREAT | O_TRUNC, 0644);
//         assert(fd != -1);
//         syscall_write(fd, file.pointer, file.length);
//
//         syscall_close(fd);
//     }
//
//     system("clang main.o -o main.exe");


fn void unit_tests()
{
    for (u64 power = 1, log2_i = 0; log2_i < 64; power <<= 1, log2_i += 1)
    {
        assert(log2_alignment(power) == log2_i);
    }
}

#if LINK_LIBC
int main()
#else
extern "C" void entry_point()
#endif
{
    {
        unit_tests();
    }
    Arena* global_arena = arena_init_default(KB(64));
    Thread* thread = arena_allocate(global_arena, Thread, 1);
    thread_init(thread);

    for (u32 i = 0; i < array_length(test_files); i += 1)
    {
        File file = {
            .path = test_files[i],
            .source = file_read(thread->arena, test_files[i]),
        };
        analyze_file(thread, &file);
        thread_clear(thread);
    }
}
