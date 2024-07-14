#include <stdint.h>
#include <stdarg.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
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

#define fn static
#define method __attribute__((visibility("internal")))
#define global static
#define assert(x) if (__builtin_expect(!(x), 0)) { trap(); }
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

#ifdef DEMAND_LIBC
#if DEMAND_LIBC
#define LINK_LIBC 1
#else 
#define LINK_LIBC 0
#endif
#else
#define LINK_LIBC 0
#endif

#if __APPLE__
    global auto constexpr page_size = KB(16);
#else
    global auto constexpr page_size = KB(4);
#endif

#define may_be_unused __attribute__((unused))

global constexpr auto brace_open = '{';
global constexpr auto brace_close = '}';

global constexpr auto parenthesis_open = '(';
global constexpr auto parenthesis_close = ')';

global constexpr auto bracket_open = '[';
global constexpr auto bracket_close = ']';

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

template<typename T>
forceinline fn u8 mem_equal_range(T* a, T* b, u64 count)
{
    return memcmp(a, b, count * sizeof(T)) == 0;
}

template<typename T>
struct Slice
{
    T* pointer;
    u64 length;

    method T& operator[](u64 index)
    {
        assert(index < length);
        return pointer[index];
    }

    fn Slice from_pointer_range(T* start, T* end)
    {
        assert(end >= start);
        return {
            .pointer = start,
            .length = u64(end - start),
        };
    }

    method Slice slice(u64 start, u64 end)
    {
        return {
            .pointer = pointer + start,
            .length = end - start,
        };
    }

    method forceinline u8 equal(Slice other)
    {
        if (length == other.length)
        {
            return mem_equal_range(pointer, other.pointer, length);
        }
        else
        {
            return 0;
        }
    }

    method forceinline T* begin()
    {
        return pointer;
    }

    method forceinline T* end()
    {
        return pointer + length;
    }

    method forceinline void copy_in(Slice other)
    {
        assert(length == other.length);
        memcpy(pointer, other.pointer, sizeof(T) * other.length);
    }

    method T* find(T item)
    {
        T* result = 0;

        for (T& i : *this)
        {
            if (i == item)
            {
                result = &i;
                break;
            }
        }

        return result;
    }

    method u32 index(T* item)
    {
        return item - pointer;
    }

    method s32 find_index(T item)
    {
        if (auto* result = find(item))
        {
            auto result_index = index(result);
            return result_index;
        }
        else
        {
            return -1;
        }
    }

    // Gotta implement this just because C++
    method u8 operator==(Slice other)
    {
        u8 result = 0;
        if (other.length == length)
        {
            if (other.pointer != pointer)
            {
                u64 i;
                for (i = 0; i < length; i += 1)
                {
                    if ((*this)[i] != other[i])
                    {
                        break;
                    }
                }

                result = i == length;
            }
            else
            {
                result = 1;
            }
        }

        return result;
    }
};

template <typename T>
forceinline fn T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
forceinline fn T max(T a, T b)
{
    return a > b ? a : b;
}


using String = Slice<u8>;
#define strlit(s) String{ .pointer = (u8*)s, .length = sizeof(s) - 1, }
#define ch_to_str(ch) String{ .pointer = &ch, .length = 1 }
#define array_to_slice(arr) { .pointer = arr, .length = array_length(arr) }

#define case_to_name(prefix, e) case prefix::e: return strlit(#e)

fn u64 parse_decimal(String string)
{
    u64 value = 0;
    for (u8 ch : string)
    {
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


template<typename T>
struct DynamicList
{
    T* pointer;
    u64 count;
    DynamicList* next;
};

template<typename T, u64 count>
struct StaticList
{
    u64 length;
    StaticList* next;
    T array[count];
};

global auto constexpr fnv_offset = 14695981039346656037ull;
global auto constexpr fnv_prime = 1099511628211ull;

fn Hash hash_bytes(String bytes)
{
    u64 result = fnv_offset;
    for (u64 i = 0; i < bytes.length; i += 1)
    {
        result ^= bytes.pointer[i];
        result *= fnv_prime;
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

global constexpr auto silent = 0;

may_be_unused fn void print(const char* format, ...)
{
    if constexpr (!silent)
    {
        u8 stack_buffer[4096];
        va_list args;
        va_start(args, format);
        String buffer = { .pointer = stack_buffer, .length = array_length(stack_buffer) };
        const char* it = format;
        u64 buffer_i = 0;

        while (*it)
        {
            while (*it && *it != brace_open)
            {
                buffer[buffer_i] = *it;
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

                                auto* bit_count_start = it;
                                while (is_decimal_digit(*it))
                                {
                                    it += 1;
                                }

                                auto* bit_count_end = it;
                                auto bit_count = parse_decimal(String::from_pointer_range((u8*)bit_count_start, (u8*)bit_count_end));

                                enum class IntegerFormat
                                {
                                    hexadecimal,
                                    decimal,
                                    octal,
                                    binary,
                                };

                                IntegerFormat format = IntegerFormat::decimal;

                                if (*it == ':')
                                {
                                    it += 1;
                                    switch (*it)
                                    {
                                        case 'x':
                                            format = IntegerFormat::hexadecimal;
                                            break;
                                        case 'd':
                                            format = IntegerFormat::decimal;
                                            break;
                                        case 'o':
                                            format = IntegerFormat::octal;
                                            break;
                                        case 'b':
                                            format = IntegerFormat::binary;
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
                                        case IntegerFormat::hexadecimal:
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
                                                    buffer[buffer_i] = reverse_buffer[reverse_index];
                                                    buffer_i += 1;
                                                }
                                            } break;
                                        case IntegerFormat::decimal:
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
                                                    buffer[buffer_i] = reverse_buffer[reverse_index];
                                                    buffer_i += 1;
                                                }
                                            } break;
                                        case IntegerFormat::octal:
                                        case IntegerFormat::binary:
                                            trap();
                                    }
                                }
                                else
                                {
                                    buffer[buffer_i] = '0';
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

        String final_string = buffer.slice(0, buffer_i);
        syscall_write(1, final_string.pointer, final_string.length);
    }
}

struct Arena
{
    u64 reserved_size;
    u64 committed;
    u64 commit_position;
    u64 granularity;
    u8 reserved[4 * 8] = {};

    global auto constexpr minimum_granularity = page_size;
    global auto constexpr middle_granularity = MB(2);
    global auto constexpr default_size = GB(4);

    fn Arena* init(u64 reserved_size, u64 granularity, u64 initial_size)
    {
        Arena* arena = (Arena*)reserve(reserved_size);
        commit(arena, initial_size);
        *arena = {
            .reserved_size = reserved_size,
            .committed = initial_size,
            .commit_position = sizeof(Arena),
            .granularity = granularity,
        };
        return arena;
    }

    method fn Arena* init_default(u64 initial_size)
    {
        return init(default_size, minimum_granularity, initial_size);
    }

    method void* allocate_bytes(u64 size, u64 alignment)
    {
        u64 aligned_offset = align_forward(commit_position, alignment);
        u64 aligned_size_after = aligned_offset + size;

        if (aligned_size_after > committed)
        {
            u64 committed_size = align_forward(aligned_size_after, granularity);
            u64 size_to_commit = committed_size - committed;
            void* commit_pointer = (u8*)this + committed;
            commit(commit_pointer, size_to_commit);
            committed = committed_size;
        }

        void* result = (u8*)this + aligned_offset;
        commit_position = aligned_size_after;
        assert(commit_position <= committed);
        return result;
    }

    template<typename T>
    method T* allocate_many(u64 count)
    {
        return (T*)allocate_bytes(sizeof(T) * count, alignof(T));
    }

    template<typename T>
    method T* allocate_one()
    {
        return allocate_many<T>(1);
    }

    template<typename T>
    method Slice<T> allocate_slice(u64 count)
    {
        return {
            .pointer = allocate_many<T>(count),
            .length = count,
        };
    }

    method void reset()
    {
        this->commit_position = sizeof(Arena);
        memset(this + 1, 0, this->committed - sizeof(Arena));
    }
};
static_assert(sizeof(Arena) == 64, "Arena must be cache aligned");

template<typename Destination, typename Source>
fn forceinline Destination transmute(Source source)
{
    static_assert(sizeof(Source) == sizeof(Destination));
    return *(Destination*)&source;
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

    result = {
        .pointer = (u8*)arena->allocate_bytes(file_size, 64),
        .length = file_size,
    };

    // TODO: big files
    ssize_t read_result = syscall_read(file_descriptor, result.pointer, result.length);
    assert(read_result >= 0);
    assert((u64)read_result == file_size);

    return result;
}

fn void print(String message)
{
    if constexpr (silent)
    {
        unused(message);
    }
    else
    {
        ssize_t result = syscall_write(1, message.pointer, message.length);
        assert(result >= 0);
        assert((u64)result == message.length);
    }
}

template<typename T> struct PinnedArray;
fn void generic_pinned_array_ensure_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T);
fn u8* generic_pinned_array_add_with_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T);

global constexpr auto granularity = page_size;
global constexpr auto reserved_size = ((u64)GB(4) - granularity);

template <typename T>
struct PinnedArray
{
    T* pointer;
    u32 length;
    u32 capacity;

    // static_assert(sizeof(T) % granularity == 0);

    method forceinline T& operator[](u32 index)
    {
        assert(index < length);
        return pointer[index];
    }

    method forceinline void ensure_capacity(u32 additional)
    {
        auto generic_array = (PinnedArray<u8>*)(this);
        generic_pinned_array_ensure_capacity(generic_array, additional, sizeof(T));
    }

    method forceinline void clear()
    {
        length = 0;
    }

    method forceinline Slice<T> add_with_capacity(u32 additional)
    {
        auto generic_array = (PinnedArray<u8>*)(this);
        auto pointer = generic_pinned_array_add_with_capacity(generic_array, additional, sizeof(T));
        return {
            .pointer = (T*)pointer,
            .length = additional,
        };
    }

    method forceinline Slice<T> add(u32 additional)
    {
        ensure_capacity(additional);
        auto slice = add_with_capacity(additional);
        return slice;
    }

    method forceinline Slice<T> append(Slice<T> items)
    {
        assert(items.length <= 0xffffffff);
        auto slice = add(items.length);
        slice.copy_in(items);
        return slice;
    }

    method forceinline T* add_one()
    {
        return add(1).pointer;
    }

    method forceinline T* append_one(T item)
    {
        T* new_item = add_one();
        *new_item = item;
        return new_item;
    }

    method forceinline T pop()
    {
        assert(length);
        length -= 1;
        return pointer[length];
    }

    method forceinline Slice<T> slice()
    {
        return {
            .pointer = pointer,
            .length = length,
        };
    }

    method T remove_swap(u32 index)
    {
        if (index >= 0 & index < length)
        {
            auto original_len = length;
            T item = pointer[index];
            T last = pointer[length - 1];
            pointer[index] = last;
            pop();
            assert(length == original_len - 1);
            return item;
        }

        trap();
    }
};

forceinline fn u32 generic_pinned_array_length(PinnedArray<u8>* array, u32 size_of_T)
{
    u32 current_length_bytes = array->length * size_of_T;
    return current_length_bytes;
}

fn void generic_pinned_array_ensure_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T)
{
    u32 wanted_capacity = array->length + additional_T;
    if (array->capacity < array->length + additional_T)
    {
        if (array->capacity == 0)
        {
            assert(array->length == 0);
            assert(array->pointer == 0);
            array->pointer = static_cast<u8*>(reserve(reserved_size));
        }

        u64 currently_committed_size = align_forward(array->capacity * size_of_T, granularity);
        u64 wanted_committed_size = align_forward(wanted_capacity * size_of_T, granularity);
        void* commit_pointer = array->pointer + currently_committed_size;
        u64 commit_size = wanted_committed_size - currently_committed_size;
        assert(commit_size > 0);
        commit(commit_pointer, commit_size);
        array->capacity = wanted_committed_size / size_of_T;
    }
}

fn u8* generic_pinned_array_add_with_capacity(PinnedArray<u8>* array, u32 additional_T, u32 size_of_T)
{
    u32 current_length_bytes = generic_pinned_array_length(array, size_of_T);
    assert(current_length_bytes < reserved_size);
    u8* pointer = array->pointer + current_length_bytes;
    array->length += additional_T;
    return pointer;
}

template <typename K, typename V>
struct Put
{
    K* key;
    V* value;
};

template <typename K, typename V>
struct GetOrPut
{
    Put<K, V> result;
    u8 existing;
};
template <typename K, typename V>
struct PutResult
{
    K* key;
    V* value;
};

global constexpr auto map_initial_capacity = 16;

template <typename V>
struct StringMap
{
    struct Pair
    {
        String key;
        V value;
    };

    Pair* pairs;
    u32 length;
    u32 capacity;

    fn StringMap init(Arena* arena, u32 capacity)
    {
        auto* pairs = arena->allocate_many<Pair>(capacity);
        return {
            .pairs = pairs,
            .length = 0,
            .capacity = capacity,
        };
    }

    StringMap duplicate(Arena* arena)
    {
        auto new_map = init(arena, capacity);
        new_map.length = length;
        memcpy(new_map.pairs, pairs, sizeof(Pair) * new_map.capacity);

        return new_map;
    }

    struct GetOrPut
    {
        Pair* pair;
        u8 existing;
    };

    method Pair* get_pair(String key)
    {
        Pair* result = 0;
        if (length)
        {
            assert(capacity);
            u32 index = find_index(key);
            auto* pair = &pairs[index];
            if (pair->key.length)
            {
                result = pair;
            }
        }

        return result;
    }

    method Hash find_index(String key)
    {
        auto hash = hash_bytes(key);
        auto index = hash & (capacity - 1);
        return index;
    }

    method V* get(String key)
    {
        V* result = 0;
        if (auto* pair = get_pair(key))
        {
            result = &pair->value;
        }

        return result;
    }

    method void ensure_capacity(Arena* arena, u32 additional)
    {
        if (length + additional > capacity)
        {
            auto new_capacity = max<u32>(capacity + additional, map_initial_capacity);
            auto* new_pairs = arena->allocate_many<Pair>(new_capacity);

            if (length)
            {
                memcpy(new_pairs, pairs, capacity * sizeof(Pair));
            }

            pairs = new_pairs;
            capacity = new_capacity;
        }
    }

    method GetOrPut get_or_put(Arena* arena, String key, V value)
    {
        if (capacity == 0)
        {
            ensure_capacity(arena, map_initial_capacity);
        }

        auto index = find_index(key);
        auto* candidate_pair = &pairs[index];

        if (candidate_pair->key.length)
        {
            for (u32 i = 0; i < capacity; i += 1)
            {
                auto wraparound_index = (index + i) & (capacity - 1);
                candidate_pair = &pairs[wraparound_index];

                if (candidate_pair->key.length == 0)
                {
                    *candidate_pair = {
                        .key = key,
                        .value = value,
                    };
                    return {
                        .pair = candidate_pair,
                        .existing = 0,
                    };
                }
                else if (candidate_pair->key.equal(key))
                {
                    return {
                        .pair = candidate_pair,
                        .existing = 1,
                    };
                }
            }

            trap();
        }
        else
        {
            ensure_capacity(arena, 1);
            candidate_pair->key = key;
            candidate_pair->value = value;
            length += 1;
            return {
                .pair = candidate_pair,
                .existing = 0,
            };
        }
    }

    method Pair* begin()
    {
        return pairs;
    }

    method Pair* end()
    {
        return pairs + capacity;
    }

    method void clear()
    {
        *this = {};
    }
};

template <typename T>
struct InternPool
{
    T** pointer;
    u32 length;
    u32 capacity;

    struct InternGetOrPut
    {
        T* result;
        u8 existing;
    };

    Hash get_index(Hash hash)
    {
        assert(capacity % 2 == 0);
        auto index = hash & (capacity - 1);
        assert(index <= 0xffffffff);
        static_assert(sizeof(index) == 8);
        return index;
    }

