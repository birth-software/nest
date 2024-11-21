#pragma once

#include <std/base.h>

#include <std/graphics.h>

#define frame_overlap (2)

typedef struct Renderer Renderer;
typedef struct RenderWindow RenderWindow;
typedef struct Pipeline Pipeline;

typedef enum IndexType : u8
{
    INDEX_TYPE_U16,
    INDEX_TYPE_U32,
} IndexType;

typedef enum TextureFormat : u8
{
    R8_UNORM,
    R8G8B8A8_SRGB,
} TextureFormat;

STRUCT(TextureMemory)
{
    u8* pointer;
    u32 width;
    u32 height;
    u32 depth;
    TextureFormat format;
};

typedef enum ShaderStage : u8
{
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_FRAGMENT,
} ShaderStage;

STRUCT(PipelineCreate)
{
    Slice(u16) shader_source_indices;
    u16 layout_index;
};
declare_slice(PipelineCreate);

STRUCT(PushConstantRange)
{
    u16 offset;
    u16 size;
    ShaderStage stage;
};
declare_slice(PushConstantRange);

typedef enum DescriptorType : u8
{
    DESCRIPTOR_TYPE_IMAGE_PLUS_SAMPLER,
    DESCRIPTOR_TYPE_COUNT,
} DescriptorType;

STRUCT(DescriptorSetLayoutBinding)
{
    u8 binding;
    DescriptorType type;
    ShaderStage stage;
    u8 count;
};
declare_slice(DescriptorSetLayoutBinding);

STRUCT(DescriptorSetLayout)
{
    Slice(DescriptorSetLayoutBinding) bindings;
};
declare_slice(DescriptorSetLayout);

STRUCT(PipelineLayoutCreate)
{
    Slice(PushConstantRange) push_constant_ranges;
    Slice(DescriptorSetLayout) descriptor_set_layouts;
};
declare_slice(PipelineLayoutCreate);

STRUCT(GraphicsPipelinesCreate)
{
    Slice(String) shader_sources;
    Slice(PipelineLayoutCreate) layouts;
    Slice(PipelineCreate) pipelines;
};

STRUCT(TextureIndex)
{
    u32 value;
};

STRUCT(PipelineIndex)
{
    u32 value;
};

STRUCT(PipelineLayoutIndex)
{
    u32 value;
};

STRUCT(DescriptorSetIndex)
{
    u32 value;
};

#define MAX_TEXTURE_UPDATE_COUNT (32)
#define MAX_DESCRIPTOR_SET_UPDATE_COUNT (16)
STRUCT(DescriptorSetUpdate)
{
    DescriptorSetIndex set;
    DescriptorType type;
    u32 binding;
    u32 descriptor_count;
    union
    {
        TextureIndex textures[MAX_TEXTURE_UPDATE_COUNT];
    };
};
declare_slice(DescriptorSetUpdate);

typedef enum BufferType : u8
{
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_STAGING,
} BufferType;

STRUCT(BufferIndex)
{
    u32 value;
};

STRUCT(HostBufferCopy)
{
    String source;
    u64 destination_offset;
};
declare_slice(HostBufferCopy);

STRUCT(LocalBufferCopyRegion)
{
    u64 source_offset;
    u64 destination_offset;
    u64 size;
};
declare_slice(LocalBufferCopyRegion);

STRUCT(LocalBufferCopy)
{
    BufferIndex destination;
    BufferIndex source;
    Slice(LocalBufferCopyRegion) regions;
};
declare_slice(LocalBufferCopy);

#define MAX_LOCAL_BUFFER_COPY_COUNT (16)

EXPORT Renderer* renderer_initialize();
EXPORT RenderWindow* renderer_window_initialize(Renderer* renderer, GraphicsWindow* window);
EXPORT PipelineIndex renderer_graphics_pipelines_create(Renderer* renderer, Arena* arena, GraphicsPipelinesCreate create_data);
EXPORT PipelineLayoutIndex renderer_pipeline_get_layout(PipelineIndex pipeline);
EXPORT GraphicsWindowSize renderer_window_frame_begin(Renderer* renderer, RenderWindow* window);
EXPORT void renderer_window_frame_end(Renderer* renderer, RenderWindow* window);
EXPORT TextureIndex renderer_texture_create(Renderer* renderer, TextureMemory texture_memory);
EXPORT BufferIndex renderer_buffer_create(Renderer* renderer, u64 size, BufferType type);
EXPORT void renderer_update_pipeline_resources(Renderer* renderer, PipelineIndex pipeline_index, Slice(DescriptorSetUpdate) descriptor_updates);
EXPORT void renderer_copy_to_host(BufferIndex buffer_index, Slice(HostBufferCopy) regions);
EXPORT void renderer_copy_to_local(Renderer* renderer, Slice(LocalBufferCopy) copies);
EXPORT void window_command_begin(RenderWindow* window);
EXPORT void window_command_end(RenderWindow* window);
EXPORT void window_render_begin(RenderWindow* window);
EXPORT void window_render_end(RenderWindow* window);

EXPORT void window_bind_pipeline(RenderWindow* window, PipelineIndex pipeline_index);
EXPORT void window_bind_pipeline_descriptor_sets(RenderWindow* window, PipelineIndex pipeline_index);
EXPORT void window_bind_index_buffer(RenderWindow* window, BufferIndex index_buffer, u64 offset, IndexType index_type);
EXPORT void window_push_constants(RenderWindow* window, PipelineIndex pipeline_index, SliceP(void) memories);
EXPORT u64 buffer_address(BufferIndex buffer_index);
EXPORT void window_draw_indexed(RenderWindow* window, u32 index_count, u32 instance_count, u32 first_index, s32 vertex_offset, u32 first_instance);
