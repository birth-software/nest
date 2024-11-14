#include <bloat-buster/font.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

TextureAtlas font_create_texture_atlas(Arena* arena, String font_path)
{

    auto font_file = file_read(arena, font_path);
    stbtt_fontinfo font_info;
    if (!stbtt_InitFont(&font_info, font_file.pointer, stbtt_GetFontOffsetForIndex(font_file.pointer, 0)))
    {
        failed_execution();
    }

    TextureAtlas result = {};
    result.character_height = 64;
    result.character_width = result.character_height;
    result.height = 1024;
    result.width = result.height;
    auto atlas_size = result.width * result.height;
    result.pointer = arena_allocate(arena, u8, atlas_size);

    float scale_x = 0.0f;
    float scale_y = stbtt_ScaleForPixelHeight(&font_info, result.character_height);
    
    // Starting position in the atlas
    u32 x = 0;
    u32 y = 0; 
    u32 char_count = 256;

    for (u32 i = 0; i < char_count; ++i)
    {
        u32 width;
        u32 height;
        u32 x_offset;
        u32 y_offset;

        auto ch = (u8)i;
        u8* bitmap = stbtt_GetCodepointBitmap(&font_info, scale_x, scale_y, ch, (int*)&width, (int*)&height, (int*)&x_offset, (int*)&y_offset);
        if (bitmap)
        {
            for (u32 j = 0; j < height; ++j)
            {
                for (u32 i = 0; i < width; ++i)
                {
                    auto atlas_index = (y + j) * result.width + (x + i);
                    auto bitmap_index = (height - j - 1) * width + i;
                    assert(atlas_index < atlas_size);
                    result.pointer[atlas_index] = bitmap[bitmap_index];
                }
            }

            stbtt_FreeBitmap(bitmap, 0);
        }

        x += result.character_width;

        if (x + result.character_width > result.width)
        {
            x = 0;
            y += result.character_height;
        }
    }

    return result;
}