    method void ensure_capacity(Arena* arena, u32 additional)
    {
        u64 wanted_capacity = max<u64>(round_up_to_next_power_of_2(length + additional), 32);

        if (capacity < wanted_capacity)
        {
            T** new_array = arena->allocate_many<T*>(wanted_capacity);
            memset(new_array, 0, sizeof(T*) * wanted_capacity);

            auto* old_pointer = pointer;
            auto old_capacity = capacity;

            length = 0;
            pointer = new_array;
            capacity = wanted_capacity;

            // print("==========\nPUTTING\n==================\n");
            for (u32 i = 0; i < old_capacity; i += 1)
            {
                auto* key = old_pointer[i];
                if (key)
                {
                    put_assume_capacity_assume_not_existent(key);
                }
            }
            // print("==========\nEND PUTTING\n==================\n");

            for (u32 i = 0; i < old_capacity; i += 1)
            {
                auto* key = old_pointer[i];
                if (key)
                {
                    // print("Getting {u64:x} (hash: {u64:x})\n", key, key->hash);
                    auto* result = get(key);
                    assert(result);
                    assert(result == key);
                }
            }
        }
    }

    method T* get(T* key)
    {
        auto hash = key->hash;
        assert(hash);
        assert(hash == key->get_hash());

        auto original_index = get_index(hash);
        auto it_index = original_index;

        for (u32 i = 0; i < capacity; i += 1)
        {
            auto index = it_index & (capacity - 1);
            auto* element = &pointer[index];
            auto* map_key = *element;

            if (!map_key)
            {
                // TODO:
                // Linear probing makes holes into the hash table, so we gotta skip null spots,
                // otherwise the removal operation will fail
                // fail();
            }
            else if (map_key->equal(key))
            {
                return map_key;
            }
            else
            {
                // TODO:
                // assert(map_key->hash);
                // assert(hash != map_key->hash);
            }

            it_index += 1;
        }

        return 0;
    }

    method Hash put_assume_not_existent(Arena* arena, T* key)
    {
        ensure_capacity(arena, 1);
        return put_assume_capacity_assume_not_existent(key);
    }

    method Hash get_suitable_index(T* key, Hash original_index)
    {
        auto it_index = original_index;

        for (u32 i = 0; i < capacity; i += 1)
        {
            auto index = it_index & (capacity - 1);
            auto* element = &pointer[index];
            auto* map_key = *element;

            if (!map_key)
            {
                return index;
            }
            else if (map_key->equal(key))
            {
                trap();
            }
            else
            {
                // TODO:
                // assert(map_key->hash);
                // assert(hash != map_key->hash);
            }

            it_index += 1;
        }

        trap();
    }

    method Hash put_assume_capacity_assume_not_existent(T* key)
    {
        assert(key->hash);
        auto original_index = get_index(key->hash);
        auto suitable_index = get_suitable_index(key, original_index);
        // print("Putting {u64:x} (hash: {u64:x}) at index {u64}\n", key, key->hash, suitable_index);
        put_at_assume_capacity_not_existent(key, suitable_index);
        return suitable_index;
    }

    method void put_at_assume_capacity_not_existent(T* key, u32 index)
    {
        assert(length < capacity);
        pointer[index] = key;
        length += 1;
    }

    method T* remove(T* value_to_remove)
    {
        auto hash = value_to_remove->hash;
        assert(hash);
        assert(hash == value_to_remove->get_hash());

        auto original_index = get_index(hash);
        auto it_index = original_index;

        // print("Trying to remove {u64:x} (hash: {u64:x}, original index: {u64:x}, capacity: {u32}\n", value_to_remove, hash, original_index, capacity);

        for (u32 i = 0; i < capacity; i += 1)
        {
            auto index = it_index & (capacity - 1);
            auto* element = &pointer[index];
            auto* map_key = *element;

            if (!map_key)
            {
                // TODO:
                // Linear probing makes holes into the hash table, so we gotta skip null spots,
                // otherwise the removal operation will fail
                // fail();
            }
            else if (value_to_remove->equal(map_key))
            {
                *element = {};
                length -= 1;
                return map_key;
            }
            else
            {
                // TODO:
                // assert(map_key->hash);
                // assert(hash != map_key->hash);
            }

            it_index += 1;
        }

        trap();
    }

    method InternGetOrPut get_or_put(Arena* arena, T* new_value)
    {
        auto hash = new_value->get_hash();
        auto original_index = get_index(hash);
        auto it_index = original_index;

        s32 first_free_spot = -1;

        for (u32 i = 0; i < capacity; i += 1)
        {
            auto index = it_index & (capacity - 1);
            assert(index <= 0xffffffff);
            static_assert(sizeof(index) == 8);

            auto* element = &pointer[index];
            auto* map_key = *element;
            // if (hash == 0x800040e1)
            // {
                // print("Comparing {u64:x} with {u64:x} ({u64:x})\n", new_value, map_key, map_key ? map_key->hash : (u64)0);
            // }

            // TODO: This is a mess, undo
            if (!map_key)
            {
                if (first_free_spot == -1)
                {
                    first_free_spot = index;
                }
            }
            else if (new_value->equal(map_key))
            {
                assert(hash == map_key->hash);
                // print("Getting {u64:x} with hash {u64:x} at index {u64:x} (from {u64:x} with hash {u64:x})\n", map_key, map_key->hash, index, new_value, hash);
                return {
                    .result = map_key,
                    .existing = 1,
                };
            }
            else
            {
                // TODO:
                // assert(map_key->hash);
                // assert(hash != map_key->hash);
            }

            it_index += 1;
        }

        Hash index;
        if (length < capacity)
        {
            index = first_free_spot;
            index &= capacity - 1;
            put_at_assume_capacity_not_existent(new_value, index);
        }
        else if (length == capacity)
        {
            index = put_assume_not_existent(arena, new_value);
        }
        else
        {
            trap();
        }

        // print("Adding {u64:x} with hash {u64:x} at index {u64}\n", new_value, hash, index);

        return {
            .result = new_value,
            .existing = 0,
        };
    }

    method void clear()
    {
        *this = {};
    }
};

template <typename T>
using Array = PinnedArray<T>;

typedef enum FileStatus
{
    FILE_STATUS_ADDED = 0,
    FILE_STATUS_QUEUED = 1,
    FILE_STATUS_READ = 2,
    FILE_STATUS_ANALYZING = 3,
} FileStatus;

enum class DebugTypeId: u8
{
    VOID,
    NORETURN,
    POINTER,
    INTEGER,
    ARRAY,
    STRUCT,
    UNION,
    COUNT,
};
global auto constexpr type_id_bit_count = 3;
static_assert(static_cast<u8>(DebugTypeId::COUNT) < (1 << type_id_bit_count), "Type bit count for id must be respected");

global auto constexpr type_flags_bit_count = 32 - (type_id_bit_count + 1);

struct Thread;
struct NodeType
{
    enum class Id: u8
    {
        BOTTOM = 1,
        TOP,
        LIVE_CONTROL,
        DEAD_CONTROL,
        INTEGER,
        MULTIVALUE,
        // TODO: this is mine. Check if it is correct:
        FUNCTION,
        CALL,
    };

    union
    {
        struct
        {
            u64 constant;
            u8 is_constant;
        } constant;
        struct
        {
            Slice<NodeType*> types;
        } multi;
    } payload = {};
    Hash hash = 0;
    Id id;


    method u8 is_simple()
    {
        switch (id)
        {
            case Id::BOTTOM:
            case Id::TOP:
            case Id::LIVE_CONTROL:
            case Id::DEAD_CONTROL:
                return 1;
            default:
                return 0;
        }
    }

    method u8 equal(NodeType* other)
    {
        if (this == other)
        {
            return 1;
        }

        if (id != other->id)
        {
            return 0;
        }

        switch (id)
        {
        case Id::BOTTOM:
        case Id::TOP:
        case Id::LIVE_CONTROL:
        case Id::DEAD_CONTROL:
        case Id::FUNCTION:
        case Id::CALL:
            return 1;
        case Id::INTEGER:
            return (payload.constant.is_constant == other->payload.constant.is_constant) &
                (payload.constant.constant == other->payload.constant.constant);
        case Id::MULTIVALUE:
            if (payload.multi.types.length == other->payload.multi.types.length)
            {
                for (u32 i = 0; i < payload.multi.types.length; i += 1)
                {
                    if (payload.multi.types[i] != other->payload.multi.types[i])
                    {
                        return 0;
                    }
                }

                return 1;
            }
            else
            {
                return 0;
            }
        }
    }

    method u8 is_constant()
    {
        switch (id)
        {
            case Id::INTEGER:
                return payload.constant.is_constant;
            default:
                return 0;
        }
    }

    method NodeType* meet(Thread* thread, NodeType* other)
    {
        if (this == other)
        {
            return this;
        }

        if (id == other->id)
        {
            return x_meet(thread, other);
        }
        
        if (is_simple())
        {
            return x_meet(thread, other);
        }

        if (other->is_simple())
        {
            return other->x_meet(thread, this);
        }

        return x_meet(thread, other);
    }

    method NodeType* x_meet(Thread* thread, NodeType* other);

    method u8 is_bot()
    {
        assert(id == Id::INTEGER);
        return !payload.constant.is_constant & (payload.constant.constant == 1);
    }

    method u8 is_top()
    {
        assert(id == Id::INTEGER);
        return !payload.constant.is_constant & (payload.constant.constant == 0);
    }

    method u8 is_a(Thread* thread, NodeType* other)
    {
        return this->meet(thread, other) == other;
    }

    method NodeType* dual()
    {
        switch (id)
        {
            case Id::TOP:
            case Id::BOTTOM:
            case Id::DEAD_CONTROL:
            case Id::LIVE_CONTROL:
                trap();
                // return { .id = id };
            default:
                trap();
        }
    }

    method NodeType* join(Thread* thread, NodeType* other)
    {
        if (this == other)
        {
            return this;
        }

        return dual()->meet(thread, other->dual())->dual();
    }
    method NodeType* intern_type(Thread* thread);
    method u8 is_high_or_const()
    {
        switch (id)
        {
        case Id::TOP:
        case Id::DEAD_CONTROL:
            return 1;
        case Id::LIVE_CONTROL:
        case Id::BOTTOM:
        case Id::MULTIVALUE:
            return 0;
        case Id::FUNCTION:
        case Id::CALL:
            return 0;
        case Id::INTEGER:
            return payload.constant.is_constant | (payload.constant.constant == 0);
        }
    }

    method Hash get_hash()
    {
        if (!hash)
        {
            switch (id)
            {
                case NodeType::Id::MULTIVALUE:
                    {
                        Hash sum = 0;
                        // print("{u64:x} Field count: {u64}\n", this, payload.multi.types.length);
                        for (auto* type : payload.multi.types)
                        {
                            auto field_hash = type->get_hash();
                            // print("Field of {u64:x} hash {u64:x}: {u32:x}\n", this, type, field_hash);
                            sum ^= field_hash;
                        }

                        hash = sum;
                    }
                    break;
                case NodeType::Id::INTEGER:
                    hash = payload.constant.constant ^ (payload.constant.is_constant ? 0 : 0x4000);
                    break;
                default:
                    hash = (Hash)id;
                    break;
            }

            if (!hash)
            {
                hash = 0xDEADBEEF;
            }
        }

        return hash;
    }
};

struct DebugType
{
    u64 size;
    u64 alignment;
    DebugTypeId id : type_id_bit_count;
    u32 resolved: 1;
    u32 flags: type_flags_bit_count;
    u32 reserved = 0;
    String name;

    method u8 get_bit_count()
    {
        assert(id == DebugTypeId::INTEGER);
        u32 bit_count_mask = (1 << (type_flags_bit_count - 1)) - 1;
        u8 bit_count = flags & bit_count_mask;
        assert(bit_count <= size * 8);
        assert(bit_count <= 64);
        return bit_count;
    }

    method NodeType* lower(Thread* thread);
};
static_assert(sizeof(DebugType) == sizeof(u64) * 5, "Type must be 24 bytes");

struct Symbol
{
    enum class Id: u8
    {
        variable,
        function,
    };

    enum class Linkage: u8
    {
        internal,
        external,
    };

    String name;
    Id id: 1;
    Linkage linkage: 1;
};

typedef enum AbiInfoKind : u8
{
    ABI_INFO_IGNORE,
    ABI_INFO_DIRECT,
    ABI_INFO_DIRECT_PAIR,
    ABI_INFO_DIRECT_COERCE,
    ABI_INFO_DIRECT_COERCE_INT,
    ABI_INFO_DIRECT_SPLIT_STRUCT_I32,
    ABI_INFO_EXPAND_COERCE,
    ABI_INFO_INDIRECT,
    ABI_INFO_EXPAND,
} AbiInfoKind;

global auto constexpr void_type_index = 0;
global auto constexpr noreturn_type_index = 1;
global auto constexpr opaque_pointer_type_index = 2;
// global auto constexpr f32_type_offset = 3;
// global auto constexpr f64_type_offset = 4;
global auto constexpr integer_type_offset = 5;
global auto constexpr integer_type_count = 64 * 2;
global auto constexpr builtin_type_count = integer_type_count + integer_type_offset + 1;

struct Function;
struct Node;

struct Bitset
{
    PinnedArray<u64> arr;

    method u8 get(u64 index)
    {
        auto element_index = index / (sizeof(u64) * 8);
        if (element_index < arr.length)
        {
            auto bit_index = index % (sizeof(u64) * 8);
            auto result = (arr[element_index] & (1 << bit_index)) != 0;
            return result;
        }

        return 0;
    }

    method void ensure_length(u64 max)
    {
        auto length = (max / (sizeof(u64) * 8)) + (max % (sizeof(u64) * 8) != 0);
        auto old_length = arr.length;
        if (old_length < length)
        {
            auto new_element_count = length - old_length;
            unused(arr.add(new_element_count));
            for (u64 byte : arr.slice().slice(old_length, length))
            {
                assert(!byte);
            }
        }
    }

    method void set_assert_unset(u64 index)
    {
        ensure_length(index + 1);
        auto element_index = index / (sizeof(u64) * 8);
        auto bit_index = index % (sizeof(u64) * 8);
        assert((arr[element_index] & (1 << bit_index)) == 0);
        arr[element_index] |= (1 << bit_index);
    }

    method u8 is_empty()
    {
        return arr.length == 0;
    }

    method void clear(u64 index)
    {
        auto bit_index = index % (sizeof(u64) * 8);
        auto element_index = index / (sizeof(u64) * 8);
        auto mask = ~(1 << bit_index);
        static_assert(sizeof(bit_index) == sizeof(arr.pointer[0]));
        arr[element_index] &= mask;
    }

    method void clear()
    {
        memset(arr.pointer, 0, arr.capacity * sizeof(u64));
        arr.clear();
    }
};

struct WorkList
{
    PinnedArray<Node*> nodes = {};
    Bitset bitset = {};

    method void add_many(Slice<Node*> nodes)
    {
        for (auto* node : nodes)
        {
            push(node);
        }
    }

    method Node* push(Node* node);

    method Node* pop()
    {
        Node* result = 0;

        if (nodes.length)
        {
            u64 rng = generate_random_number();
            u32 index = rng & (nodes.capacity - 1);
            result = nodes[index];
            nodes[index] = nodes[nodes.length - 1];
            nodes.length -= 1;
            bitset.clear(index);
        }

        return result;
    }

    method u8 on(Node* node);

    method void clear()
    {
        nodes.clear();
        bitset.clear();
    }
};

struct Thread
{
    Arena* arena;
    PinnedArray<Function> functions = {};
    PinnedArray<NodeType> types = {};
    PinnedArray<Node> nodes = {};
    InternPool<NodeType> interned_types = {};
    InternPool<Node> interned_nodes = {};
    WorkList worklist = {};
    Bitset visited = {};

    struct
    {
        NodeType* bottom;
        NodeType* top;
        NodeType* live_control;
        NodeType* dead_control;
        NodeType* integer_bot;
        NodeType* integer_top;
        NodeType* integer_zero;
        NodeType* if_both;
        NodeType* if_neither;
        NodeType* if_true;
        NodeType* if_false;
    } common_types;
    u64 peephole_iteration_count = 0;
    u64 peephole_nop_iteration_count = 0;
    u32 node_count = 0;
    u8 mid_assert = 0;


    method void init();
    method void clear();

    method u8 progress_on_list(Function* function, Node* node);
};

