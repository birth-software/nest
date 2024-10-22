#include <std/string.h>

s32 string_first_ch(String string, u8 ch)
{
    s32 result = -1;
    for (u64 i = 0; i < string.length; i += 1)
    {
        if (string.pointer[i] == ch)
        {
            result = i;
            break;
        }
    }

    return result;
}

s64 string_last_ch(String string, u8 ch)
{
    s64 result = -1;
    u64 i = string.length;
    while (i > 0)
    {
        i -= 1;
        if (string.pointer[i] == ch)
        {
            result = cast_to(s64, u64, i);
            break;
        }
    }

    return result;
}

u8 string_starts_with(String string, String start)
{
    u8 result = 0;

    if (likely(start.length <= string.length))
    {
        if (unlikely(start.pointer == string.pointer))
        {
            result = 1;
        }
        else
        {
            u64 i;
            for (i = 0; i < start.length; i += 1)
            {
                auto start_ch = start.pointer[i];
                auto string_ch = string.pointer[i];
                if (unlikely(string_ch != start_ch))
                {
                    break;
                }
            }

            result = i == start.length;
        }
    }

    return result;
}
