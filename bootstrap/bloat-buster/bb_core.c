#if BB_CI == 0
#include <bloat-buster/bb_core.h>

#include <std/virtual_buffer.h>
#include <std/graphics.h>
#include <std/render.h>
#include <std/shader_compilation.h>
#include <std/image_loader.h>
#include <std/font_provider.h>

STRUCT(Vertex)
{
    f32 x;
    f32 y;
    f32 uv_x;
    f32 uv_y;
    Vec4 color;
    u32 texture_index;
    u32 reserved[3];
};
decl_vb(Vertex);

fn TextureIndex white_texture_create(Arena* arena, Renderer* renderer)
{
    u32 white_texture_width = 1024;
    u32 white_texture_height = white_texture_width;
    auto* white_texture_buffer = arena_allocate(arena, u32, white_texture_width * white_texture_height);
    memset(white_texture_buffer, 0xff, white_texture_width * white_texture_height * sizeof(u32));

    auto white_texture = renderer_texture_create(renderer, (TextureMemory) {
        .pointer = white_texture_buffer,
        .width = white_texture_width,
        .height = white_texture_height,
        .depth = 1,
        .format = TEXTURE_FORMAT_R8G8B8A8_SRGB,
    });

    return white_texture;
}

fn void draw_string(RenderWindow* window, Vec4 color, String string, TextureAtlas texture_atlas, u32 texture_index, u32 x_offset, u32 y_offset)
{
    auto height = texture_atlas.ascent - texture_atlas.descent;
    for (u64 i = 0; i < string.length; i += 1)
    {
        auto ch = string.pointer[i];
        auto* character = &texture_atlas.characters[ch];
        auto pos_x = x_offset;
        auto pos_y = y_offset + character->y_offset + height; // Offset of the height to render the character from the bottom (y + height) up (y)
        auto uv_x = character->x;
        auto uv_y = character->y;
        auto uv_width = character->width;
        auto uv_height = character->height;

        Vertex vertices[] = {
            (Vertex) {
                .x = pos_x,
                .y = pos_y,
                .uv_x = (f32)uv_x,
                .uv_y = (f32)uv_y,
                .color = color,
                .texture_index = texture_index,
            },
            (Vertex) {
                .x = pos_x + character->width,
                .y = pos_y,
                .uv_x = (f32)(uv_x + uv_width),
                .uv_y = (f32)uv_y,
                .color = color,
                .texture_index = texture_index,
            },
            (Vertex) {
                .x = pos_x,
                .y = pos_y + character->height,
                .uv_x = (f32)uv_x,
                .uv_y = (f32)(uv_y + uv_height),
                .color = color,
                .texture_index = texture_index,
            },
            (Vertex) {
                .x = pos_x + character->width,
                .y = pos_y + character->height,
                .uv_x = (f32)(uv_x + uv_width),
                .uv_y = (f32)(uv_y + uv_height),
                .color = color,
                .texture_index = texture_index,
            },
        };

        auto vertex_offset = window_pipeline_add_vertices(window, BB_PIPELINE_RECT, (String)array_to_bytes(vertices), array_length(vertices));

        u32 indices[] = {
            vertex_offset + 0,
            vertex_offset + 1,
            vertex_offset + 2,
            vertex_offset + 1,
            vertex_offset + 3,
            vertex_offset + 2,
        };

        window_pipeline_add_indices(window, BB_PIPELINE_RECT, (Slice(u32))array_to_slice(indices));

        auto kerning = (texture_atlas.kerning_tables + ch * 256)[string.pointer[i + 1]];
        x_offset += character->advance + kerning;
    }
}

STRUCT(BBWindow)
{
    OSWindow os;
    RenderWindow* render;
};

STRUCT(BBGUIState)
{
    Arena* arena;
    BBWindow* first;
    BBWindow* last;
};
global_variable BBGUIState state;

void run_app()
{
    state.arena = arena_init(MB(512), MB(2), MB(2));

    u8 use_x11 = 1;
    os_graphics_init(use_x11);
    OSWindow os_window = os_window_create((OSWindowCreate) {
        .name = strlit("Bloat Buster"),
        .size = {
            .width = 1024,
            .height= 768,
        },
    });

    if (!os_window)
    {
        failed_execution();
    }

    Renderer* renderer = renderer_initialize(state.arena);
    RenderWindow* render_window = renderer_window_initialize(renderer, os_window);

    auto font_path = 
#ifdef _WIN32
        strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf");
#elif defined(__linux__)
        strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf");
#else
        strlit("WRONG_PATH");
#endif

    window_rect_texture_update_begin(render_window);

    auto white_texture = white_texture_create(state.arena, renderer);
    auto monospace_font = font_texture_atlas_create(state.arena, renderer, (TextureAtlasCreate) {
        .font_path = font_path,
        .text_height = 50,
    });
    auto proportional_font = monospace_font;
    // auto proportional_font = font_texture_atlas_create(state.arena, renderer, (TextureAtlasCreate) {
    //     .font_path = font_path,
    //     .text_height = 36,
    // });
    window_queue_rect_texture_update(render_window, RECT_TEXTURE_SLOT_WHITE, white_texture);
    renderer_queue_font_update(renderer, render_window, RENDER_FONT_TYPE_MONOSPACE, monospace_font);
    renderer_queue_font_update(renderer, render_window, RENDER_FONT_TYPE_PROPORTIONAL, proportional_font);

    window_rect_texture_update_end(renderer, render_window);

    while (!os_window_should_close(os_window))
    {
        os_poll_events();

        auto mouse_position = os_window_cursor_position_get(os_window);
        // print("Mouse position: ({f64}, {f64})\n", mouse_position.x, mouse_position.y);

        renderer_window_frame_begin(renderer, render_window);

        // u32 box_width = 10;
        // u32 box_height = 10;
        // auto box_color = Color4(1, 0, 0, 1);
        // 
        // Vertex box_vertices[] = {
        //     {
        //         .x = mouse_position.x,
        //         .y = mouse_position.y,
        //         .color = box_color,
        //     },
        //     {
        //         .x = mouse_position.x + box_width,
        //         .y = mouse_position.y,
        //         .color = box_color,
        //     },
        //     {
        //         .x = mouse_position.x,
        //         .y = mouse_position.y + box_height,
        //         .color = box_color,
        //     },
        //     {
        //         .x = mouse_position.x + box_width,
        //         .y = mouse_position.y + box_height,
        //         .color = box_color,
        //     },
        // };
        //
        // auto vertex_offset = window_pipeline_add_vertices(render_window, BB_PIPELINE_RECT, (String)array_to_bytes(box_vertices), array_length(box_vertices));
        //
        // u32 box_indices[] = {
        //     vertex_offset + 0, vertex_offset + 1, vertex_offset + 2,
        //     vertex_offset + 1, vertex_offset + 3, vertex_offset + 2,
        // };
        //
        // window_pipeline_add_indices(render_window, BB_PIPELINE_RECT, (Slice(u32))array_to_slice(box_indices));
        draw_string(render_window, Color4(0, 0, 0, 1), strlit("abcdefghijklmnopqrstuvwxyz!"), monospace_font, RECT_TEXTURE_SLOT_MONOSPACE_FONT, 100, 100);

        renderer_window_frame_end(renderer, render_window);
    }

    // TODO: deinitialization
}

#endif