method NodeType* NodeType::x_meet(Thread* thread, NodeType* other)
{
    switch (id)
    {
        case Id::BOTTOM:
        case Id::TOP:
        case Id::LIVE_CONTROL:
        case Id::DEAD_CONTROL:
            {
                assert(is_simple());
                if (this == thread->common_types.bottom || other == thread->common_types.top)
                {
                    return this;
                }

                if ((this == thread->common_types.top) | (other == thread->common_types.bottom))
                {
                    return other;
                }

                if (!other->is_simple())
                {
                    return thread->common_types.bottom;
                }

                if (this == thread->common_types.live_control || other == thread->common_types.live_control)
                {
                    return thread->common_types.live_control;
                }
                else
                {
                    return thread->common_types.dead_control;
                }
            }
        case Id::INTEGER:
            if (this == thread->common_types.integer_bot)
            {
                return this;
            }

            if (other == thread->common_types.integer_bot)
            {
                return other;
            }

            if (other == thread->common_types.integer_top)
            {
                return this;
            }

            if (this == thread->common_types.integer_top)
            {
                return other;
            }

            return thread->common_types.integer_bot;
        case Id::MULTIVALUE:
            fail();
        default:
            trap();
    }
}

method NodeType* DebugType::lower(Thread* thread)
{
    switch (id)
    {
        case DebugTypeId::VOID:
            trap();
        case DebugTypeId::NORETURN:
            trap();
        case DebugTypeId::POINTER:
            trap();
        case DebugTypeId::INTEGER:
            return thread->common_types.integer_bot;
        case DebugTypeId::ARRAY:
            trap();
        case DebugTypeId::STRUCT:
            trap();
        case DebugTypeId::UNION:
            trap();
        case DebugTypeId::COUNT:
            trap();
    }
}


method NodeType* NodeType::intern_type(Thread* thread)
{
    auto result = thread->interned_types.get_or_put(thread->arena, this);
    return result.result;
}

struct Unit
{
    // PinnedArray<File> files;
    // PinnedArray<Function> functions;
    // Arena* arena;
    // Arena* node_arena;
    // Arena* type_arena;
    DebugType* builtin_types;
    u64 generate_debug_information : 1;

    method DebugType* get_integer_type(u8 bit_count, u8 signedness)
    {
        auto index = integer_type_offset + signedness * 64 + bit_count - 1;
        return &builtin_types[index];
    }
};

union AbiInfoPayload
{
    NodeType* direct;
    NodeType* direct_pair[2];
    NodeType* direct_coerce;
    struct
    {
        NodeType* type;
        u32 alignment;
    } indirect;
};
typedef union AbiInfoPayload AbiInfoPayload;

struct AbiInfoAttributes
{
    u8 by_reg: 1;
    u8 zero_extend: 1;
    u8 sign_extend: 1;
    u8 realign: 1;
    u8 by_value: 1;
};
typedef struct AbiInfoAttributes AbiInfoAttributes;

struct AbiInfo
{
    AbiInfoPayload payload;
    u16 indices[2] = {};
    AbiInfoAttributes attributes = {};
    AbiInfoKind kind;
};

struct Function
{
    struct Prototype
    {
        AbiInfo* argument_type_abis; // The count for this array is "original_argument_count", not "abi_argument_count"
        DebugType** original_argument_types;
        // TODO: are these needed?
        // Node::DataType* abi_argument_types;
        // u32 abi_argument_count;
        DebugType* original_return_type;
        AbiInfo return_type_abi;
        u32 original_argument_count;
        // TODO: is this needed?
        // Node::DataType abi_return_type;
        u8 varags:1;
    };

    Symbol symbol;
    Node* root_node;
    Node* stop_node;
    Node** parameters;
    Function::Prototype prototype;
    // u32 node_count;
    u32 uid;
    u16 parameter_count;

    method Node* iterate(Thread* thread);
};

struct ConstantIntData
{
    u64 value;
    Node* input;
};

struct File;
// This is a node in the "sea of nodes" sense:
// https://en.wikipedia.org/wiki/Sea_of_nodes
struct Node
{
    enum class Id: u8
    {
        ROOT,
        STOP,
        PROJECTION,
        RETURN,
        IF,
        CONSTANT,
        SCOPE,
        SYMBOL_FUNCTION,
        CALL,
        REGION,
        REGION_LOOP,
        PHI,

        INTEGER_ADD,
        INTEGER_SUB,

        INTEGER_COMPARE_EQUAL,
        INTEGER_COMPARE_NOT_EQUAL,
        INTEGER_COMPARE_LESS,
        INTEGER_COMPARE_LESS_EQUAL,
        INTEGER_COMPARE_GREATER,
        INTEGER_COMPARE_GREATER_EQUAL,
    };

    using Type = NodeType;

    Array<Node*> inputs = {};
    Array<Node*> outputs = {};
    Array<Node*> dependencies = {};
    Type* type = 0;
    u32 uid;
    Hash hash = 0;
    Id id;
    s32 immediate_depth = 0;

    union
    {
        struct
        {
            String name;
            s32 index;
        } projection;
        struct
        {
            Array<StringMap<u16>> stack;
        } scope;
        struct
        {
            Type* args;
            Function* function;
        } root;
        Symbol* symbol;
        struct
        {
            String label;
        } phi;
        struct
        {
            Node* immediate_dominator = 0;
        } region;
        struct
        {
            Type* type;
        } constant;
    } payload;

    u8 padding[20 ] = {};

    method forceinline Slice<Node*> get_inputs()
    {
        return {
            .pointer = inputs.pointer,
            .length = inputs.length,
        };
    }

    method forceinline Slice<Node*> get_outputs()
    {
        return {
            .pointer = outputs.pointer,
            .length = outputs.length,
        };
    }

    struct NodeData
    {
        Slice<Node*> inputs;
        Id id;
    };

    [[nodiscard]] fn Node* add(Thread* thread, NodeData data)
    {
        auto node_id = thread->node_count;
        thread->node_count += 1;

        auto* node = thread->nodes.add_one();
        *node = {
            .uid = node_id,
            .id = data.id,
            .payload = {},
        };

        node->inputs.append(data.inputs);

        for (Node* input : data.inputs)
        {
            if (input)
            {
                input->add_output(node);
            }
        }

        return node;
    }


    method Node* add_output(Node* output)
    {
        outputs.append_one(output);
        return this;
    }

    method Node* add_input(Thread* thread, Node* input)
    {
        unlock(thread);
        inputs.append_one(input);
        if (input)
        {
            input->add_output(this);
        }
        return input;
    }

    method Node* add_dependency(Thread* thread, Node* dependency)
    {
        if (thread->mid_assert)
        {
            return this;
        }

        if (dependencies.slice().find(dependency))
        {
            return this;
        }

        if (inputs.slice().find(dependency))
        {
            return this;
        }

        if (outputs.slice().find(dependency))
        {
            return this;
        }

        dependencies.append_one(dependency);

        return this;
    }

    method Node* set_input(Thread* thread, s32 index, Node* new_input)
    {
        unlock(thread);
        Node* old_input = inputs[index];
        if (old_input == new_input)
        {
            return this;
        }

        if (new_input)
        {
            new_input->add_output(this);
        }

        if (old_input && old_input->remove_output(this))
        {
            old_input->kill(thread);
        }

        inputs[index] = new_input;

        return new_input;
    }

    method u8 remove_output(Node* output)
    {
        s32 index = outputs.slice().find_index(output);
        assert(index != -1);
        outputs.remove_swap(index);
        return outputs.length == 0;
    }

    method void remove_input(Thread* thread, u32 index)
    {
        unlock(thread);
        if (Node* old_input = inputs[index])
        {
            if (old_input->remove_output(this))
            {
                old_input->kill(thread);
            }
        }

        inputs.remove_swap(index);
    }

    method Node* idealize(Thread* thread, Function* function)
    {
        switch (id)
        {
        case Id::INTEGER_SUB:
            if (inputs[1] == inputs[2])
            {
                trap();
            }
            else
            {
                return 0;
            }
        case Id::CONSTANT:
            return 0;
        case Id::STOP:
            {
                auto input_count = inputs.length;
                for (u32 i = 0; i < inputs.length; i += 1)
                {
                    if (inputs[i]->type == thread->common_types.dead_control)
                    {
                        remove_input(thread, i);
                        i -= 1;
                    }
                }

                if (input_count != inputs.length)
                {
                    return this;
                }
                else
                {
                    return 0;
                }
            }
        case Id::PROJECTION:
            {
                auto* control = get_control();
                if (control->type->id == NodeType::Id::MULTIVALUE)
                {
                    auto control_types = control->type->payload.multi.types;
                    auto projection_index = payload.projection.index;
                    if (control_types[projection_index] == thread->common_types.dead_control)
                    {
                        trap();
                    }

                    // TODO: fix

                    // auto index = 1 - projection_index;
                    // assert(index >= 0);
                    // assert((u32)index < control_types.length);
                    // if (control_types[index] == thread->common_types.dead_control)
                    // {
                    //     trap();
                    // }
                }
                
                return 0;
            }
        case Id::ROOT:
            return 0;
        case Id::IF:
            if (!predicate()->type->is_high_or_const())
            {
                for (Node* dominator = get_immediate_dominator(), *prior = this; dominator; prior = dominator, dominator = dominator->get_immediate_dominator())
                {
                    auto* dep = dominator->add_dependency(thread, this);
                    if (dep->id == Id::IF && dep->predicate()->add_dependency(thread, this) == predicate() && prior->id == Id::PROJECTION)
                    {
                        trap();
                    }
                }
            }

            return 0;
        case Id::INTEGER_ADD:
            {
                auto* left = inputs[1];
                auto* right = inputs[2];
                assert(!(left->type->is_constant() && right->type->is_constant()));

                if (right->type->id == NodeType::Id::INTEGER && right->type->payload.constant.constant == 0)
                {
                    return left;
                }

                if (left == right)
                {
                    trap();
                }

                return 0;
            }
        case Id::REGION_LOOP:
        case Id::REGION:
            if (!region_in_progress())
            {
                // Find dead input
                for (u32 i = 1; i < inputs.length; i += 1)
                {
                    if (inputs[i]->type == thread->common_types.dead_control)
                    {
                        for (u32 output_index = 0; output_index < outputs.length; output_index += 1)
                        {
                            Node* output = outputs[output_index];
                            if (output->id == Id::PHI)
                            {
                                output->remove_input(thread, i);
                            }
                        }

                        remove_input(thread, i);

                        if (inputs.length == 2)
                        {
                            for (u32 output_index = 0; output_index < outputs.length; output_index += 1)
                            {
                                Node* output = outputs[output_index];
                                if (output->id == Id::PHI)
                                {
                                    // TODO:
                                    trap();
                                }
                            }

                            return inputs[1];
                        }
                        else
                        {
                            trap();
                        }
                    }
                }
            }

            return 0;
        case Id::SCOPE:
            return 0;
        // TODO:
        case Id::SYMBOL_FUNCTION:
        case Id::CALL:
            return 0;
        case Id::INTEGER_COMPARE_EQUAL:
        case Id::INTEGER_COMPARE_NOT_EQUAL:
        case Id::INTEGER_COMPARE_LESS:
        case Id::INTEGER_COMPARE_LESS_EQUAL:
        case Id::INTEGER_COMPARE_GREATER:
        case Id::INTEGER_COMPARE_GREATER_EQUAL:
            if (inputs[1] == inputs[2])
            {
                trap();
            }
            else
            {
                return 0;
            }
        case Id::PHI:
            {
                auto* region = phi_get_region();
                if (!region->is_region())
                {
                    return inputs[1];
                }

                if (region->region_in_progress() || region->inputs.length <= 1)
                {
                    return 0;
                }

                Node* single_unique_input = 0;
                if (!(region->id == Id::REGION_LOOP && region->loop_entry()->type == thread->common_types.dead_control))
                {
                    Node* live = 0;

                    for (u32 i = 1; i < inputs.length; i += 1)
                    {
                        if (region->inputs[i]->add_dependency(thread, this)->type != thread->common_types.dead_control && inputs[i] != this)
                        {
                            if (!live || live == inputs[i])
                            {
                                live = inputs[i];
                            }
                            else
                            {
                                live = 0;
                                break;
                            }
                        }
                    }

                    single_unique_input = live;
                }

                if (single_unique_input)
                {
                    return single_unique_input;
                }

                Node* operand = inputs[1];

                if (operand->inputs.length == 3 && !operand->inputs[0] && !operand->is_cfg() && phi_same_operand())
                {
                    u32 input_count = inputs.length;
                    auto lefts = thread->arena->allocate_slice<Node*>(input_count);
                    auto rights = thread->arena->allocate_slice<Node*>(input_count);

                    lefts[0] = inputs[0];
                    rights[0] = inputs[0];

                    for (u32 i = 1; i < input_count; i += 1)
                    {
                        lefts[i] = inputs[i]->inputs[1];
                        rights[i] = inputs[i]->inputs[2];
                    }

                    auto* left_phi = Node::add(thread, {
                            .inputs = lefts,
                            .id = Node::Id::PHI,
                            });
                    left_phi->payload.phi.label = payload.phi.label;
                    left_phi = left_phi->peephole(thread, function);

                    auto* right_phi = Node::add(thread, {
                            .inputs = rights,
                            .id = Node::Id::PHI,
                            });
                    right_phi->payload.phi.label = payload.phi.label;
                    right_phi = right_phi->peephole(thread, function);

                    auto* result = operand->copy(thread, left_phi, right_phi);
                    return result;
                }

                return 0;
            }
        case Id::RETURN:
            {
                if (get_control()->type->id == Node::Type::Id::DEAD_CONTROL)
                {
                    trap();
                }
                else
                {
                    return 0;
                }
            }
        }
    }

    method Node* copy(Thread* thread, Node* left, Node* right)
    {
        switch (id)
        {
            case Id::INTEGER_COMPARE_EQUAL:
            case Id::INTEGER_COMPARE_NOT_EQUAL:
            case Id::INTEGER_COMPARE_LESS:
            case Id::INTEGER_COMPARE_LESS_EQUAL:
            case Id::INTEGER_COMPARE_GREATER:
            case Id::INTEGER_COMPARE_GREATER_EQUAL:
            case Id::INTEGER_ADD:
                {
                    Node* inputs[] = { 0, left, right };
                    auto* result = Node::add(thread, {
                        .inputs = array_to_slice(inputs),
                        .id = id,
                    });
                    return result;
                } break;
            default:
                trap();
        }
    }

    method u8 is_cfg()
    {
        switch (id)
        {
        case Id::ROOT:
        case Id::STOP:
        case Id::RETURN:
        case Id::REGION:
        case Id::REGION_LOOP:
        case Id::IF:
            return 1;
        case Id::PROJECTION:
            return (payload.projection.index == 0) || (get_control()->id == Node::Id::IF);
        default:
            return 0;
        }
    }

    method u8 phi_same_operand()
    {
        assert(id == Id::PHI);
        auto input_class = inputs[1]->id;
        for (u32 i = 2; i < inputs.length; i += 1)
        {
            auto other_input_class = inputs[i]->id;
            if (input_class != other_input_class)
            {
                return 0;
            }
        }

        return 1;
    }

    method u8 phi_same_inputs()
    {
        assert(id == Id::PHI);
        auto* input = inputs[1];
        for (u32 i = 2; i < inputs.length; i += 1)
        {
            if (input != inputs[i])
            {
                return 0;
            }
        }

        return 1;
    }

    method u8 is_unused()
    {
        return outputs.length == 0;
    }

    method u8 is_dead()
    {
        return is_unused() & (inputs.length == 0) && !type;
    }

    method void pop_inputs(Thread* thread, u32 count)
    {
        unlock(thread);
        for (u32 i = 0; i < count; i += 1)
        {
            Node* old_input = inputs.pop();
            if (old_input)
            {
                if (old_input->remove_output(this))
                {
                    old_input->kill(thread);
                }
            }
        }
    }

