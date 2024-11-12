#include <bloat-buster/font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

TextureAtlas font_create_texture_atlas(Arena* arena, String font_path)
{
    auto font_file = file_read(arena, font_path);
    stbtt_fontinfo font_info;
    if (!stbtt_InitFont(&font_info, font_file.pointer, 0))
    {
        failed_execution();
    }

    int atlas_width = 1024;
    int atlas_height = 1024;
    int atlas_size = atlas_width * atlas_height;

    auto* atlas = arena_allocate(arena, u8, atlas_size);

    int char_width = 64; // Width of each character’s cell in the atlas
    int char_height = 64; // Height of each character’s cell in the atlas
    int x = 0;
    int y = 0; // Starting position in the atlas
    float scale_x = 0.0f;
    float scale_y = stbtt_ScaleForPixelHeight(&font_info, char_height);

    for (u32 i = 0; i <= 256; ++i)
    {
        int width;
        int height;
        int x_offset;
        int y_offset;

        auto ch = (u8)i;
        u8* bitmap = stbtt_GetCodepointBitmap(&font_info, scale_x, scale_y, ch, &width, &height, &x_offset, &y_offset);
        if (bitmap)
        {
            for (int j = 0; j < height; ++j)
            {
                for (int i = 0; i < width; ++i)
                {
                    atlas[(y + j) * atlas_width + (x + i)] = bitmap[j * width + i];
                }
            }

            stbtt_FreeBitmap(bitmap, nullptr);
        }

        x += char_width;

        if (x + char_width > atlas_width)
        {
            x = 0;
            y += char_height;
        }
    }

    return (TextureAtlas) {
        .pointer = atlas,
        .width = atlas_width,
        .height = atlas_height,
        .char_height = char_height,
    };
}
