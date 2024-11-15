#include <std/base.h>
#include <std/os.h>

STRUCT(FontCharacter)
{
    u32 advance;
    u32 left_bearing;
};

STRUCT(TextureAtlas)
{
    u8* pointer;
    FontCharacter* characters;
    u32 width;
    u32 height;
    u32 character_width;
    u32 character_height;
    f32 scale;
};

EXPORT TextureAtlas font_create_texture_atlas(Arena* arena, String font_path);