    method String get_id_name()
    {
        switch (id)
        {
            case_to_name(Id, ROOT);
            case_to_name(Id, STOP);
            case_to_name(Id, PROJECTION);
            case_to_name(Id, RETURN);
            case_to_name(Id, IF);
            case_to_name(Id, CONSTANT);
            case_to_name(Id, SCOPE);
            case_to_name(Id, SYMBOL_FUNCTION);
            case_to_name(Id, CALL);
            case_to_name(Id, REGION);
            case_to_name(Id, REGION_LOOP);
            case_to_name(Id, PHI);
            case_to_name(Id, INTEGER_ADD);
            case_to_name(Id, INTEGER_SUB);
            case_to_name(Id, INTEGER_COMPARE_EQUAL);
            case_to_name(Id, INTEGER_COMPARE_NOT_EQUAL);
            case_to_name(Id, INTEGER_COMPARE_LESS);
            case_to_name(Id, INTEGER_COMPARE_LESS_EQUAL);
            case_to_name(Id, INTEGER_COMPARE_GREATER);
            case_to_name(Id, INTEGER_COMPARE_GREATER_EQUAL);
        }
    }

    method void kill(Thread* thread)
    {
        unlock(thread);
        assert(is_unused());
        type = 0;

        while (inputs.length > 0)
        {
            if (Node* old_input = inputs.pop())
            {
                thread->worklist.push(old_input);
                if (old_input->remove_output(this))
                {
                    old_input->kill(thread);
                }
            }
        }

        assert(is_dead());
    }

    global auto constexpr enable_peephole = 1;

    [[gnu::hot]] method Node* peephole(Thread* thread, Function* function)
    {
        if (!enable_peephole)
        {
            type = compute(thread);
            return this;
        }
        else
        {
            if (Node* node = peephole_optimize(thread, function))
            {
                auto* new_node = node->peephole(thread, function);
                auto* result = dead_code_elimination(thread, new_node);
                return result;
            }
            else
            {
                return this;
            }
        }
    }

    method Node::Type* set_type(Thread* thread, Node::Type* new_type)
    {
        auto* old_type = type;

        assert(!old_type || new_type->is_a(thread, old_type));

        if (old_type != new_type)
        {
            type = new_type;
            thread->worklist.add_many(outputs.slice());
            move_dependencies_to_worklist(thread);
        }

        return old_type;
    }

    method void move_dependencies_to_worklist(Thread* thread)
    {
        thread->worklist.add_many(dependencies.slice());
        dependencies.clear();
    }

    [[gnu::hot]] method Node* peephole_optimize(Thread* thread, Function* function)
    {
        thread->peephole_iteration_count += 1;
        auto old_type = set_type(thread, compute(thread));
        if (!is_constant() && type->is_high_or_const())
        {
            auto* constant_node = Node::add(thread, {
                .inputs = { .pointer = &function->root_node, .length = 1 },
                .id = Id::CONSTANT,
            });
            constant_node->payload.constant.type = type;
            return constant_node->peephole_optimize(thread, function);
        }

        if (!hash)
        {
            auto gop = thread->interned_nodes.get_or_put(thread->arena, this);
            // print("{s} node {u64:x} ({u64:x}) -> {u64:x} ({u64:x})\n", gop.existing ? strlit("Getting") : strlit("Adding"), this, this->get_hash(), gop.result, gop.result->get_hash());
            if (gop.existing)
            {
                Node* found = gop.result;
                found->set_type(thread, found->type->join(thread, type));
                hash = 0;
                auto* result = dead_code_elimination(thread, found);
                return result;
            }
        }

        Node* node = idealize(thread, function);
        if (node)
        {
            return node;
        }
        else
        {
            u8 condition = old_type == type;
            thread->peephole_nop_iteration_count += condition;
            auto* result = condition ? 0 : this;
            return result;
        }
    }

    method Node* keep()
    {
        return add_output(0);
    }

    method Node* unkeep()
    {
        remove_output(0);
        return this;
    }

    method u8 is_constant()
    {
        switch (id)
        {
            default:
                return 0;
            case Id::CONSTANT:
                return 1;
        }
    }

    method Node* predicate()
    {
        assert(id == Id::IF);
        return inputs[1];
    }

    method Node::Type* compute(Thread* thread)
    {
        switch (id)
        {
            case Node::Id::ROOT:
                return payload.root.args;
            case Node::Id::STOP:
                return thread->common_types.bottom;
            case Node::Id::IF:
                {
                    auto* control_node = get_control();
                    if (control_node->type != thread->common_types.live_control && control_node->type != thread->common_types.bottom)
                    {
                        return thread->common_types.if_neither;
                    }

                    auto* this_predicate = predicate();
                    auto* predicate_type = this_predicate->type;

                    if (predicate_type == thread->common_types.top || predicate_type == thread->common_types.integer_top)
                    {
                        return thread->common_types.if_neither;
                    }

                    if ((predicate_type->id == Node::Type::Id::INTEGER) & predicate_type->is_constant())
                    {
                        auto value = predicate_type->payload.constant.constant;
                        if (value)
                        {
                            return thread->common_types.if_true;
                        }
                        else
                        {
                            return thread->common_types.if_false;
                        }
                    }

                    return thread->common_types.if_both;
                }
            case Node::Id::INTEGER_ADD:
            case Node::Id::INTEGER_SUB:
            case Node::Id::INTEGER_COMPARE_EQUAL:
            case Node::Id::INTEGER_COMPARE_NOT_EQUAL:
            case Node::Id::INTEGER_COMPARE_LESS:
            case Node::Id::INTEGER_COMPARE_LESS_EQUAL:
            case Node::Id::INTEGER_COMPARE_GREATER:
            case Node::Id::INTEGER_COMPARE_GREATER_EQUAL:
                {
                    auto left_type = inputs[1]->type;
                    auto right_type = inputs[2]->type;

                    if ((left_type->id == Node::Type::Id::INTEGER) & (right_type->id == Node::Type::Id::INTEGER))
                    {
                        if (left_type->is_constant() & right_type->is_constant())
                        {
                            u64 result;
                            switch (id)
                            {
                                default:
                                    trap();
                                case Id::INTEGER_ADD:
                                    result = left_type->payload.constant.constant + right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_SUB:
                                    result = left_type->payload.constant.constant - right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_EQUAL:
                                    result = left_type->payload.constant.constant == right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_NOT_EQUAL:
                                    result = left_type->payload.constant.constant != right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_LESS:
                                    result = left_type->payload.constant.constant < right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_LESS_EQUAL:
                                    result = left_type->payload.constant.constant <= right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_GREATER:
                                    result = left_type->payload.constant.constant > right_type->payload.constant.constant;
                                    break;
                                case Id::INTEGER_COMPARE_GREATER_EQUAL:
                                    result = left_type->payload.constant.constant >= right_type->payload.constant.constant;
                                    break;
                            }

                            return thread->types.append_one({
                                .payload = {
                                    .constant = {
                                        .constant = result,
                                        .is_constant = 1,
                                    },
                                },
                                .id = Node::Type::Id::INTEGER,
                            })->intern_type(thread);
                        }
                    }

                    auto* result = left_type->meet(thread, right_type);
                    return result;
                }
            case Node::Id::CONSTANT:
                return payload.constant.type;
            case Node::Id::PROJECTION:
                {
                    auto* control_node = inputs[0];
                    if (control_node->type->id == NodeType::Id::MULTIVALUE)
                    {
                        auto type = control_node->type->payload.multi.types[this->payload.projection.index];
                        return type;
                    }
                    else
                    {
                        trap();
                    }
                } break;
            // TODO: change
            case Node::Id::SYMBOL_FUNCTION:
                trap();
                // return { .id = Type::Id::FUNCTION };
            case Node::Id::CALL:
                {
                    // TODO: undo this mess
                    return thread->common_types.integer_bot;
                }
                // return { .id = Type::Id::CALL };
            case Node::Id::RETURN:
                {
                    Array<Type*> types = {};
                    // First INPUT: control
                    // Second INPUT: expression
                    types.append_one(inputs[0]->type);
                    types.append_one(inputs[1]->type);

                    auto* ty = thread->types.append_one({
                        .payload = {
                            .multi = {
                                .types = types.slice(),
                            },
                        },
                        .id = Node::Type::Id::MULTIVALUE,
                    });
                    auto* interned_type = ty->intern_type(thread);
                    // print("Ty: {u64:x} -> interned {u64:x}\n", ty, interned_type);
                    return interned_type;
                }
            case Node::Id::REGION_LOOP:
                if (region_in_progress())
                {
                    return thread->common_types.live_control;
                }
                else
                {
                    auto* entry = loop_entry();
                    return entry->type;
                }
            case Node::Id::REGION:
                if (region_in_progress())
                {
                    trap();
                    // return { .id = Type::Id::LIVE_CONTROL };
                }
                else
                {
                    auto* ty = thread->common_types.dead_control;
                    for (u32 i = 1; i < inputs.length; i += 1)
                    {
                        ty = ty->meet(thread, inputs[i]->type);
                    }
                    return ty;
                }
            case Node::Id::PHI:
                {
                    auto* region = phi_get_region();
                    auto is_reg = region->is_region();
                    if (!is_reg)
                    {
                        trap();
                    }
                    if (region->region_in_progress())
                    {
                        return thread->common_types.bottom;
                    }

                    Type* ty = thread->common_types.top;
                    for (u32 i = 1; i < inputs.length; i += 1)
                    {
                        if (region->inputs[i]->add_dependency(thread, this)->type != thread->common_types.dead_control && inputs[i] != this)
                        {
                            ty = ty->meet(thread, inputs[i]->type);
                        }
                    }

                    return ty;
                }
            case Id::SCOPE:
                return thread->common_types.bottom;
        }
    }

    u8 is_region()
    {
        switch (id)
        {
            case Id::REGION: case Id::REGION_LOOP:
                return 1;
            default:
                return 0;
        }
    }

    method u8 is_associative()
    {
        switch (id)
        {
            case Id::INTEGER_ADD:
                return 1;
            default:
                return 0;
        }
    }
    method Node* associative_phi_constant(u8 should_rotate)
    {
        unused(should_rotate);
        assert(is_associative());
        trap();
    }

    method Node* project(Thread* thread, Node* control, s32 index, String label)
    {
        assert(type->id == Node::Type::Id::MULTIVALUE);
        auto* projection = Node::add(thread, {
            .inputs = { .pointer = &control, .length = 1 },
            .id = Node::Id::PROJECTION,
        });
        projection->payload.projection.index = index;
        projection->payload.projection.name = label;
        return projection;
    }

    [[gnu::hot]] method Node* dead_code_elimination(Thread* thread, Node* new_node)
    {
        if (new_node != this && is_unused())
        {
            new_node->keep();
            kill(thread);
            new_node->unkeep();
        }

        return new_node;
    }

    method DebugType* get_debug_type(Unit* unit)
    {
        switch (type->id)
        {
        case NodeType::Id::BOTTOM:
            // TODO:
            return unit->get_integer_type(32, 1);
        case NodeType::Id::TOP:
            trap();
        case NodeType::Id::LIVE_CONTROL:
        case NodeType::Id::DEAD_CONTROL:
            trap();
        case NodeType::Id::INTEGER:
            // TODO:
            return unit->get_integer_type(32, 1);
        case NodeType::Id::MULTIVALUE:
            trap();
        case NodeType::Id::FUNCTION:
            trap();
        case NodeType::Id::CALL:
            trap();
        }
    }

    method Node* get_control()
    {
        switch (id)
        {
            case Node::Id::SCOPE:
            case Node::Id::RETURN:
            case Node::Id::IF:
            case Node::Id::PROJECTION:
                return inputs[0];
            default:
                trap();
        }
    }

    method Node* swap_inputs_1_2(Thread* thread)
    {
        unlock(thread);
        Node* temporal = inputs[1];
        inputs[1] = inputs[2];
        inputs[2] = temporal;
        return this;
    }

    method u8 all_constants()
    {
        if (id == Id::PHI)
        {
            auto* region = phi_get_region();
            auto is_r = region->is_region();
            if (!is_r || region->region_in_progress())
            {
                return 0;
            }
        }

        for (u32 i = 1; i < inputs.length; i += 1)
        {
            if (!inputs[i]->type->is_constant())
            {
                return 0;
            }
        }

        return 1;
    }

