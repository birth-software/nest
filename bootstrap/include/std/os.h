#pragma once

#include <std/base.h>

typedef enum TimeUnit
{
    TIME_UNIT_NANOSECONDS,
    TIME_UNIT_MICROSECONDS,
    TIME_UNIT_MILLISECONDS,
    TIME_UNIT_SECONDS,
} TimeUnit;


STRUCT(Timestamp)
{
    u128 value;
};

STRUCT(OSFileOpenFlags)
{
    u32 truncate:1;
    u32 executable:1;
    u32 write:1;
    u32 read:1;
    u32 create:1;
    u32 directory:1;
};

STRUCT(OSFilePermissions)
{
    u8 readable:1;
    u8 writable:1;
    u8 executable:1;
};

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

STRUCT(Arena)
{
    u64 reserved_size;
    u64 position;
    u64 os_position;
    u64 granularity;
    u8 reserved[4 * 8];
};

STRUCT(FileWriteOptions)
{
    String path;
    String content;
    u8 executable;
};

#if __APPLE__
    const global_variable u64 page_size = KB(16);
#else
    const global_variable u64 page_size = KB(4);
#endif

global_variable u64 minimum_granularity = page_size;
// global_variable u64 middle_granularity = MB(2);
global_variable u64 default_size = GB(4);

EXPORT void print(const char* format, ...);
EXPORT void run_command(Arena* arena, CStringSlice arguments, char* envp[]);
EXPORT String file_read(Arena* arena, String path);
EXPORT void file_write(FileWriteOptions options);

EXPORT String path_dir(String string);
EXPORT String path_base(String string);
EXPORT String path_no_extension(String string);


EXPORT Arena* arena_init(u64 reserved_size, u64 granularity, u64 initial_size);
EXPORT Arena* arena_init_default(u64 initial_size);
EXPORT void arena_clear(Arena* arena);
EXPORT String arena_join_string(Arena* arena, Slice(String) pieces);
EXPORT u8* arena_allocate_bytes(Arena* arena, u64 size, u64 alignment);
EXPORT void arena_reset(Arena* arena);

#define arena_allocate(arena, T, count) (T*)(arena_allocate_bytes(arena, sizeof(T) * count, alignof(T)))
#define arena_allocate_slice(arena, T, count) (Slice(T)){ .pointer = arena_allocate(arena, T, count), .length = count }

EXPORT u8* os_reserve(u64 base, u64 size, OSReserveProtectionFlags protection, OSReserveMapFlags map);
EXPORT void os_commit(void* address, u64 size);

EXPORT u8 os_file_descriptor_is_valid(FileDescriptor fd);
EXPORT FileDescriptor os_file_open(String path, OSFileOpenFlags flags, OSFilePermissions permissions);
EXPORT void os_file_close(FileDescriptor fd);
EXPORT u64 os_file_get_size(FileDescriptor fd);
EXPORT void os_file_write(FileDescriptor fd, String content);
EXPORT FileDescriptor os_stdout_get();
EXPORT void os_directory_make(String path);

EXPORT void calibrate_cpu_timer();

EXPORT void print_string(String string);

EXPORT Timestamp os_timestamp();
EXPORT f64 os_resolve_timestamps(Timestamp start, Timestamp end, TimeUnit time_unit);
EXPORT u8 os_is_being_debugged();

#if _WIN32
typedef void* HANDLE;
EXPORT HANDLE os_windows_get_module_handle();
#endif
