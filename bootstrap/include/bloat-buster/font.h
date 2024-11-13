#include <std/base.h>
#include <std/os.h>

STRUCT(FontCharacter)
{
    int advance;
};

STRUCT(TextureAtlas)
{
    u8* pointer;
    FontCharacter* characters;
    u32 width;
    u32 height;
    u32 character_width;
    u32 char_height;
};

EXPORT TextureAtlas font_create_texture_atlas(Arena* arena, String font_path);