    method Node* get_immediate_dominator()
    {
        switch (id)
        {
            case Id::ROOT:
                return 0;
            case Id::REGION_LOOP:
                return loop_entry();
            case Id::REGION:
                if (payload.region.immediate_dominator)
                {
                    return payload.region.immediate_dominator;
                }
                else
                {
                    if (inputs.length == 3)
                    {
                        Node* left = inputs[1]->get_immediate_dominator();
                        Node* right = inputs[2]->get_immediate_dominator();

                        while (left != right)
                        {
                            if (!left || !right)
                            {
                                return 0;
                            }
                            else
                            {
                                auto comp = left->immediate_depth - right->immediate_depth;

                                if (comp >= 0)
                                {
                                    left = left->get_immediate_dominator();
                                }

                                if (comp <= 0)
                                {
                                    right = right->get_immediate_dominator();
                                }
                            }
                        }

                        if (left)
                        {
                            immediate_depth = left->immediate_depth + 1;
                            payload.region.immediate_dominator = left;
                            return left;
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        trap();
                    }
                }
            default:
                {
                    Node* result = inputs[0];
                    if (result->immediate_depth == 0)
                    {
                        result->get_immediate_dominator();
                    }
                    if (immediate_depth == 0)
                    {
                        immediate_depth = result->immediate_depth + 1;
                    }

                    return result;
                }
        }
    }

    method u8 region_in_progress()
    {
        assert(is_region());
        return inputs.length > 1 && !(inputs[inputs.length - 1]);
    }

    method u8 phi_in_progress()
    {
        assert(id == Id::PHI);
        return !(inputs[inputs.length - 1]);
    }

    method Node* loop_entry()
    {
        assert(id == Id::REGION_LOOP);
        return inputs[1];
    }

    method Node* loop_backedge()
    {
        assert(id == Id::REGION_LOOP);
        return inputs[2];
    }

    method Node* set_control(Thread* thread, Node* node)
    {
        assert(id == Id::SCOPE);

        return set_input(thread, 0, node);
    }

    method Node* phi_get_region()
    {
        assert(id == Id::PHI);
        return inputs[0];
    }

    method void subsume(Thread* thread, Node* node)
    {
        assert(node != this);

        while (outputs.length > 0)
        {
            Node* n = outputs.pop();
            n->unlock(thread);
            s32 index = n->inputs.slice().find_index(this);
            assert(index != -1);
            n->inputs[index] = node;
            node->add_output(n);
        }

        kill(thread);
    }

    method Slice<String> scope_reverse_names(Arena* arena)
    {
        assert(id == Node::Id::SCOPE);
        Slice<String> names = arena->allocate_slice<String>(inputs.length);

        for (auto& string_map : payload.scope.stack.slice())
        {
            for (auto& pair : string_map)
            {
                auto name = pair.key;
                if (name.length > 0)
                {
                    auto index = *string_map.get(name);
                    names[index] = name;
                }
            }
        }

        return names;
    }

    method Node* scope_update_extended(Thread* thread, Function* function, String name, Node* node, s32 nesting_level)
    {
        assert(id == Id::SCOPE);
        if (nesting_level < 0)
        {
            return 0;
        }

        // TODO: avoid recursion
        auto& map = payload.scope.stack[nesting_level];
        if (auto* index_ptr = map.get(name))
        {
            auto index = *index_ptr;
            auto* old = get_inputs()[index];

            if (old->id == Node::Id::SCOPE)
            {
                auto* loop = old;
                if (loop->inputs[index]->id == Id::PHI && loop->get_control() == loop->inputs[index]->phi_get_region())
                {
                    old = loop->inputs[index];
                }
                else
                {

                    Node* phi_inputs[] = {
                        loop->get_control(),
                        loop->scope_update_extended(thread, function, name, 0, nesting_level),
                        0,
                    };
                    auto* phi_node = Node::add(thread, {
                        .inputs = array_to_slice(phi_inputs),
                        .id = Node::Id::PHI,
                    });
                    phi_node->payload.phi.label = name;
                    phi_node = phi_node->peephole(thread, function);
                    old = loop->set_input(thread, index, phi_node);
                }

                set_input(thread, index, old);
            }

            if (node)
            {
                return set_input(thread, index, node);
            }
            else
            {
                return old;
            }
        }
        else
        {
            return scope_update_extended(thread, function, name, node, nesting_level - 1);
        }
    }

    method Node* scope_update(Thread* thread, Function* function, String name, Node* node)
    {
        assert(id == Id::SCOPE);
        return scope_update_extended(thread, function, name, node, payload.scope.stack.length - 1);
    }

    method void scope_end_loop(Thread* thread, Function* function, Node* back, Node* exit)
    {
        assert(id == Id::SCOPE);
        assert(back->id == Id::SCOPE);
        assert(exit->id == Id::SCOPE);

        Node* control_node = get_control();
        assert(control_node->id == Id::REGION_LOOP);
        assert(control_node->region_in_progress());
        control_node->set_input(thread, 2, back->get_control());
        for (u32 i = 1; i < inputs.length; i += 1)
        {
            if (back->inputs[i] != this)
            {
                auto* phi = inputs[i];
                assert(phi->id == Id::PHI);
                assert(phi->phi_get_region() == get_control());
                assert(!phi->inputs[2]);
                phi->set_input(thread, 2, back->inputs[i]);
            }

            if (exit->inputs[i] == this)
            {
                exit->set_input(thread, i, inputs[i]);
            }
        }

        back->kill(thread);

        for (u32 i = 1; i < inputs.length; i += 1)
        {
            auto* node = inputs[i];
            if (node->id == Id::PHI)
            {
                Node* input = node->peephole(thread, function);
                if (input != node)
                {
                    node->subsume(thread, input);
                    set_input(thread, i, input);
                }
            }
        }
    }

    method Hash get_hash()
    {
        if (!hash)
        {
            Hash new_hash;
            switch (id)
            {
                case Id::PROJECTION:
                    new_hash = payload.projection.index;
                    break;
                case Id::CONSTANT:
                    new_hash = payload.constant.type->get_hash();
                    break;
                default:
                    new_hash = 0;
                    break;
            }

            for (Node* input : inputs.slice())
            {
                if (input)
                {
                    new_hash = new_hash ^ (new_hash << 17) ^ (new_hash >> 13) ^ input->uid;
                }
            }

            if (!new_hash)
            {
                new_hash = 0xDEADBEEF;
            }

            hash = new_hash;
        }

        // print("Hashing node {u64:x} -> {u32:x}\n", this, hash);

        return hash;
    }

    method void unlock(Thread* thread)
    {
        // print("Removing node {u64:x} (in: {u32}, out: {u32}, id: {s}, hash: {u64:x})\n", this, inputs.length, outputs.length, get_id_name(), hash);
        if (hash)
        {
            Node* old = thread->interned_nodes.remove(this);
            if (old != this)
            {
                // print("Tried to remove node {u64:x} ({u64:x}), but got {u64:x} instead ({u64:x})\n", this, this->hash, old, old->hash);
                trap();
            }
            hash = 0;
        }
    }

    method u8 equal(Node* other)
    {
        if (this == other)
        {
            return 1;
        }
        else if (id != other->id)
        {
            return 0;
        }
        else if (inputs.length != other->inputs.length)
        {
            return 0;
        }
        else
        {
            u32 input_count = inputs.length;
            for (u32 i = 0; i < input_count; i += 1)
            {
                if (inputs[i] != other->inputs[i])
                {
                    return 0;
                }
            }

            switch (id)
            {
            case Id::PROJECTION:
                return payload.projection.index == other->payload.projection.index;
            case Id::PHI:
                return !phi_in_progress();
            case Id::CONSTANT:
                return type == other->type;
            case Id::REGION:
            case Id::REGION_LOOP:
                trap();
            case Id::ROOT:
                return payload.root.function == other->payload.root.function;
            case Id::SCOPE:
                return 1;
            case Id::STOP:
            case Id::RETURN:
            case Id::IF:
            case Id::SYMBOL_FUNCTION:
            case Id::CALL:
            case Id::INTEGER_ADD:
            case Id::INTEGER_SUB:
            case Id::INTEGER_COMPARE_EQUAL:
            case Id::INTEGER_COMPARE_NOT_EQUAL:
            case Id::INTEGER_COMPARE_LESS:
            case Id::INTEGER_COMPARE_LESS_EQUAL:
            case Id::INTEGER_COMPARE_GREATER:
            case Id::INTEGER_COMPARE_GREATER_EQUAL:
                trap();
            }
        }
    }

    method Node* walk(Thread* thread, Function* function, Node* (*callback)(Thread*, Function*, Node*))
    {
        assert(thread->visited.is_empty());
        Node* node = walk_internal(thread, function, callback);
        thread->visited.clear();
        return node;
    }


    method Node* walk_internal(Thread* thread, Function* function, Node* (*callback)(Thread*, Function*, Node*))
    {
        if (!thread->visited.get(this->uid))
        {
            thread->visited.set_assert_unset(this->uid);
            if (Node* node = callback(thread, function, this))
            {
                return node;
            }

            Slice<Node*> lists[] = {inputs.slice(), outputs.slice()};
            for (auto list : lists)
            {
                for (Node* node : list)
                {
                    if (node)
                    {
                        if (Node* result = node->walk_internal(thread, function, callback))
                        {
                            return result;
                        }
                    }
                }
            }
        }

        return 0;
    }

    method Node* scope_lookup(Thread* thread, Function* function, File* file, String name);
    method Node* merge_scopes(Thread* thread, File* file, Function* function, Node* other);
};

Node* Function::iterate(Thread* thread)
{
    assert(thread->progress_on_list(this, stop_node));
    u64 count = 0;
    unused(count);

    while (auto* node = thread->worklist.pop())
    {
        if (!node->is_dead())
        {
            count += 1;
            if (Node* x = node->peephole_optimize(thread, this))
            {
                if (!x->is_dead())
                {
                    if (!x->type)
                    {
                        x->set_type(thread, x->compute(thread));
                    }

                    if (x != node || x->id != Node::Id::CONSTANT)
                    {
                        for (Node* output : node->outputs.slice())
                        {
                            thread->worklist.push(output);
                        }

                        thread->worklist.push(x);

                        if (x != node)
                        {
                            for (Node* input : node->inputs.slice())
                            {
                                thread->worklist.push(input);
                            }

                            node->subsume(thread, x);
                        }
                    }

                    node->move_dependencies_to_worklist(thread);
                    assert(thread->progress_on_list(this, stop_node));
                }
            }
        }
    }

    return stop_node;
}

struct File
{
    String path;
    String source_code;
    FileStatus status;
    StringMap<Node> symbols = {};
};

method Node* Node::scope_lookup(Thread* thread, Function* function, File* file, String name)
{
    auto* result = scope_update_extended(thread, function, name, nullptr, payload.scope.stack.length - 1);
    if (file && !result)
    {
        result = file->symbols.get(name);
    }

    return result;
}

method Node* Node::merge_scopes(Thread* thread, File* file, Function* function, Node* other_scope)
{
    assert(id == Node::Id::SCOPE);
    assert(other_scope->id == Node::Id::SCOPE);

    Node* region_inputs[] = {
        0,
        get_control(),
        other_scope->get_control(),
    };

    auto* region_node = set_control(thread, Node::add(thread, {
                .inputs = array_to_slice(region_inputs),
                .id = Node::Id::REGION,
                })->keep());
    auto names = scope_reverse_names(thread->arena);

    // Skip input[0] ($ctrl)
    for (u32 i = 1; i < inputs.length; i += 1)
    {
        if (inputs[i] != other_scope->inputs[i])
        {
            String label = names[i];
            Node* input_a = scope_lookup(thread, function, file, label);
            Node* input_b = other_scope->scope_lookup(thread, function, file, label);

            Node* inputs[] = {
                region_node,
                input_a,
                input_b,
            };

            auto* phi_node = Node::add(thread, {
                    .inputs = array_to_slice(inputs),
                    .id = Node::Id::PHI,
                    });
            phi_node->payload.phi.label = label;
            phi_node = phi_node->peephole(thread, function);

            set_input(thread, i, phi_node);
        }
    }

    other_scope->kill(thread);
    return region_node->unkeep()->peephole(thread, function);
}

static_assert(sizeof(Node) == 128);
static_assert(page_size % sizeof(Node) == 0);

global String integer_names[] =
{
    strlit("u1"),
    strlit("u2"),
    strlit("u3"),
    strlit("u4"),
    strlit("u5"),
    strlit("u6"),
    strlit("u7"),
    strlit("u8"),
    strlit("u9"),
    strlit("u10"),
    strlit("u11"),
    strlit("u12"),
    strlit("u13"),
    strlit("u14"),
    strlit("u15"),
    strlit("u16"),
    strlit("u17"),
    strlit("u18"),
    strlit("u19"),
    strlit("u20"),
    strlit("u21"),
    strlit("u22"),
    strlit("u23"),
    strlit("u24"),
    strlit("u25"),
    strlit("u26"),
    strlit("u27"),
    strlit("u28"),
    strlit("u29"),
    strlit("u30"),
    strlit("u31"),
    strlit("u32"),
    strlit("u33"),
    strlit("u34"),
    strlit("u35"),
    strlit("u36"),
    strlit("u37"),
    strlit("u38"),
    strlit("u39"),
    strlit("u40"),
    strlit("u41"),
    strlit("u42"),
    strlit("u43"),
    strlit("u44"),
    strlit("u45"),
    strlit("u46"),
    strlit("u47"),
    strlit("u48"),
    strlit("u49"),
    strlit("u50"),
    strlit("u51"),
    strlit("u52"),
    strlit("u53"),
    strlit("u54"),
    strlit("u55"),
    strlit("u56"),
    strlit("u57"),
    strlit("u58"),
    strlit("u59"),
    strlit("u60"),
    strlit("u61"),
    strlit("u62"),
    strlit("u63"),
    strlit("u64"),
    strlit("s1"),
    strlit("s2"),
    strlit("s3"),
    strlit("s4"),
    strlit("s5"),
    strlit("s6"),
    strlit("s7"),
    strlit("s8"),
    strlit("s9"),
    strlit("s10"),
    strlit("s11"),
    strlit("s12"),
    strlit("s13"),
    strlit("s14"),
    strlit("s15"),
    strlit("s16"),
    strlit("s17"),
    strlit("s18"),
    strlit("s19"),
    strlit("s20"),
    strlit("s21"),
    strlit("s22"),
    strlit("s23"),
    strlit("s24"),
    strlit("s25"),
    strlit("s26"),
    strlit("s27"),
    strlit("s28"),
    strlit("s29"),
    strlit("s30"),
    strlit("s31"),
    strlit("s32"),
    strlit("s33"),
    strlit("s34"),
    strlit("s35"),
    strlit("s36"),
    strlit("s37"),
    strlit("s38"),
    strlit("s39"),
    strlit("s40"),
    strlit("s41"),
    strlit("s42"),
    strlit("s43"),
    strlit("s44"),
    strlit("s45"),
    strlit("s46"),
    strlit("s47"),
    strlit("s48"),
    strlit("s49"),
    strlit("s50"),
    strlit("s51"),
    strlit("s52"),
    strlit("s53"),
    strlit("s54"),
    strlit("s55"),
    strlit("s56"),
    strlit("s57"),
    strlit("s58"),
    strlit("s59"),
    strlit("s60"),
    strlit("s61"),
    strlit("s62"),
    strlit("s63"),
    strlit("s64"),
};

fn void unit_initialize(Unit* unit)
{
    Arena* type_arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(64));
    DebugType* builtin_types = type_arena->allocate_many<DebugType>(builtin_type_count);

    *unit = {
        // .arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(4)),
        // .node_arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(64)),
        // .type_arena = type_arena,
        .builtin_types = builtin_types,
        .generate_debug_information = 1,
    };

    builtin_types[void_type_index] = {
        .size = 0,
        .alignment = 1,
        .id = DebugTypeId::VOID,
        .resolved = 1,
        .flags = 0,
        .name = strlit("void"),
    };
    builtin_types[noreturn_type_index] = {
        .size = 0,
        .alignment = 1,
        .id = DebugTypeId::NORETURN,
        .resolved = 1,
        .flags = 0,
        .name = strlit("noreturn"),
    };
    builtin_types[opaque_pointer_type_index] = {
        .size = 8,
        .alignment = 8,
        .id = DebugTypeId::POINTER,
        .resolved = 1,
        .flags = 0,
        .name = strlit("*any"),
    };
    // TODO: float types
    
    u64 i;
    for (i = integer_type_offset; i < integer_type_offset + 64; i += 1)
    {
        u64 bit_count = i - integer_type_offset + 1;
        assert(bit_count >= 1 | bit_count <= 64);
        auto aligned_bit_count = round_up_to_next_power_of_2(bit_count);
        auto byte_count = max<u64>(aligned_bit_count / 8, 1);
        assert(byte_count <= bit_count);
        assert(byte_count == 1 | byte_count == 2 | byte_count == 4 | byte_count == 8);
        builtin_types[i] =
        {
            .size = byte_count,
            .alignment = byte_count,
            .id = DebugTypeId::INTEGER,
            .resolved = 1,
            .flags = static_cast<u32>(bit_count),
            .name = integer_names[bit_count - 1],
        };
    }

    for (; i < integer_type_offset + integer_type_count; i += 1)
    {
        u64 bit_count = i - (integer_type_offset + 64 - 1);
        assert(bit_count >= 1 | bit_count <= 64);
        auto aligned_bit_count = round_up_to_next_power_of_2(bit_count);
        auto byte_count = max<u64>(aligned_bit_count / 8, 1);
        assert(byte_count <= bit_count);
        assert(byte_count == 1 | byte_count == 2 | byte_count == 4 | byte_count == 8);
        builtin_types[i] =
        {
            .size = byte_count,
            .alignment = byte_count,
            .id = DebugTypeId::INTEGER,
            .resolved = 1,
            .flags = static_cast<u32>(bit_count | (1 << (type_flags_bit_count - 1))), // Signedness bit
            .name = integer_names[bit_count + 63],
        };
    }
}


static_assert(array_length(integer_names) == 128, "Integer name array must be 128 bytes");

struct Instance
{
    Arena* arena;
};

fn Unit* instance_add_unit(Instance* instance)
{
    Unit* unit = instance->arena->allocate_one<Unit>();
    *unit = {
    };
    return unit;
}

// TODO: make it into an array
fn Thread* instance_add_thread(Instance* instance)
{
    auto* thread = instance->arena->allocate_one<Thread>();
    *thread = {
        .arena = Arena::init_default(KB(64)),
        .common_types = {},
    };
    return thread;
}

struct Parser
{
    u64 i;
    u32 line;
    u32 column;

    [[gnu::hot]] method void skip_space(String src)
    {
        u64 original_i = i;

        if (original_i != src.length)
        {
            if (is_space(src.pointer[original_i], get_next_ch_safe(src, original_i)))
            {
                while (i < src.length)
                {
                    u64 index = i;
                    u8 ch = src.pointer[index];
                    u64 new_line = ch == '\n';
                    line += new_line;

                    if (new_line)
                    {
                        column = index + 1;
                    }

                    if (!is_space(ch, get_next_ch_safe(src, i)))
                    {
                        break;
                    }

                    u32 is_comment = src.pointer[index] == '/';
                    i += is_comment + is_comment;
                    if (is_comment)
                    {
                        while (i < src.length)
                        {
                            if (src.pointer[i] == '\n')
                            {
                                break;
                            }

                            i += 1;
                        }

                        continue;
                    }

                    i += 1;
                }
            }
        }
    }

    [[gnu::hot]] method void expect_character(String src, u8 expected_ch)
    {
        u64 index = i;
        if (expect(index < src.length, 1))
        {
            u8 ch = src.pointer[index];
            u64 matches = ch == expected_ch;
            expect(matches, 1);
            i += matches;
            if (!matches)
            {
                print(strlit("expected character '"));
                print(ch_to_str(expected_ch));
                print(strlit("', but found '"));
                print(ch_to_str(ch));
                print(strlit("'\n"));
                fail();
            }
        }
        else
        {
            print(strlit("expected character '"));
            print(ch_to_str(expected_ch));
            print(strlit("', but found end of file\n"));
            fail();
        }
    }

