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
};

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

STRUCT(GPUDrawPushConstants)
{
    u64 vertex_buffer;
    f32 width;
    f32 height;
};

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

        print("UV x: {u32}. UV y: {u32}\n", uv_x, uv_y);

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

void run_app(Arena* arena)
{
    u8 use_x11 = 1;
    graphics_init(use_x11);
    GraphicsWindow* window = graphics_window_create((GraphicsWindowCreate) {
        .name = strlit("Bloat Buster"),
        .size = {
            .width = 1024,
            .height= 768,
        },
    });

    if (!window)
    {
        failed_execution();
    }

    auto initial_window_size = graphics_window_size_get(window);

    Renderer* renderer = renderer_initialize();
    RenderWindow* render_window = renderer_window_initialize(renderer, window);

    String shader_source_paths[] = {
        strlit("bootstrap/shaders/font.vert"),
        strlit("bootstrap/shaders/font.frag"),
    };
    PipelineLayoutCreate pipeline_layouts[] = {
        (PipelineLayoutCreate) {
            .push_constant_ranges = array_to_slice(((PushConstantRange[]){
                (PushConstantRange) {
                    .offset = 0,
                    .size = sizeof(GPUDrawPushConstants),
                    .stage = SHADER_STAGE_VERTEX,
                },
            })),
            .descriptor_set_layouts = array_to_slice(((DescriptorSetLayout[]){
                (DescriptorSetLayout) {
                    .bindings = array_to_slice(((DescriptorSetLayoutBinding[]) {
                        {
                            .binding = 0,
                            .type = DESCRIPTOR_TYPE_IMAGE_PLUS_SAMPLER,
                            .stage = SHADER_STAGE_FRAGMENT,
                            .count = 2,
                        },
                    })),
                },
            })),
        },
    };
    PipelineCreate pipeline_create[] = {
        (PipelineCreate) {
            .shader_source_indices = array_to_slice(((u16[]){0, 1})),
            .layout_index = 0,
        },
    };
    auto pipeline_index = renderer_graphics_pipelines_create(renderer, arena, (GraphicsPipelinesCreate){
        .layouts = array_to_slice(pipeline_layouts),
        .pipelines = array_to_slice(pipeline_create),
        .shader_sources = array_to_slice(shader_source_paths),
    });

    u32 white_texture_width = 1024;
    u32 white_texture_height = white_texture_width;
    auto* white_texture_buffer = arena_allocate(arena, u32, white_texture_width * white_texture_height / sizeof(u32));
    memset(white_texture_buffer, 0xff, white_texture_width * white_texture_height);

    auto white_texture = renderer_texture_create(renderer, (TextureMemory) {
        .pointer = white_texture_buffer,
        .width = white_texture_width,
        .height = white_texture_height,
        .depth = 1,
        .format = R8G8B8A8_SRGB,
    });

    auto font_path = 
#ifdef _WIN32
strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf")
#elif defined(__linux__)
strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf")
#else
#endif
;
    auto texture_atlas = font_create_texture_atlas(arena, (TextureAtlasCreate) {
        .font_path = font_path,
        .text_height = 72,
    });
    auto texture_atlas_image = renderer_texture_create(renderer, (TextureMemory) {
        .pointer = texture_atlas.pointer,
        .width = texture_atlas.width,
        .height = texture_atlas.height,
        .depth = 1,
        .format = R8G8B8A8_SRGB,
    });
    DescriptorSetUpdate descriptor_set_updates[] = {
        {
            .set = { .value = 0 },
            .type = DESCRIPTOR_TYPE_IMAGE_PLUS_SAMPLER,
            .binding = 0,
            .descriptor_count = 2,
            .textures = {
                [0] = white_texture,
                [1] = texture_atlas_image,
            },
        },
    };

    renderer_update_pipeline_resources(renderer, pipeline_index, (Slice(DescriptorSetUpdate)) array_to_slice(descriptor_set_updates));

    VirtualBuffer(Vertex) vertices = {};
    VirtualBuffer(u32) indices = {};

    Vec4 color = {1, 1, 1, 1};
    static_assert(sizeof(color) == 4 * sizeof(float));

    u32 texture_index = 1;
    draw_string(renderer, &vertices, &indices, initial_window_size.width, initial_window_size.height, color, strlit("He"), texture_atlas, texture_index);

    auto vertex_buffer_size = sizeof(*vertices.pointer) * vertices.length;
    auto index_buffer_size = sizeof(*indices.pointer) * indices.length;

    auto vertex_buffer = renderer_buffer_create(renderer, vertex_buffer_size, BUFFER_TYPE_VERTEX);
    auto vertex_buffer_device_address = buffer_address(vertex_buffer);
    auto index_buffer = renderer_buffer_create(renderer, index_buffer_size, BUFFER_TYPE_INDEX);
    auto staging_buffer = renderer_buffer_create(renderer, vertex_buffer_size + index_buffer_size, BUFFER_TYPE_STAGING);

    renderer_copy_to_host(staging_buffer, (Slice(HostBufferCopy)) array_to_slice(((HostBufferCopy[]) {
        {
            .destination_offset = 0,
            .source = {
                .pointer = (u8*)vertices.pointer,
                .length = vertex_buffer_size,
            },
        },
        {
            .destination_offset = vertex_buffer_size,
            .source = {
                .pointer = (u8*)indices.pointer,
                .length = index_buffer_size,
            },
        },
    })));

    renderer_copy_to_local(renderer, (Slice(LocalBufferCopy)) array_to_slice(((LocalBufferCopy[]) {
        {
            .destination = vertex_buffer,
            .source = staging_buffer,
            .regions = array_to_slice(((LocalBufferCopyRegion[]) {
                {
                    .source_offset = 0,
                    .destination_offset = 0,
                    .size = vertex_buffer_size,
                },
            })),
        },
        {
            .destination = index_buffer,
            .source = staging_buffer,
            .regions = array_to_slice(((LocalBufferCopyRegion[]) {
                {
                    .source_offset = vertex_buffer_size,
                    .destination_offset = 0,
                    .size = index_buffer_size,
                },
            })),
        },
    })));

    while (!graphics_window_should_close(window))
    {
        graphics_poll_events();

        auto mouse_position = graphics_window_cursor_position_get(window);
        // print("Mouse position: ({f64}, {f64})\n", mouse_position.x, mouse_position.y);

        auto frame_window_size = renderer_window_frame_begin(renderer, render_window);


        window_command_begin(render_window);

        window_bind_pipeline(render_window, pipeline_index);
        window_bind_pipeline_descriptor_sets(render_window, pipeline_index);
        window_bind_index_buffer(render_window, index_buffer, 0, INDEX_TYPE_U32);
        GPUDrawPushConstants push_constants = {
            .vertex_buffer = vertex_buffer_device_address,
            .width = (f32)frame_window_size.width,
            .height = (f32)frame_window_size.height,
        };
        window_push_constants(render_window, pipeline_index, (SliceP(void)) array_to_slice(((void*[]) {&push_constants})));

        window_render_begin(render_window);
        {
            window_draw_indexed(render_window, indices.length, 1, 0, 0, 0);
        }
        window_render_end(render_window);

        window_command_end(render_window);

        renderer_window_frame_end(renderer, render_window);
    }

    // TODO: deinitialization
}

#endif
