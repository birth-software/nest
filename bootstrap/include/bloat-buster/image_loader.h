#include <std/base.h>
#include <std/os.h>

STRUCT(TextureMemory)
{
    u8* pointer;
    u32 width;
    u32 height;
    u32 channel_count;
    u32 padding[1];
};

EXPORT TextureMemory texture_load_from_file(Arena* arena, String path);