    [[gnu::hot]] method String parse_raw_identifier(String src)
    {
        u64 identifier_start_index = i;
        u64 is_string_literal = src.pointer[identifier_start_index] == '"';
        i += is_string_literal;
        u8 identifier_start_ch = src.pointer[i];
        u64 is_valid_identifier_start = is_identifier_start(identifier_start_ch);
        i += is_valid_identifier_start;

        if (expect(is_valid_identifier_start, 1))
        {
            while (i < src.length)
            {
                u8 ch = src.pointer[i];
                u64 is_identifier = is_identifier_ch(ch);
                expect(is_identifier, 1);
                i += is_identifier;

                if (!is_identifier)
                {
                    if (expect(is_string_literal, 0))
                    {
                        expect_character(src, '"');
                    }

                    String result = src.slice(identifier_start_index, i - is_string_literal);
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

    method String parse_and_check_identifier(String src)
    {
        String identifier = parse_raw_identifier(src);

        if (expect(identifier.equal(strlit("_")), 0))
        {
            return {};
        }

        return identifier;
    }
};

// fn u32 get_line(Parser* parser)
// {
//     return parser->line + 1;
// }
//
// fn u32 get_column(Parser* parser)
// {
//     return parser->i - parser->column + 1;
// }
fn File* add_file(Arena* arena, String file_path)
{
    auto* file = arena->allocate_one<File>();
    *file = {
        .path = file_path,
        .source_code = {},
        .status = FILE_STATUS_ADDED,
    };
    return file;
}

fn void compiler_file_read(Arena* arena, File* file)
{
    assert(file->status == FILE_STATUS_ADDED || file->status == FILE_STATUS_QUEUED);
    file->source_code = file_read(arena, file->path);
    file->status = FILE_STATUS_READ;
}

global constexpr auto pointer_sign = '*';
global constexpr auto end_of_statement = ';';
global constexpr auto end_of_argument = ',';

global constexpr auto function_argument_start = parenthesis_open;
global constexpr auto function_argument_end = parenthesis_close;

global constexpr auto function_attribute_start = bracket_open;
global constexpr auto function_attribute_end = bracket_close;

global constexpr auto symbol_attribute_start = bracket_open;
global constexpr auto symbol_attribute_end = bracket_close;

global constexpr auto block_start = brace_open;
global constexpr auto block_end = brace_close;

global constexpr auto local_symbol_declaration_start = '>';

global constexpr auto array_expression_start = bracket_open;
// global constexpr auto array_expression_end = bracket_close;

// global constexpr auto composite_initialization_start = brace_open;
// global constexpr auto composite_initialization_end = brace_close;

global String function_attributes[] =
{
    strlit("cc"),
};

typedef enum FunctionAttribute
{
    FUNCTION_ATTRIBUTE_CC,
    FUNCTION_ATTRIBUTE_COUNT,
} FunctionAttribute;

static_assert(array_length(function_attributes) == FUNCTION_ATTRIBUTE_COUNT, "");

global String calling_conventions[] =
{
    strlit("c"),
    strlit("custom"),
};

typedef enum CallingConvention
{
    CALLING_CONVENTION_C,
    CALLING_CONVENTION_CUSTOM,
    CALLING_CONVENTION_COUNT,
} CallingConvention;

static_assert(array_length(calling_conventions) == CALLING_CONVENTION_COUNT, "");

typedef enum GlobalSymbolAttribute
{
    GLOBAL_SYMBOL_ATTRIBUTE_EXPORT,
    GLOBAL_SYMBOL_ATTRIBUTE_EXTERN,
    GLOBAL_SYMBOL_ATTRIBUTE_COUNT,
} GlobalSymbolAttribute;

global String global_symbol_attributes[] =
{
    strlit("export"),
    strlit("extern"),
};

struct GlobalSymbolAttributes
{
    u8 exported: 1;
    u8 external: 1;
};

static_assert(array_length(global_symbol_attributes) == GLOBAL_SYMBOL_ATTRIBUTE_COUNT, "");

Node* create_scope(Thread* thread)
{
    auto* scope = Node::add(thread, {
        .inputs = {},
        .id = Node::Id::SCOPE,
    });
    scope->type = thread->common_types.bottom;
    scope->payload.scope.stack = {};

    return scope;
}

struct Analyzer
{
    Function* function;
    Node* scope;
    Node* break_scope = 0;
    Node* continue_scope = 0;
    File* file;


    method Node* set_control(Thread* thread, Node* node)
    {
        return scope->set_control(thread, node);
    }

    method void kill_control(Thread* thread)
    {
        set_control(thread, 0);
    }

    method Node* add_return(Thread* thread, Node* return_value)
    {
        Node* inputs[] = { get_control(), return_value };

        auto* return_node = Node::add(thread, {
            .inputs = array_to_slice(inputs),
            .id = Node::Id::RETURN,
        })->peephole(thread, function);

        auto* node = function->stop_node->add_input(thread, return_node);

        // Kill control
        auto* dead_control = Node::add(thread, {
            .inputs = { .pointer = &function->root_node, .length = 1 },
            .id = Node::Id::CONSTANT,
        });
        dead_control->payload.constant.type = thread->common_types.dead_control;
        dead_control = dead_control->peephole(thread, function);
        set_control(thread, dead_control);
        
        return node;
    }

    method Node* get_control()
    {
        return scope->get_control();
    }

    method Node* duplicate_scope(Thread* thread, u8 loop)
    {
        auto* original_scope = scope;
        auto original_input_count = original_scope->inputs.length;
        auto* duplicate_scope = create_scope(thread);
        
        // // TODO: make this more efficient
        for (auto& hashmap: original_scope->payload.scope.stack.slice())
        {
            auto duplicate_hashmap = hashmap.duplicate(thread->arena);

            duplicate_scope->payload.scope.stack.append_one(duplicate_hashmap);
        }
        
        duplicate_scope->add_input(thread, get_control());
      
        for (u32 i = 1; i < original_scope->inputs.length; i += 1)
        {
            duplicate_scope->add_input(thread, loop ? original_scope : original_scope->inputs[i]);
        }
        assert(duplicate_scope->inputs.length == original_input_count);
        return duplicate_scope;
    }

};

fn DebugType* analyze_type(Parser* parser, Unit* unit, String src)
{
    u64 start_index = parser->i;
    u8 start_ch = src.pointer[start_index];
    u32 array_start = start_ch == array_expression_start;
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
    else if (array_start)
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
                    auto* result = unit->get_integer_type(bit_size, signedness);
                    return result;
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

fn u64 parse_hex(String string)
{
    u64 value = 0;
    for (u8 ch : string)
    {
        u8 byte;
        auto is_decimal = (ch >= '0') & (ch <= '9');
        auto is_lower_hex = (ch >= 'a') & (ch <= 'f');
        auto is_upper_hex = (ch >= 'A') & (ch <= 'F');
        if (is_decimal)
        {
            byte = ch - '0';
        }
        else if (is_lower_hex)
        {
            byte = ch - 'a' + 10;
        }
        else if (is_upper_hex)
        {
            byte = ch - 'A' + 10;
        }
        else
        {
            fail();
        }

        value = (value << 4) | (byte & 0x0f);
    }

    return value;
}

[[nodiscard]] [[gnu::hot]] fn Node* analyze_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src);

[[nodiscard]] [[gnu::hot]] fn Node* analyze_primary_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    parser->skip_space(src);
    auto starting_ch_index = parser->i; 
    u8 starting_ch = src[starting_ch_index];
    auto is_digit = is_decimal_digit(starting_ch);
    auto is_identifier = is_identifier_start(starting_ch);
    auto is_open_parenthesis = starting_ch == parenthesis_open;

    if (is_identifier)
    {
        String identifier = parser->parse_and_check_identifier(src);
        auto* node = analyzer->scope->scope_lookup(thread, analyzer->function, analyzer->file, identifier);
        if (!node)
        {
            fail();
        }

        return node;
    }
    else if (is_digit)
    {
        u64 value = 0;

        if (starting_ch == '0')
        {
            auto follow_up_character = src[parser->i + 1];
            auto is_hex_start = follow_up_character == 'x';
            auto is_octal_start = follow_up_character == 'o';
            auto is_bin_start = follow_up_character == 'b';
            auto is_prefixed_start = is_hex_start | is_octal_start | is_bin_start;
            auto follow_up_alpha = is_alphabetic(follow_up_character);
            auto follow_up_digit = is_decimal_digit(follow_up_character);
            auto is_valid_after_zero = is_space(follow_up_character, get_next_ch_safe(src, follow_up_character)) | (!follow_up_digit and !follow_up_alpha);

            if (is_prefixed_start) {
                enum class IntegerPrefix {
                    hexadecimal,
                    octal,
                    binary,
                };
                IntegerPrefix prefix;
                switch (follow_up_character) {
                    case 'x': prefix = IntegerPrefix::hexadecimal; break;
                    case 'o': prefix = IntegerPrefix::octal; break;
                    case 'b': prefix = IntegerPrefix::binary; break;
                    default: fail();
                };

                parser->i += 2;

                auto start = parser->i;

                switch (prefix) {
                    case IntegerPrefix::hexadecimal:
                        {
                            while (is_hex_digit(src[parser->i])) {
                                parser->i += 1;
                            }

                            auto slice = src.slice(start, parser->i);
                            value = parse_hex(slice);
                        }
                    case IntegerPrefix::octal: 
                        trap();
                    case IntegerPrefix::binary:
                        trap();
                }
            } else if (is_valid_after_zero) {
                value = 0;
                parser->i += 1;
            } else {
                fail();
            }
        }
        else
        {
            while (is_decimal_digit(src[parser->i]))
            {
                parser->i += 1;
            }

            auto slice = src.slice(starting_ch_index, parser->i);
            value = parse_decimal(slice);
        }

        NodeType* type;
        if (value == 0)
        {
            type = thread->common_types.integer_zero;
        }
        else
        {
            type = thread->types.append_one({
                    .payload = {
                    .constant = {
                    .constant = value,
                    .is_constant = 1,
                    },
                    },
                    .id = NodeType::Id::INTEGER,
                    })->intern_type(thread);
        }

        auto* constant_int = Node::add(thread, {
            .inputs = { .pointer = &analyzer->function->root_node, .length = 1 },
            .id = Node::Id::CONSTANT,
        });
        constant_int->payload.constant.type = type;
        return constant_int->peephole(thread, analyzer->function);
    }
    else if (is_open_parenthesis)
    {
        trap();
    }
    else
    {
        trap();
    }

    trap();
}

[[nodiscard]] fn Node* analyze_unary_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    parser->skip_space(src);
    Node* result;

    switch (src[parser->i])
    {
        case '-':
            trap();
        default:
            result = analyze_primary_expression(analyzer, parser, thread, src);
            break;
    }

    switch (src[parser->i])
    {
        // Function call
        case function_argument_start:
            {
                parser->i += 1;
                Array<Node*> argument_nodes = {};
                while (1)
                {
                    parser->skip_space(src);

                    if (src[parser->i] == function_argument_end)
                    {
                        break;
                    }

                    Node* argument_value = analyze_expression(analyzer, parser, thread, src)->peephole(thread, analyzer->function);
                    argument_nodes.append_one(argument_value);

                    parser->skip_space(src);

                    switch (src[parser->i])
                    {
                        case function_argument_end:
                            break;
                        case ',':
                            parser->i += 1;
                            break;
                        default:
                            fail();
                    }
                }

                parser->expect_character(src, function_argument_end);

                // Add function definition
                argument_nodes.append_one(result);

                Node* call_node = Node::add(thread, {
                        .inputs = argument_nodes.slice(),
                        .id = Node::Id::CALL,
                        })->peephole(thread, analyzer->function);
                result = call_node;
            }
        default:
            break;
    }

    return result;
}

[[nodiscard]] fn Node* analyze_multiplication_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    auto* result = analyze_unary_expression(analyzer, parser, thread, src);
    while (1)
    {
        parser->skip_space(src);

        u8 op_ch = src[parser->i];
        if (op_ch == '*')
        {
            trap();
        }
        else if (op_ch == '/')
        {
            trap();
        }
        else
        {
            break;
        }

        trap();
    }

    return result;
}

[[nodiscard]] fn Node* analyze_addition_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    auto* result = analyze_multiplication_expression(analyzer, parser, thread, src);
    while (1)
    {
        parser->skip_space(src);

        auto* left = result;
        u8 op_ch = src[parser->i];
        Node* inputs[] = {
            0,
            left,
            0,
        };

        Node::Id new_node_id;

        if (op_ch == '+')
        {
            new_node_id = Node::Id::INTEGER_ADD;
        }
        else if (op_ch == '-')
        {
            new_node_id = Node::Id::INTEGER_SUB;
        }
        else
        {
            break;
        }

        parser->i += 1;

        result = Node::add(thread, {
            .inputs = array_to_slice(inputs),
            .id = new_node_id,
        });

        auto* right = analyze_addition_expression(analyzer, parser, thread, src);
        result->set_input(thread, 2, right);
        result = result->peephole(thread, analyzer->function);
    }

    return result;
}

[[nodiscard]] fn Node* analyze_comparison_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    auto* result = analyze_addition_expression(analyzer, parser, thread, src);
    while (1)
    {
        parser->skip_space(src);

        auto* left = result;
        Node* inputs[] = {
            0,
            left,
            0,
        };
        u8 op_ch = src[parser->i];
        u8 next_ch = src[parser->i + 1];

        Node::Id id;
        u8 negate = 0;

        if (op_ch == '=' && next_ch == '=')
        {
            parser->i += 1;
            id = Node::Id::INTEGER_COMPARE_EQUAL;
        }
        else if (op_ch == '!' && next_ch == '=')
        {
            parser->i += 1;
            id = Node::Id::INTEGER_COMPARE_NOT_EQUAL;
            negate = 1;
        }
        else if (op_ch == '<')
        {
            if (next_ch == '=')
            {
                parser->i += 1;
                id = Node::Id::INTEGER_COMPARE_LESS_EQUAL;
            }
            else
            {
                id = Node::Id::INTEGER_COMPARE_LESS;
            }
        }
        else if (op_ch == '>')
        {
            if (next_ch == '=')
            {
                parser->i += 1;
                id = Node::Id::INTEGER_COMPARE_GREATER_EQUAL;
            }
            else
            {
                id = Node::Id::INTEGER_COMPARE_GREATER;
            }
        }
        else
        {
            break;
        }

        parser->i += 1;

        result = Node::add(thread, {
            .inputs = array_to_slice(inputs),
            .id = id,
        });

        auto* right = analyze_addition_expression(analyzer, parser, thread, src);
        result->set_input(thread, 2, right);

        result = result->peephole(thread, analyzer->function);
        if (negate)
        {
            trap();
        }
    }

    return result;
}

[[nodiscard]] [[gnu::hot]] fn Node* analyze_expression(Analyzer* analyzer, Parser* parser, Thread* thread, String src)
{
    return analyze_comparison_expression(analyzer, parser, thread, src);
}

fn void push_scope(Analyzer* analyzer)
{
    analyzer->scope->payload.scope.stack.append_one({});
}

fn void pop_scope(Analyzer* analyzer, Thread* thread)
{
    analyzer->scope->pop_inputs(thread, analyzer->scope->payload.scope.stack.pop().length);
}

fn Node* define_variable(Analyzer* analyzer, Thread* thread, String name, Node* node)
{
    auto* stack = &analyzer->scope->payload.scope.stack;
    assert(stack->length);
    auto* last = &stack->pointer[stack->length - 1];

    auto input_index = analyzer->scope->inputs.length;

    if (last->get_or_put(thread->arena, name, input_index).existing)
    {
        trap();
        return 0;
    }

    return analyzer->scope->add_input(thread, node);
}

fn Node* analyze_local_block(Analyzer* analyzer, Parser* parser, Unit* unit, Thread* thread, String src);

fn Node* jump_to(Analyzer* analyzer, Thread* thread, Node* target_scope)
{
    auto* current_scope = analyzer->duplicate_scope(thread, 0);
    // Kill current scope
    auto* dead_control = Node::add(thread, {
        .inputs = { .pointer = &analyzer->function->root_node, .length = 1 },
        .id = Node::Id::CONSTANT,
    });
    dead_control->payload.constant.type = thread->common_types.dead_control;
    dead_control = dead_control->peephole(thread, analyzer->function);
    analyzer->set_control(thread, dead_control);
    
    while (current_scope->payload.scope.stack.length > analyzer->break_scope->payload.scope.stack.length)
    {
        current_scope->payload.scope.stack.pop();
    }
    
    if (target_scope)
    {
        assert(target_scope->payload.scope.stack.length <= analyzer->break_scope->payload.scope.stack.length);
        auto* result = target_scope->merge_scopes(thread, analyzer->file, analyzer->function, current_scope);
        target_scope->set_control(thread, result);
        return target_scope;
    }
    else
    {
        return current_scope;
    }
}

