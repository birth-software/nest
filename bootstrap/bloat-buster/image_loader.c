#include <bloat-buster/image_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

EXPORT TextureMemory texture_load_from_file(Arena* arena, String path)
{
    auto file = file_read(arena, path);
    int width;
    int height;
    int channels;
    u8* buffer = stbi_load_from_memory(file.pointer, file.length, &width, &height, &channels, 0);

    return (TextureMemory) {
        .pointer = buffer,
        .width = width,
        .height = height,
        .channel_count = channels,
    };
}
