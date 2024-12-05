#if BB_CI == 0
#include <bloat-buster/bb_core.h>

#include <std/virtual_buffer.h>
#include <std/graphics.h>
#include <std/render.h>
#include <std/shader_compilation.h>
#include <std/image_loader.h>
#include <std/font_provider.h>
#include <std/ui_core.h>
#include <std/ui_builder.h>

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

STRUCT(BBPanel)
{
    BBPanel* first;
    BBPanel* last;
    BBPanel* next;
    BBPanel* previous;
    BBPanel* parent;
    f32 parent_percentage;
    Axis2 split_axis;
};

STRUCT(BBWindow)
{
    OSWindow os;
    RenderWindow* render;
    BBWindow* previous;
    BBWindow* next;
    BBPanel* root_panel;
    UI_State* ui;
};

STRUCT(BBGUIState)
{
    Arena* arena;
    Timestamp last_frame_timestamp;
    BBWindow* first_window;
    BBWindow* last_window;
    Renderer* renderer;
    // TODO: should this not be thread local?
    OSEventQueue event_queue;
};
global_variable BBGUIState state;

fn void app_update()
{
    auto frame_end = os_timestamp();
    os_poll_events(&state.event_queue);
    auto frame_ms = os_resolve_timestamps(state.last_frame_timestamp, frame_end, TIME_UNIT_MILLISECONDS);
    state.last_frame_timestamp = frame_end;

    Renderer* renderer = state.renderer;

    BBWindow* window = state.first_window;
    while (likely(window))
    {
        auto* previous = window->previous;
        auto* next = window->next;

        auto* render_window = window->render;
        renderer_window_frame_begin(renderer, render_window);

        ui_state_select(window->ui);

        if (likely(ui_build_begin(window->os, frame_ms, &state.event_queue)))
        {
            {
                if (unlikely(ui_button(strlit("Hello world\n")).clicked_left))
                {
                    print("Clicked on hello world\n");
                }
            }

            ui_build_end();

            ui_draw();

            renderer_window_frame_end(renderer, render_window);
        }
        else
        {
            if (previous)
            {
                previous->next = next;
            }

            if (next)
            {
                next->previous = previous;
            }

            if (state.first_window == window)
            {
                state.first_window = next;
            }

            if (state.last_window == window)
            {
                state.last_window = previous;
            }
        }

        window = next;
    }
}

fn void window_refresh_callback(OSWindow window, void* context)
{
    unused(window);
    unused(context);
    app_update();
}

void run_app()
{
    state.arena = arena_init(MB(512), MB(2), MB(2));

    os_graphics_init((OSGraphicsInitializationOptions) {
        .should_use_x11 = 1,
    });
    state.renderer = renderer_initialize(state.arena);

    state.first_window = state.last_window = arena_allocate(state.arena, BBWindow, 1);
    state.first_window->os = os_window_create((OSWindowCreate) {
        .name = strlit("Bloat Buster"),
        .size = {
            .width = 1024,
            .height= 768,
        },
        .refresh_callback = &window_refresh_callback,
    });

    if (!state.first_window->os)
    {
        failed_execution();
    }

    state.first_window->ui = arena_allocate(state.arena, UI_State, 1);
    state.first_window->render = renderer_window_initialize(state.renderer, state.first_window->os);
    state.first_window->root_panel = arena_allocate(state.arena, BBPanel, 1);
    state.first_window->root_panel->parent_percentage = 1.0f;
    state.first_window->root_panel->split_axis = AXIS2_X;

    auto font_path = 
#ifdef _WIN32
        strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf");
#elif defined(__linux__)
        strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf");
#elif defined(__APPLE__)
strlit("/Users/david/Library/Fonts/FiraSans-Regular.ttf");
#else
        strlit("WRONG_PATH");
#endif

    window_rect_texture_update_begin(state.first_window->render);

    auto white_texture = white_texture_create(state.arena, state.renderer);
    auto monospace_font = font_texture_atlas_create(state.arena, state.renderer, (TextureAtlasCreate) {
        .font_path = font_path,
        .text_height = 50,
    });
    auto proportional_font = monospace_font;

    window_queue_rect_texture_update(state.first_window->render, RECT_TEXTURE_SLOT_WHITE, white_texture);
    renderer_queue_font_update(state.renderer, state.first_window->render, RENDER_FONT_TYPE_MONOSPACE, monospace_font);
    renderer_queue_font_update(state.renderer, state.first_window->render, RENDER_FONT_TYPE_PROPORTIONAL, proportional_font);

    window_rect_texture_update_end(state.renderer, state.first_window->render);

    state.last_frame_timestamp = os_timestamp();

    while (state.first_window)
    {
        app_update();
    }

    // TODO: deinitialization
}

#endif