fn Node* analyze_statement(Analyzer* analyzer, Parser* parser, Unit* unit, Thread* thread, String src)
{
    auto statement_start_index = parser->i;
    u8 statement_start_ch = src[statement_start_index];
    Function* function = analyzer->function;

    if (is_identifier_start(statement_start_ch))
    {
        Node* statement_node = 0;
        String identifier = parser->parse_raw_identifier(src);

        if (identifier.equal(strlit("return")))
        {
            parser->skip_space(src);

            auto* return_value = analyze_expression(analyzer, parser, thread, src)->peephole(thread, function);
            parser->expect_character(src, ';');

            auto* return_node = analyzer->add_return(thread, return_value);
            statement_node = return_node;
        }
        else if (identifier.equal(strlit("if")))
        {
            parser->skip_space(src);
            parser->expect_character(src, parenthesis_open);
            parser->skip_space(src);

            auto* predicate_node = analyze_expression(analyzer, parser, thread, src);

            parser->skip_space(src);
            parser->expect_character(src, parenthesis_close);
            parser->skip_space(src);

            Node* if_inputs[] = {
                analyzer->get_control(),
                predicate_node,
            };

            auto* if_node = Node::add(thread, {
                .inputs = array_to_slice(if_inputs),
                .id = Node::Id::IF,
            })->peephole(thread, function);

            Node* if_true = if_node->project(thread, if_node->keep(), 0, strlit("True"))->peephole(thread, function)->keep();
            Node* if_false = if_node->project(thread, if_node->unkeep(), 1, strlit("False"))->peephole(thread, function)->keep();

            u32 original_input_count = analyzer->scope->inputs.length;
            auto* false_scope = analyzer->duplicate_scope(thread, 0);

            analyzer->set_control(thread, if_true->unkeep());
            analyze_statement(analyzer, parser, unit, thread, src);
            parser->skip_space(src);
            auto* true_scope = analyzer->scope;

            analyzer->scope = false_scope;
            analyzer->set_control(thread, if_false->unkeep());

            if (is_identifier_start(src[parser->i]))
            {
                auto before_else = parser->i;
                String identifier = parser->parse_raw_identifier(src);
                if (identifier.equal(strlit("else")))
                {
                    parser->skip_space(src);

                    analyze_statement(analyzer, parser, unit, thread, src); 

                    false_scope = analyzer->scope;
                }
                else
                {
                    parser->i = before_else;
                }
            }

            if ((true_scope->inputs.length != original_input_count) | (false_scope->inputs.length != original_input_count))
            {
                fail();
            }

            analyzer->scope = true_scope;

            auto* merged_scope = true_scope->merge_scopes(thread, analyzer->file, analyzer->function, false_scope);
            statement_node = analyzer->set_control(thread, merged_scope);
            assert(statement_node);
        }
        else if (identifier.equal(strlit("while")))
        {
            parser->skip_space(src);
            parser->expect_character(src, parenthesis_open);
            parser->skip_space(src);

            auto* old_break_scope = analyzer->break_scope;
            auto* old_continue_scope = analyzer->continue_scope;

            Node* loop_inputs[] = {
                0,
                analyzer->get_control(),
                0,
            };
            auto* loop_node = Node::add(thread, {
                .inputs = array_to_slice(loop_inputs),
                .id = Node::Id::REGION_LOOP,
            })->peephole(thread, function);

            analyzer->set_control(thread, loop_node);

            Node* head_scope = analyzer->scope->keep();
            analyzer->scope = analyzer->duplicate_scope(thread, 1);

            auto* predicate_node = analyze_expression(analyzer, parser, thread, src);

            parser->skip_space(src);
            parser->expect_character(src, parenthesis_close);
            parser->skip_space(src);

            Node* if_inputs[] = {
                analyzer->get_control(),
                predicate_node,
            };

            auto* if_node = Node::add(thread, {
                .inputs = array_to_slice(if_inputs),
                .id = Node::Id::IF,
            })->peephole(thread, function);

            Node* if_true = if_node->project(thread, if_node->keep(), 0, strlit("True"))->peephole(thread, function)->keep();
            Node* if_false = if_node->project(thread, if_node->unkeep(), 1, strlit("False"))->peephole(thread, function);

            analyzer->set_control(thread, if_false);
            analyzer->break_scope = analyzer->duplicate_scope(thread, 0);
            analyzer->continue_scope = 0;

            analyzer->set_control(thread, if_true->unkeep());
            analyze_statement(analyzer, parser, unit, thread, src);

            if (analyzer->continue_scope)
            {
                analyzer->continue_scope = jump_to(analyzer, thread, analyzer->continue_scope);
                analyzer->scope->kill(thread);
                analyzer->scope = analyzer->continue_scope;
            }

            auto* exit_scope = analyzer->break_scope;
            head_scope->scope_end_loop(thread, function, analyzer->scope, exit_scope);
            head_scope->unkeep()->kill(thread);

            analyzer->break_scope = old_break_scope;
            analyzer->continue_scope = old_continue_scope;

            analyzer->scope = exit_scope;

            statement_node = exit_scope;
        }
        else if (identifier.equal(strlit("break")))
        {
            parser->skip_space(src);
            parser->expect_character(src, end_of_statement);

            if (!analyzer->break_scope)
            {
                fail();
            }

            analyzer->break_scope = jump_to(analyzer, thread, analyzer->break_scope);
            statement_node = analyzer->break_scope;
        }
        else if (identifier.equal(strlit("continue")))
        {
            parser->skip_space(src);
            parser->expect_character(src, end_of_statement);

            if (!analyzer->break_scope)
            {
                fail();
            }

            analyzer->continue_scope = jump_to(analyzer, thread, analyzer->continue_scope);
            statement_node = analyzer->continue_scope;
        }

        if (statement_node)
        {
            return statement_node;
        }
        else
        {
            if (auto* left_node = analyzer->scope->scope_lookup(thread, analyzer->function, analyzer->file, identifier))
            {
                parser->skip_space(src);

                enum class LoadStoreOperation : u8
                {
                    NONE
                };
                LoadStoreOperation operation;
                switch (src[parser->i])
                {
                    case '=':
                        operation = LoadStoreOperation::NONE;
                        parser->i += 1;
                        break;
                    default:
                        trap();
                }

                parser->skip_space(src);

                Node* right_expression = analyze_expression(analyzer, parser, thread, src); 

                parser->skip_space(src);
                parser->expect_character(src, ';');

                switch (operation)
                {
                    case LoadStoreOperation::NONE:
                        if (!analyzer->scope->scope_update(thread, function, identifier, right_expression))
                        {
                            fail();
                        }
                        break;
                    default:
                        trap();
                }

                return 0;
            }
            else
            {
                fail();
            }
        }
    }
    else
    {
        switch (statement_start_ch)
        {
            case local_symbol_declaration_start:
                {
                    parser->i += 1;

                    parser->skip_space(src);

                    String name = parser->parse_and_check_identifier(src);

                    u8 has_local_attributes = src[parser->i] == symbol_attribute_start;
                    parser->i += has_local_attributes;

                    if (has_local_attributes)
                    {
                        // TODO: local attributes
                        fail();
                    }

                    parser->skip_space(src);

                    struct LocalResult
                    {
                        Node* node;
                        DebugType* type;
                    };

                    LocalResult local_result = {};
                    switch (src[parser->i])
                    {
                        case ':':
                            {
                                parser->i += 1;
                                parser->skip_space(src);

                                DebugType* type = analyze_type(parser, unit, src);

                                parser->skip_space(src);
                                parser->expect_character(src, '=');
                                parser->skip_space(src);

                                auto* initial_node = analyze_expression(analyzer, parser, thread, src);
                                if (!define_variable(analyzer, thread, name, initial_node))
                                {
                                    fail();
                                }
                                local_result = {
                                    .node = initial_node,
                                    .type = type,
                                };
                            } break;
                        case '=':
                            {
                                parser->i += 1;
                                parser->skip_space(src);

                                auto* initial_node = analyze_expression(analyzer, parser, thread, src);
                                if (!define_variable(analyzer, thread, name, initial_node))
                                {
                                    fail();
                                }
                                local_result = {
                                    .node = initial_node,
                                    .type = initial_node->get_debug_type(unit),
                                };
                            } break;
                        default: fail();
                    }

                    parser->skip_space(src);
                    parser->expect_character(src, ';');

                    return local_result.node;
                } break;
            case block_start:
                {
                    return analyze_local_block(analyzer, parser, unit, thread, src);
                } break;
            default:
                trap();
        }
    }
}

fn Node* analyze_local_block(Analyzer* analyzer, Parser* parser, Unit* unit, Thread* thread, String src)
{
    push_scope(analyzer);
    parser->expect_character(src, block_start);

    while (1)
    {
        parser->skip_space(src);

        if (src[parser->i] == block_end)
        {
            break;
        }

        analyze_statement(analyzer, parser, unit, thread, src);
    }

    parser->expect_character(src, block_end);

    pop_scope(analyzer, thread);

    return 0;
}

typedef enum SystemVClass
{
    SYSTEMV_CLASS_NONE,
    SYSTEMV_CLASS_MEMORY,
    SYSTEMV_CLASS_INTEGER,
    SYSTEMV_CLASS_SSE,
    SYSTEMV_CLASS_SSEUP,
} SystemVClass;

struct SystemVClassification
{
    SystemVClass v[2];
};

struct SystemVRegisterCount
{
    u32 gp_registers;
    u32 sse_registers;
};

fn SystemVClassification systemv_classify(DebugType* type, u64 base_offset)
{
    SystemVClassification result;
    u32 is_memory = base_offset >= 8;
    u32 current_index = is_memory;
    result.v[current_index] = SYSTEMV_CLASS_MEMORY;
    result.v[!current_index] = SYSTEMV_CLASS_NONE;

    switch (type->id)
    {
        case DebugTypeId::VOID:
            trap();
        case DebugTypeId::NORETURN:
            trap();
        case DebugTypeId::POINTER:
            trap();
        case DebugTypeId::INTEGER:
        {
            u8 bit_count = type->get_bit_count();
            switch (bit_count)
            {
                case 8: case 16: case 32: case 64:
                    result.v[current_index] = SYSTEMV_CLASS_INTEGER;
                    break;
                default:
                    trap();
            }
        } break;
        case DebugTypeId::COUNT:
            trap();
        default:
            trap();
    }

    return result;
}

fn u8 contains_no_user_data(DebugType* type, u64 start, u64 end)
{
    unused(end);
    if (type->size <= start)
    {
        return 1;
    }

    switch (type->id)
    {
        case DebugTypeId::ARRAY:
            trap();
        case DebugTypeId::STRUCT:
            trap();
        case DebugTypeId::UNION:
            trap();
        default:
            return 0;
        case DebugTypeId::COUNT:
            trap();
    }
}

fn DebugType* systemv_get_int_type_at_offset(DebugType* type, u64 offset, DebugType* source_type, u64 source_offset)
{
    unused(source_type);

    switch (type->id)
    {
        case DebugTypeId::VOID:
            trap();
        case DebugTypeId::NORETURN:
            trap();
        case DebugTypeId::POINTER:
            trap();
        case DebugTypeId::INTEGER:
        {
            u8 bit_count = type->get_bit_count();
            switch (bit_count)
            {
                case 8: case 16: case 32: case 64:
                    if (offset == 0)
                    {
                        u64 start = source_offset + type->size;
                        u64 end = source_offset + 8;
                        if (contains_no_user_data(type, start, end))
                        {
                            return type;
                        }
                        trap();
                    }
                    else
                    {
                        trap();
                    }
                default:
                    trap();
            }
            trap();
        } break;
        case DebugTypeId::COUNT:
            trap();
        case DebugTypeId::ARRAY:
            trap();
        case DebugTypeId::STRUCT:
            trap();
        case DebugTypeId::UNION:
            trap();
    }
}

