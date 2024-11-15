#include <std/base.h>
#include <std/os.h>

STRUCT(FontCharacter)
{
    u32 advance;
    u32 left_bearing;
    u32 x;
    u32 y;
    u32 width;
    u32 height;
    s32 x_offset;
    s32 y_offset;
};

STRUCT(TextureAtlas)
{
    u8* pointer;
    FontCharacter* characters;
    s32* kerning_tables;
    u32 width;
    u32 height;
    s32 ascent;
    s32 descent;
    s32 line_gap;
};

STRUCT(TextureAtlasCreate)
{
    String font_path;
    u32 text_height;
};

EXPORT TextureAtlas font_create_texture_atlas(Arena* arena, TextureAtlasCreate create);
