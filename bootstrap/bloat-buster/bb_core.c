#if BB_CI == 0
#include <bloat-buster/bb_core.h>

#include <std/virtual_buffer.h>
#include <std/graphics.h>
#include <std/render.h>
#include <std/shader_compilation.h>
#include <std/image_loader.h>
#include <std/font_provider.h>

STRUCT(Vec4)
{
    f32 v[4];
}__attribute__((aligned(16)));

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

fn void draw_string(Renderer* renderer, VirtualBuffer(Vertex)* vertices, VirtualBuffer(u32)* indices, u32 width, u32 height, Vec4 color, String string, TextureAtlas texture_atlas, u32 texture_index)
{
    u32 index_offset = 0;
    auto x_offset = width / 2;
    auto y_offset = height / 2;

    for (u64 i = 0; i < string.length; i += 1, index_offset += 4)
    {
        auto ch = string.pointer[i];
        auto* character = &texture_atlas.characters[ch];
        auto pos_x = x_offset;
        auto pos_y = y_offset - (character->height + character->y_offset);
        auto uv_x = character->x;
        auto uv_y = character->y;
        auto uv_width = character->width;
        auto uv_height = character->height;

        *vb_add(vertices, 1) = (Vertex) {
            .x = pos_x,
            .y = pos_y,
            .uv_x = (f32)uv_x,
            .uv_y = (f32)uv_y,
            .color = color,
            .texture_index = texture_index,
        };
        *vb_add(vertices, 1) = (Vertex) {
            .x = pos_x + character->width,
            .y = pos_y,
            .uv_x = (f32)(uv_x + uv_width),
            .uv_y = (f32)uv_y,
            .color = color,
            .texture_index = texture_index,
        };
        *vb_add(vertices, 1) = (Vertex) {
            .x = pos_x,
            .y = pos_y + character->height,
            .uv_x = (f32)uv_x,
            .uv_y = (f32)(uv_y + uv_height),
            .color = color,
            .texture_index = texture_index,
        };
        *vb_add(vertices, 1) = (Vertex) {
            .x = pos_x + character->width,
            .y = pos_y + character->height,
            .uv_x = (f32)(uv_x + uv_width),
            .uv_y = (f32)(uv_y + uv_height),
            .color = color,
            .texture_index = texture_index,
        };

        *vb_add(indices, 1) = index_offset + 0;
        *vb_add(indices, 1) = index_offset + 1;
        *vb_add(indices, 1) = index_offset + 2;
        *vb_add(indices, 1) = index_offset + 1;
        *vb_add(indices, 1) = index_offset + 3;
        *vb_add(indices, 1) = index_offset + 2;

        auto kerning = (texture_atlas.kerning_tables + ch * 256)[string.pointer[i + 1]];
        // print("Advance: {u32}. Kerning: {s32}\n", character->advance, kerning);
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
strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf")
#elif defined(__linux__)
strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf")
#else
#endif
;

    window_rect_texture_update_begin(render_window);
    {
        auto white_texture = white_texture_create(state.arena, renderer);
        auto monospace_font = font_texture_atlas_create(state.arena, renderer, (TextureAtlasCreate) {
                .font_path = font_path,
                .text_height = 180,
                });
        auto proportional_font = font_texture_atlas_create(state.arena, renderer, (TextureAtlasCreate) {
                .font_path = font_path,
                .text_height = 180,
                });
        window_queue_rect_texture_update(render_window, RECT_TEXTURE_SLOT_WHITE, white_texture);
        renderer_queue_font_update(renderer, render_window, RENDER_FONT_TYPE_MONOSPACE, monospace_font);
        renderer_queue_font_update(renderer, render_window, RENDER_FONT_TYPE_PROPORTIONAL, proportional_font);
    }
    window_rect_texture_update_end(renderer, render_window);

    // Vec4 color = {1, 1, 1, 1};
    // static_assert(sizeof(color) == 4 * sizeof(float));
    //
    // u32 texture_index = 1;
    // // draw_string(renderer, &vertices, &indices, window_size.width, window_size.height, color, strlit("He"), texture_atlas, texture_index);
    //
    //
    // vb_copy_array(&vertices, box_vertices);
    // vb_copy_array(&indices, box_indices);
    //
    // auto vertex_buffer_size = sizeof(*vertices.pointer) * vertices.length;
    // auto index_buffer_size = sizeof(*indices.pointer) * indices.length;
    //
    // auto vertex_buffer = renderer_buffer_create(renderer, vertex_buffer_size, BUFFER_TYPE_VERTEX);
    // auto vertex_buffer_device_address = buffer_address(vertex_buffer);
    // auto index_buffer = renderer_buffer_create(renderer, index_buffer_size, BUFFER_TYPE_INDEX);
    // auto staging_buffer = renderer_buffer_create(renderer, vertex_buffer_size + index_buffer_size, BUFFER_TYPE_STAGING);
    //
    // renderer_copy_to_host(staging_buffer, (Slice(HostBufferCopy)) array_to_slice(((HostBufferCopy[]) {
    //     {
    //         .destination_offset = 0,
    //         .source = {
    //             .pointer = (u8*)vertices.pointer,
    //             .length = vertex_buffer_size,
    //         },
    //     },
    //     {
    //         .destination_offset = vertex_buffer_size,
    //         .source = {
    //             .pointer = (u8*)indices.pointer,
    //             .length = index_buffer_size,
    //         },
    //     },
    // })));
    //
    // renderer_copy_to_local(renderer, (Slice(LocalBufferCopy)) array_to_slice(((LocalBufferCopy[]) {
    //     {
    //         .destination = vertex_buffer,
    //         .source = staging_buffer,
    //         .regions = array_to_slice(((LocalBufferCopyRegion[]) {
    //             {
    //                 .source_offset = 0,
    //                 .destination_offset = 0,
    //                 .size = vertex_buffer_size,
    //             },
    //         })),
    //     },
    //     {
    //         .destination = index_buffer,
    //         .source = staging_buffer,
    //         .regions = array_to_slice(((LocalBufferCopyRegion[]) {
    //             {
    //                 .source_offset = vertex_buffer_size,
    //                 .destination_offset = 0,
    //                 .size = index_buffer_size,
    //             },
    //         })),
    //     },
    // })));

    while (!os_window_should_close(os_window))
    {
        os_poll_events();

        auto mouse_position = os_window_cursor_position_get(os_window);
        // print("Mouse position: ({f64}, {f64})\n", mouse_position.x, mouse_position.y);

        renderer_window_frame_begin(renderer, render_window);
        u32 box_x = 200;
        u32 box_y = 200;
        u32 box_width = 100;
        u32 box_height = 100;

        Vec4 box_color = { 1, 0, 0, 1 };

        Vertex box_vertices[] = {
            {
                .x = box_x,
                .y = box_y,
                .color = box_color,
            },
            {
                .x = box_x + box_width,
                .y = box_y,
                .color = box_color,
            },
            {
                .x = box_x,
                .y = box_y + box_height,
                .color = box_color,
            },
            {
                .x = box_x + box_width,
                .y = box_y + box_height,
                .color = box_color,
            },
        };

        u32 box_indices[] = {
            0, 1, 2,
            1, 3, 2,
        };

        window_pipeline_add_vertices(render_window, BB_PIPELINE_RECT, (String)array_to_bytes(box_vertices));
        window_pipeline_add_indices(render_window, BB_PIPELINE_RECT, (Slice(u32))array_to_slice(box_indices));

        renderer_window_frame_end(renderer, render_window);
        // window_size = os_window_size_get(os_window);
        //
        // window_command_begin(render_window);
        //
        // // window_bind_pipeline(render_window, pipeline_index);
        // // window_bind_pipeline_descriptor_sets(render_window, pipeline_index);
        // // window_bind_index_buffer(render_window, index_buffer, 0, INDEX_TYPE_U32);
        // // GPUDrawPushConstants push_constants = {
        // //     .vertex_buffer = vertex_buffer_device_address,
        // //     .width = (f32)window_size.width,
        // //     .height = (f32)window_size.height,
        // // };
        // // window_push_constants(render_window, pipeline_index, (SliceP(void)) array_to_slice(((void*[]) {&push_constants})));
        //
        // window_render_begin(render_window);
        // {
        //     // window_draw_indexed(render_window, indices.length, 1, 0, 0, 0);
        // }
        // window_render_end(render_window);
        //
        // window_command_end(render_window);
        //
        // renderer_window_frame_end(renderer, render_window);
    }

    // TODO: deinitialization
}

#endif