fn Function* analyze_function(Parser* parser, Thread* thread, Unit* unit, File* file)
{
    String src = file->source_code;
    parser->expect_character(src, 'f');
    parser->expect_character(src, 'n');

    parser->skip_space(src);
    
    u64 has_function_attributes = src.pointer[parser->i] == function_attribute_start;
    parser->i += has_function_attributes;

    CallingConvention calling_convention = CALLING_CONVENTION_CUSTOM;

    if (has_function_attributes)
    {
        u64 mask = 0;

        while (1)
        {
            parser->skip_space(src);

            if (src[parser->i] == function_attribute_end)
            {
                break;
            }

            String attribute_candidate = parser->parse_raw_identifier(src);

            u64 attribute_i;
            for (attribute_i = 0; attribute_i < array_length(function_attributes); attribute_i += 1)
            {
                String function_attribute_string = function_attributes[attribute_i];
                if (attribute_candidate.equal(function_attribute_string))
                {
                    if (mask & (1 << attribute_i))
                    {
                        fail();
                    }

                    auto function_attribute = static_cast<FunctionAttribute>(attribute_i);
                    mask |= (1 << attribute_i);

                    switch (function_attribute)
                    {
                        case FUNCTION_ATTRIBUTE_CC:
                            {
                                parser->skip_space(src);
                                parser->expect_character(src, '(');
                                parser->skip_space(src);
                                parser->expect_character(src, '.');
                                String candidate_cc = parser->parse_raw_identifier(src);
                                parser->skip_space(src);
                                parser->expect_character(src, ')');

                                u64 cc_i;
                                for (cc_i = 0; cc_i < array_length(calling_conventions); cc_i += 1)
                                {
                                    String calling_convention_string = calling_conventions[cc_i];

                                    if (calling_convention_string.equal(candidate_cc))
                                    {
                                        calling_convention = static_cast<CallingConvention>(cc_i);
                                        break;
                                    }
                                }

                                if (cc_i == array_length(calling_conventions))
                                {
                                    fail();
                                }
                            } break;
                        default:
                            trap();
                    }

                    break;
                }
            }

            if (attribute_i == array_length(function_attributes))
            {
                fail();
            }

            parser->skip_space(src);

            u8 after_ch = src.pointer[parser->i];
            switch (after_ch)
            {
                case function_attribute_end: break;
                default: fail();
            }
        }

        parser->expect_character(src, function_attribute_end);

        parser->skip_space(src);
    }

    String name = parser->parse_and_check_identifier(src);
    if (!name.pointer | !name.length)
    {
        fail();
    }

    auto function_index = thread->functions.length;
    auto* function = thread->functions.add_one();
    auto node_id = thread->node_count;
    thread->node_count += 1;

    auto symbol_result = file->symbols.get_or_put(thread->arena, name, Node{
        .type = {},
        .uid = node_id,
        .id = Node::Id::SYMBOL_FUNCTION,
        .payload = {
            .symbol = &function->symbol,
        },
    });

    if (symbol_result.existing)
    {
        fail();
    }

    parser->skip_space(src);

    u64 has_global_attributes = src.pointer[parser->i] == symbol_attribute_start;
    parser->i += has_global_attributes;

    GlobalSymbolAttributes symbol_attributes = {};
    if (has_global_attributes)
    {
        u64 mask = 0;

        while (1)
        {
            parser->skip_space(src);

            if (src.pointer[parser->i] == symbol_attribute_end)
            {
                break;
            }

            String candidate_attribute = parser->parse_raw_identifier(src);
            parser->skip_space(src);
            switch (src.pointer[parser->i])
            {
                case symbol_attribute_end:
                    break;
                case end_of_argument:
                    parser->i += 1;
                    break;
                default:
                    fail();
            }

            u64 attribute_i;
            for (attribute_i = 0; attribute_i < array_length(global_symbol_attributes); attribute_i += 1)
            {
                String attribute_string = global_symbol_attributes[attribute_i];
                if (attribute_string.equal(candidate_attribute))
                {
                    if (mask & (1 << attribute_i))
                    {
                        fail();
                    }

                    mask |= 1 << attribute_i;

                    auto attribute = static_cast<GlobalSymbolAttribute>(attribute_i);

                    switch (attribute)
                    {
                    case GLOBAL_SYMBOL_ATTRIBUTE_EXPORT:
                        symbol_attributes.exported = 1;
                        break;
                    case GLOBAL_SYMBOL_ATTRIBUTE_EXTERN:
                        symbol_attributes.external = 1;
                        break;
                    default:
                        trap();
                    }
                    break;
                }
            }

            if (attribute_i == array_length(global_symbol_attributes))
            {
                fail();
            }
        }

        parser->expect_character(src, symbol_attribute_end);

        parser->skip_space(src);
    }

    if (symbol_attributes.exported & symbol_attributes.external)
    {
        fail();
    }

    parser->expect_character(src, function_argument_start);

    Array<DebugType*> original_argument_types = {};
    Array<String> argument_names = {};

    while (1)
    {
        parser->skip_space(src);

        if (src.pointer[parser->i] == function_argument_end)
        {
            break;
        }

        String argument_name = parser->parse_and_check_identifier(src);
        argument_names.append_one(argument_name);

        parser->skip_space(src);
        parser->expect_character(src, ':');
        parser->skip_space(src);

        DebugType* argument_type = analyze_type(parser, unit, src);
        original_argument_types.append_one(argument_type);

        parser->skip_space(src);

        switch (src[parser->i])
        {
            case function_argument_end:
                break;
            case end_of_argument:
                parser->i += 1;
            default:
                fail();
        }
    }

    parser->expect_character(src, function_argument_end);

    parser->skip_space(src);

    DebugType* original_return_type = analyze_type(parser, unit, src);

    parser->skip_space(src);

    AbiInfo return_type_abi = {};
    Array<AbiInfo> argument_type_abis = {};

    switch (calling_convention)
    {
    case CALLING_CONVENTION_C:
        {
            // First process the return type ABI
            {
                SystemVClassification return_type_classes = systemv_classify(original_return_type, 0);
                assert(return_type_classes.v[1] != SYSTEMV_CLASS_MEMORY | return_type_classes.v[0] == SYSTEMV_CLASS_MEMORY);
                assert(return_type_classes.v[1] != SYSTEMV_CLASS_SSEUP | return_type_classes.v[0] == SYSTEMV_CLASS_SSE);
                DebugType* low_part = 0;
                switch (return_type_classes.v[0])
                {
                    case SYSTEMV_CLASS_INTEGER:
                        {
                            DebugType* result_type = systemv_get_int_type_at_offset(original_return_type, 0, original_return_type, 0);
                            if (return_type_classes.v[1] == SYSTEMV_CLASS_NONE & original_return_type->get_bit_count() < 32)
                            {
                                trap();
                            }

                            low_part = result_type;
                        } break;
                    default:
                        trap();
                }
                assert(low_part);

                DebugType* high_part = 0;
                switch (return_type_classes.v[1])
                {
                    case SYSTEMV_CLASS_NONE:
                        break;
                    case SYSTEMV_CLASS_MEMORY:
                        trap();
                    case SYSTEMV_CLASS_INTEGER:
                        trap();
                    case SYSTEMV_CLASS_SSE:
                        trap();
                    case SYSTEMV_CLASS_SSEUP:
                        trap();
                }

                if (high_part)
                {
                    trap();
                }
                else
                {
                    // TODO:
                    u8 is_type = 1;
                    if (is_type)
                    {
                        if (low_part == original_return_type)
                        {
                            return_type_abi =
                            {
                                .payload = {
                                    .direct = low_part->lower(thread),
                                },
                                .kind = ABI_INFO_DIRECT,
                            };
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

            // Now process the ABI for argument types
            
            // u32 abi_argument_type_count = 0;
            {
                SystemVRegisterCount available_registers = {
                    .gp_registers = 6,
                    .sse_registers = 8,
                };
                
                available_registers.gp_registers -= return_type_abi.kind == ABI_INFO_INDIRECT;

                // TODO: return by reference
                u8 return_by_reference = 0;
                if (return_by_reference)
                {
                    trap();
                }

                for (u32 original_argument_index = 0; original_argument_index < original_argument_types.length; original_argument_index += 1)
                {
                    trap();
                }
            }
        } break;
    case CALLING_CONVENTION_CUSTOM:
        {
            return_type_abi = {
                .payload = {
                    .direct = original_return_type->lower(thread),
                },
                .kind = ABI_INFO_DIRECT,
            };

            for (DebugType* original_argument_type : original_argument_types.slice())
            {
                argument_type_abis.append_one({
                    .payload = {
                        .direct = original_argument_type->lower(thread),
                    },
                    .kind = AbiInfoKind::ABI_INFO_DIRECT,
                });
            }
        } break;
    case CALLING_CONVENTION_COUNT:
        trap();
        break;
    }

    switch (symbol_attributes.external)
    {
        case 0:
            {
                switch (return_type_abi.kind)
                {
                    case ABI_INFO_IGNORE: case ABI_INFO_DIRECT:
                        break;
                    case ABI_INFO_DIRECT_PAIR:
                        trap();
                    case ABI_INFO_DIRECT_COERCE:
                        trap();
                    case ABI_INFO_DIRECT_COERCE_INT:
                        trap();
                    case ABI_INFO_DIRECT_SPLIT_STRUCT_I32:
                        trap();
                    case ABI_INFO_EXPAND_COERCE:
                        trap();
                    case ABI_INFO_INDIRECT:
                        trap();
                    case ABI_INFO_EXPAND:
                        trap();
                }

                *function = {
                    .symbol = {
                        .name = name,
                        .id = Symbol::Id::function,
                        .linkage = symbol_attributes.external ? Symbol::Linkage::external : Symbol::Linkage::internal,
                    },
                    .root_node = 0,
                    .stop_node = 0,
                    .parameters = thread->arena->allocate_many<Node*>(argument_type_abis.length),
                    .prototype = {
                        .argument_type_abis = argument_type_abis.pointer,
                        .original_argument_types = original_argument_types.pointer,
                        .original_return_type = original_return_type,
                        .return_type_abi = return_type_abi,
                        .original_argument_count = original_argument_types.length,
                        .varags = 0,
                    },
                    .uid = function_index,
                    .parameter_count = (u16)argument_type_abis.length,
                };

                Array<Node::Type*> abi_argument_types = {};
                Array<Node::Type*> root_arg_types = {};
                root_arg_types.append_one(thread->common_types.live_control);

                for (u32 i = 0; i < argument_type_abis.length; i += 1)
                {
                    u16 start = abi_argument_types.length;
                    auto* abi_info = &argument_type_abis[i];

                    // TODO: figure out how to interact with the C ABI
                    switch (abi_info->kind)
                    {
                    case ABI_INFO_IGNORE:
                        trap();
                    case ABI_INFO_DIRECT:
                        {
                            auto node_type = abi_info->payload.direct;
                            abi_argument_types.append_one(node_type);
                        } break;
                    case ABI_INFO_DIRECT_PAIR:
                        trap();
                    case ABI_INFO_DIRECT_COERCE:
                        trap();
                    case ABI_INFO_DIRECT_COERCE_INT:
                        trap();
                    case ABI_INFO_DIRECT_SPLIT_STRUCT_I32:
                        trap();
                    case ABI_INFO_EXPAND_COERCE:
                        trap();
                    case ABI_INFO_INDIRECT:
                        trap();
                    case ABI_INFO_EXPAND:
                        trap();
                    }

                    u16 end = abi_argument_types.length;

                    abi_info->indices[0] = start;
                    abi_info->indices[1] = end;
                }

                root_arg_types.append(abi_argument_types.slice());

                auto* root_type = thread->types.append_one({
                    .payload = {
                        .multi = {
                            .types = root_arg_types.slice(),
                        },
                    },
                    .id = NodeType::Id::MULTIVALUE,
                })->intern_type(thread);

                function->root_node = Node::add(thread, {
                    .inputs = {},
                    .id = Node::Id::ROOT,
                });
                function->root_node->type = root_type,
                function->root_node->payload.root.args = root_type;
                function->root_node->payload.root.function = function;
                function->root_node->peephole(thread, function);

                function->stop_node = Node::add(thread, {
                    .inputs = {},
                    .id = Node::Id::STOP,
                });

                Analyzer analyzer = {
                    .function = function,
                    .scope = create_scope(thread),
                    .file = file,
                };
                push_scope(&analyzer);
                auto control_name = strlit("$control");
                s32 next_index = 0;
                Node* control_node = function->root_node->project(thread, function->root_node, next_index, control_name)->peephole(thread, function);
                next_index += 1;
                define_variable(&analyzer, thread, control_name, control_node);
                // assert(abi_argument_type_count == 0);
                // TODO: reserve memory for them

                assert(argument_type_abis.length == argument_type_abis.length);
                assert(argument_names.length == argument_type_abis.length);

                for (u32 i = 0; i < argument_type_abis.length; i += 1)
                {
                    auto* abi_info = &argument_type_abis[i];
                    auto argument_name = argument_names[i];

                    // TODO: figure out how to interact with the C ABI
                    switch (abi_info->kind)
                    {
                    case ABI_INFO_IGNORE:
                        trap();
                    case ABI_INFO_DIRECT:
                        {
                            auto* argument_node = function->root_node->project(thread, function->root_node, next_index, argument_name)->peephole(thread, function);
                            define_variable(&analyzer, thread, argument_name, argument_node);
                            next_index += 1;
                        } break;
                    case ABI_INFO_DIRECT_PAIR:
                        trap();
                    case ABI_INFO_DIRECT_COERCE:
                        trap();
                    case ABI_INFO_DIRECT_COERCE_INT:
                        trap();
                    case ABI_INFO_DIRECT_SPLIT_STRUCT_I32:
                        trap();
                    case ABI_INFO_EXPAND_COERCE:
                        trap();
                    case ABI_INFO_INDIRECT:
                        trap();
                    case ABI_INFO_EXPAND:
                        trap();
                    }
                }

                analyze_local_block(&analyzer, parser, unit, thread, src);

                pop_scope(&analyzer, thread);

                function->stop_node->peephole(thread, function);

                return function;
            } break;
        case 1:
            trap();
        default:
            trap();
    }
}

fn void unit_file_analyze(Thread* thread, Unit* unit, File* file)
{
    compiler_file_read(thread->arena, file);

    Parser parser = {};
    String src = file->source_code;

    while (1) 
    {
        parser.skip_space(src);

        if (parser.i >= src.length)
        {
            break;
        }

        // u32 line = get_line(&parser);
        // u32 column = get_column(&parser);
        u64 declaration_start_index = parser.i;
        u8 declaration_start_ch = src.pointer[declaration_start_index];

        switch (declaration_start_ch)
        {
            case '>':
                trap();
                break;
            case 'f':
                if (get_next_ch_safe(src, declaration_start_index) == 'n')
                {
                    auto* function = analyze_function(&parser, thread, unit, file);
                    function->iterate(thread);
                }
                else
                {
                    fail();
                }
                break;
            default:
                fail();
        }
    }
}

method void Thread::init()
{
    auto* live_control = types.append_one({
        .id = NodeType::Id::LIVE_CONTROL,
    })->intern_type(this);
    auto* dead_control = types.append_one({
        .id = NodeType::Id::DEAD_CONTROL,
    })->intern_type(this);

    auto if_both_types = arena->allocate_slice<NodeType*>(2);
    if_both_types[0] = live_control;
    if_both_types[1] = live_control;

    auto if_neither_types = arena->allocate_slice<NodeType*>(2);
    if_neither_types[0] = dead_control;
    if_neither_types[1] = dead_control;

    auto if_true_types = arena->allocate_slice<NodeType*>(2);
    if_true_types[0] = live_control;
    if_true_types[1] = dead_control;

    auto if_false_types = arena->allocate_slice<NodeType*>(2);
    if_false_types[0] = dead_control;
    if_false_types[1] = live_control;

    common_types = {
        .bottom = types.append_one({
                .id = NodeType::Id::BOTTOM,
                })->intern_type(this),
        .top = types.append_one({
                .id = NodeType::Id::TOP,
                })->intern_type(this),
        .live_control = types.append_one({
                .id = NodeType::Id::LIVE_CONTROL,
                })->intern_type(this),
        .dead_control = types.append_one({
                .id = NodeType::Id::DEAD_CONTROL,
                })->intern_type(this),
        .integer_bot = types.append_one({
                .payload = {
                .constant = {
                .constant = 0,
                .is_constant = 0,
                },
                },
                .id = NodeType::Id::INTEGER,
                })->intern_type(this),
        .integer_top = types.append_one({
                .payload = {
                .constant = {
                .constant = 1,
                .is_constant = 0,
                },
                },
                .id = NodeType::Id::INTEGER,
                })->intern_type(this),
        .integer_zero = types.append_one({
                .payload = {
                .constant = {
                .constant = 0,
                .is_constant = 1,
                },
                },
                .id = NodeType::Id::INTEGER,
                })->intern_type(this),
        .if_both = types.append_one({
                .payload = {
                .multi = {
                .types = if_both_types, 
                },
                },
                .id = NodeType::Id::MULTIVALUE,
                })->intern_type(this),
        .if_neither = types.append_one({
                .payload = {
                .multi = {
                .types = if_neither_types, 
                },
                },
                .id = NodeType::Id::MULTIVALUE,
                })->intern_type(this),
        .if_true = types.append_one({
                .payload = {
                .multi = {
                .types = if_true_types, 
                },
                },
                .id = NodeType::Id::MULTIVALUE,
                })->intern_type(this),
        .if_false = types.append_one({
                .payload = {
                .multi = {
                .types = if_false_types, 
                },
                },
                .id = NodeType::Id::MULTIVALUE,
                })->intern_type(this),
    };

}

method void Thread::clear()
{
    functions.clear();
    nodes.clear();
    types.clear();
    interned_nodes.clear();
    interned_types.clear();
    worklist.clear();
    visited.clear();
    common_types = {};
    node_count = 0;
    peephole_iteration_count = 0;
    peephole_nop_iteration_count = 0;
    arena->reset();
}

fn Node* progress_on_list_callback(Thread* thread, Function* function, Node* node)
{
    Node* result = 0;

    if (!thread->worklist.on(node))
    {
        Node* new_node = node->peephole_optimize(thread, function);
        if (new_node)
        {
            result = new_node;
        }
    }

    return result;
}

method u8 Thread::progress_on_list(Function* function, Node* stop)
{
    mid_assert = 1;
    auto old_iteration_count = peephole_iteration_count;
    auto old_nop_iteration_count = peephole_nop_iteration_count;

    Node* changed = stop->walk(this, function, &progress_on_list_callback);
    peephole_iteration_count = old_iteration_count;
    peephole_nop_iteration_count = old_nop_iteration_count;
    mid_assert = 0;
    assert(changed == 0);
    return changed == 0;
}

method u8 WorkList::on(Node* node)
{
    return bitset.get(node->uid);
}

method Node* WorkList::push(Node* node)
{
    Node* result = 0;

    if (node)
    {
        result = node;
        u32 index = node->uid;

        if (!bitset.get(index))
        {
            bitset.set_assert_unset(index);
            nodes.append_one(node);
        }
    }

    return result;
}

global Instance instance;

global String test_file_paths[] = {
    strlit("tests/first/main.nat"),
    strlit("tests/constant_prop/main.nat"),
    strlit("tests/simple_variable_declaration/main.nat"),
    strlit("tests/function_call_args/main.nat"),
    strlit("tests/comparison/main.nat"),
    strlit("tests/if/main.nat"),
    strlit("tests/while/main.nat"),
    strlit("tests/break_continue/main.nat"),
};

#if LINK_LIBC
int main()
#else
extern "C" void entry_point()
#endif
{
    instance.arena = Arena::init(Arena::default_size, Arena::minimum_granularity, KB(4));
    Thread* thread = instance_add_thread(&instance);
    for (String test_file_path : test_file_paths)
    {
        thread->init();
        print(test_file_path);
        print(strlit("... "));
        Unit* unit = instance_add_unit(&instance);
        unit_initialize(unit);
        File* file = add_file(thread->arena, test_file_path);
        unit_file_analyze(thread, unit, file);
        print(strlit("[\x1b[32mOK\x1b[0m]\n"));
        file->symbols.clear();
        thread->clear();
    }

    print(strlit("\x1b[32mTESTS SUCCEEDED!\x1b[0m\n"));
}
