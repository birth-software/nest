#if BB_CI == 0
#include <bloat-buster/gui.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <volk.h>

#include <bloat-buster/shader_compilation.h>
#include <bloat-buster/image_loader.h>
#include <bloat-buster/font.h>

[[noreturn]] [[gnu::cold]] fn void wrong_vulkan_result(VkResult result, String call_string, String file, int line)
{
    unused(result);
    unused(call_string);
    unused(file);
    unused(line);
    trap();
}

#define vkok(call) do {\
    VkResult _r_e_s_u_l_t_ = call; \
    if (unlikely(_r_e_s_u_l_t_ != VK_SUCCESS)) wrong_vulkan_result(_r_e_s_u_l_t_, strlit(#call), strlit(__FILE__), __LINE__); \
} while(0)

#define vkok_swapchain(call) do {\
    VkResult result = call; \
    if (unlikely(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)) wrong_vulkan_result(result, strlit(#call), strlit(__FILE__), __LINE__); \
} while(0)

STRUCT(GPUDrawPushConstants)
{
    VkDeviceAddress vertex_buffer;
    f32 width;
    f32 height;
};

fn u8 vk_layer_is_supported(String layer_name)
{
    assert(layer_name.pointer[layer_name.length] == 0);

    VkLayerProperties layers[256];
    u32 layer_count;

    vkok(vkEnumerateInstanceLayerProperties(&layer_count, 0));

    if (layer_count > array_length(layers))
    {
        failed_execution();
    }

    vkok(vkEnumerateInstanceLayerProperties(&layer_count, layers));

    u8 supported = 0;
    for (u32 i = 0; i < layer_count; i += 1)
    {
        VkLayerProperties* properties = &layers[i];

        auto candidate_layer_name = cstr(properties->layerName);
        if (s_equal(candidate_layer_name, layer_name))
        {
            supported = 1;
            break;
        }
    }

    return supported;
}

fn VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    unused(message_severity);
    unused(message_type);
    unused(user_data);
    print("Validation message ({cstr}): {cstr}\n", callback_data->pMessageIdName ? callback_data->pMessageIdName : "ID_NONE", callback_data->pMessage);
    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        failed_execution();
    }
    return VK_FALSE;
}

#define frame_overlap (2)

fn void vk_image_transition(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .image = image,
        .subresourceRange = {
            .aspectMask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    };

    VkDependencyInfo dependency_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &image_barrier,
    };

    vkCmdPipelineBarrier2(command_buffer, &dependency_info);
}

fn VkShaderModule vk_shader_module_create(Arena* arena, VkDevice device, const VkAllocationCallbacks* allocator, String path, ShaderStage shader_stage)
{
    auto binary = compile_shader(arena, path, shader_stage);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = binary.length,
        .pCode = (u32*)binary.pointer,
    };

    VkShaderModule shader_module;
    vkok(vkCreateShaderModule(device, &create_info, allocator, &shader_module));

    return shader_module;
}

fn VkDeviceMemory vk_allocate_memory(VkDevice device, const VkAllocationCallbacks* allocation_callbacks, VkPhysicalDeviceMemoryProperties memory_properties, VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags flags, u8 use_device_address_bit)
{
    u32 memory_type_index;
    for (memory_type_index = 0; memory_type_index < memory_properties.memoryTypeCount; memory_type_index += 1)
    {
        auto memory_type = memory_properties.memoryTypes[memory_type_index];

        if ((memory_requirements.memoryTypeBits & (1 << memory_type_index)) != 0 && (memory_type.propertyFlags & flags) == flags)
        {
            break;
        }
    }

    if (memory_type_index == memory_properties.memoryTypeCount)
    {
        failed_execution();
    }

    VkMemoryAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index,
    };

    VkMemoryAllocateFlagsInfo flags_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .pNext = 0,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        .deviceMask = 1,
    };

    if (use_device_address_bit)
    {
        allocate_info.pNext = &flags_info;
    }

    VkDeviceMemory memory = 0;
    vkok(vkAllocateMemory(device, &allocate_info, allocation_callbacks, &memory));

    return memory;
}

STRUCT(VulkanBuffer)
{
    VkBuffer handle;
    VkDeviceMemory gpu_data;
    void* cpu_data;
    VkDeviceSize size;
};

STRUCT(ImmediateContext)
{
    VkDevice device;
    VkFence fence;
    VkCommandBuffer command_buffer;
    VkCommandPool command_pool;
    VkQueue queue;
};

fn void immediate_start(ImmediateContext context)
{
    VkFence fences[] = { context.fence };
    VkCommandBufferResetFlags reset_flags = 0;

    vkok(vkResetFences(context.device, array_length(fences), fences));
    vkok(vkResetCommandBuffer(context.command_buffer, reset_flags));

    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkok(vkBeginCommandBuffer(context.command_buffer, &command_buffer_begin_info));
}

fn void immediate_end(ImmediateContext context)
{
    VkFence fences[] = { context.fence };

    vkok(vkEndCommandBuffer(context.command_buffer));

    VkCommandBufferSubmitInfo command_buffer_submit_infos[] = {
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .pNext = 0,
            .deviceMask = 0,
            .commandBuffer = context.command_buffer,
        }
    };

    VkSubmitFlags submit_flags = 0;

    VkSubmitInfo2 submit_info[] = {
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = 0,
            .flags = submit_flags,
            .waitSemaphoreInfoCount = 0,
            .pWaitSemaphoreInfos = 0,
            .commandBufferInfoCount = array_length(command_buffer_submit_infos),
            .pCommandBufferInfos = command_buffer_submit_infos,
            .signalSemaphoreInfoCount = 0,
            .pSignalSemaphoreInfos = 0,
        }
    };

    vkok(vkQueueSubmit2(context.queue, array_length(submit_info), submit_info, context.fence));
    VkBool32 wait_all = 1;
    auto timeout = ~(u64)0;
    vkok(vkWaitForFences(context.device, array_length(fences), fences, wait_all, timeout));
}
STRUCT(VulkanCopyImage)
{
    VkImage handle;
    VkExtent2D extent;
};

STRUCT(VulkanCopyImageArgs)
{
    VulkanCopyImage source;
    VulkanCopyImage destination;
};

fn void vk_image_copy(VkCommandBuffer command_buffer, VulkanCopyImageArgs args)
{
    VkImageSubresourceLayers subresource_layers = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    VkImageBlit2 blit_regions[] = {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
            .pNext = 0,
            .srcSubresource = subresource_layers,
            .srcOffsets = {
                [1] = {
                    .x = args.source.extent.width,
                    .y = args.source.extent.height,
                    .z = 1,
                },
            },
            .dstSubresource = subresource_layers,
            .dstOffsets = {
                [1] = {
                    .x = args.destination.extent.width,
                    .y = args.destination.extent.height,
                    .z = 1,
                },
            },
        },
    };

    VkBlitImageInfo2 blit_info = {
        .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
        .pNext = 0,
        .srcImage = args.source.handle,
        .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .dstImage = args.destination.handle,
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = array_length(blit_regions),
        .pRegions = blit_regions,
        .filter = VK_FILTER_LINEAR,
    };

    vkCmdBlitImage2(command_buffer, &blit_info);
}

fn VulkanBuffer vk_buffer_create(VkDevice device, const VkAllocationCallbacks* allocation_callbacks, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkDeviceSize buffer_size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_flags)
{
    VkBuffer buffer;
    VkBufferCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .size = buffer_size,
        .usage = usage_flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
    };
    vkok(vkCreateBuffer(device, &create_info, allocation_callbacks, &buffer));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

    u8 use_device_address_bit = (create_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0;
    VkDeviceMemory memory = vk_allocate_memory(device, allocation_callbacks, physical_device_memory_properties, memory_requirements, memory_flags, use_device_address_bit);

    VkDeviceSize memory_offset = 0;
    vkok(vkBindBufferMemory(device, buffer, memory, memory_offset));

    void* data = 0;
    if (memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        VkDeviceSize offset = 0;
        VkMemoryMapFlags map_flags = 0;
        vkok(vkMapMemory(device, memory, offset, memory_requirements.size, map_flags, &data));
    }

    VulkanBuffer result = {
        .handle = buffer,
        .cpu_data = data,
        .gpu_data = memory,
        .size = buffer_size,
    };

    return result;
}

STRUCT(VulkanImageCreate)
{
    u32 width;
    u32 height;
    u32 mip_levels;
    VkFormat format;
    VkImageUsageFlags usage;
};

STRUCT(VulkanImage)
{
    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory;
    VkFormat format;
};

fn VulkanImage vk_image_create(VkDevice device, const VkAllocationCallbacks* allocation_callbacks, VkPhysicalDeviceMemoryProperties memory_properties, VulkanImageCreate create)
{
    assert(create.width);
    assert(create.height);
    VulkanImage result = {};
    result.format = create.format;

    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = create.format,
        .extent = {
            .width = create.width,
            .height = create.height,
            .depth = 1,
        },
        .mipLevels = create.mip_levels,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = create.usage,
        .sharingMode = 0,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
        .initialLayout = 0,
    };
    vkok(vkCreateImage(device, &create_info, allocation_callbacks, &result.handle));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(device, result.handle, &memory_requirements);

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    u8 use_device_address_bit = 0;
    result.memory = vk_allocate_memory(device, allocation_callbacks, memory_properties, memory_requirements, flags, use_device_address_bit);

    VkDeviceSize memory_offset = 0;
    vkok(vkBindImageMemory(device, result.handle, result.memory, memory_offset));

    VkImageViewCreateInfo view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = result.handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = create_info.format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = create.mip_levels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkok(vkCreateImageView(device, &view_create_info, allocation_callbacks, &result.view));
    return result;
}

STRUCT(ImageFromTextureOptions)
{
    VkFormat format;
};

fn VulkanImage vk_image_from_texture(VkDevice device, const VkAllocationCallbacks* allocation_callbacks, ImmediateContext immediate_context, VkPhysicalDeviceMemoryProperties memory_properties, TextureMemory texture, VkFormat format)
{
    assert(texture.depth == 1);

    VulkanImage result = vk_image_create(device, allocation_callbacks, memory_properties, (VulkanImageCreate) {
        .width = texture.width,
        .height = texture.height,
        .mip_levels = 1,
        .format = format,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });

    auto image_size = (u64)texture.depth * texture.width * texture.height * texture.channel_count;
    VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags buffer_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto transfer_buffer = vk_buffer_create(device, allocation_callbacks, memory_properties, image_size, buffer_usage_flags, buffer_memory_flags);
    memcpy(transfer_buffer.cpu_data, texture.pointer, image_size);

    immediate_start(immediate_context);

    vk_image_transition(immediate_context.command_buffer, result.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copy_regions[] = {
        {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = { .x = 0, .y = 0, .z = 0 },
            .imageExtent = {
                .width = texture.width,
                .height = texture.height,
                .depth = texture.depth,
            },
        }
    };

    vkCmdCopyBufferToImage(immediate_context.command_buffer, transfer_buffer.handle, result.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, array_length(copy_regions), copy_regions);

    vk_image_transition(immediate_context.command_buffer, result.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    immediate_end(immediate_context);

    return result;
}

void run_app(Arena* arena)
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
#endif

    if (glfwInit() != GLFW_TRUE)
    {
        failed_execution();
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    vkok(volkInitialize());

    const VkAllocationCallbacks* allocation_callbacks = 0;
    VkInstance instance;
    {
        auto debug_layer = strlit("VK_LAYER_KHRONOS_validation");
        if (!vk_layer_is_supported(debug_layer))
        {
            failed_execution();
        }
        const char* layers[] =
        {
            string_to_c(debug_layer),
        };

        auto api_version = volkGetInstanceVersion();
        if (api_version < VK_API_VERSION_1_3)
        {
            failed_execution();
        }

        const char* extensions[] = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
            VK_KHR_COCOA_SURFACE_EXTENSION_NAME,
#endif
        };

        VkDebugUtilsMessengerCreateInfoEXT msg_ci = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = 0,
        };

        VkValidationFeatureEnableEXT enabled_validation_features[] =
        {
            // VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        };

        VkValidationFeaturesEXT validation_features = { 
            .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
            .enabledValidationFeatureCount = array_length(enabled_validation_features),
            .pEnabledValidationFeatures = enabled_validation_features,
            .pNext = &msg_ci,
        };

        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = api_version,
        };

        VkInstanceCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .ppEnabledLayerNames = layers,
            .enabledLayerCount = array_length(layers),
            .ppEnabledExtensionNames = extensions,
            .enabledExtensionCount = array_length(extensions),
            .pNext = &validation_features,
        };

        vkok(vkCreateInstance(&ci, allocation_callbacks, &instance));

        volkLoadInstance(instance);
        VkDebugUtilsMessengerEXT messenger;
        vkok(vkCreateDebugUtilsMessengerEXT(instance, &msg_ci, allocation_callbacks, &messenger));
    }

    int initial_width = 1024;
    int initial_height = 768;
    GLFWwindow* window = glfwCreateWindow(initial_width, initial_height, "Bloat Buster", 0, 0);

    if (!window)
    {
        failed_execution();
    }

    VkSurfaceKHR surface;
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = 0,
            .flags = 0,
            .hinstance = GetModuleHandleW(0),
            .hwnd = glfwGetWin32Window(window),
        };
        vkok(vkCreateWin32SurfaceKHR(instance, &create_info, allocation_callbacks, &surface));
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        VkXlibSurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = 0,
            .flags = 0,
            .dpy = glfwGetX11Display(),
            .window = glfwGetX11Window(window),
        };
        vkok(vkCreateXlibSurfaceKHR(instance, &create_info, allocation_callbacks, &surface));
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
            VK_KHR_COCOA_SURFACE_EXTENSION_NAME,
#endif
    }

    VkPhysicalDevice physical_device;
    {
        u32 physical_device_count;
        VkPhysicalDevice physical_devices[256];
        vkok(vkEnumeratePhysicalDevices(instance, &physical_device_count, 0));

        if (physical_device_count == 0)
        {
            failed_execution();
        }

        if (physical_device_count > array_length(physical_devices))
        {
            failed_execution();
        }

        vkok(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices));

        physical_device = physical_devices[0];
    }

    VkDevice device;
    u32 graphics_queue_family_index;
    u32 present_queue_family_index;
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        u32 queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, 0);

        VkQueueFamilyProperties queue_properties[64];
        if (queue_count > array_length(queue_properties))
        {
            failed_execution();
        }

        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_properties);

        for (graphics_queue_family_index = 0; graphics_queue_family_index < queue_count; graphics_queue_family_index += 1)
        {
            VkQueueFamilyProperties* properties = &queue_properties[graphics_queue_family_index];
            if (properties->queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                break;
            }
        }

        if (graphics_queue_family_index == queue_count)
        {
            failed_execution();
        }

        for (present_queue_family_index = 0; present_queue_family_index < queue_count; present_queue_family_index += 1)
        {
            VkBool32 support;
            vkok(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, present_queue_family_index, surface, &support));
            if (support)
            {
                break;
            }
        }

        if (present_queue_family_index == queue_count)
        {
            failed_execution();
        }

        if (present_queue_family_index != graphics_queue_family_index)
        {
            failed_execution();
        }

        f32 queue_priorities[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_create_infos[] = {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = graphics_queue_family_index,
                .queueCount = array_length(queue_priorities),
                .pQueuePriorities = queue_priorities,
            },
        };

        const char* extensions[] =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        VkPhysicalDeviceVulkan13Features features13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .dynamicRendering = 1,
            .synchronization2 = 1,
        };

        VkPhysicalDeviceVulkan12Features features12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .bufferDeviceAddress = 1,
            .descriptorIndexing = 1,
            .pNext = &features13,
        };

        VkPhysicalDeviceFeatures2 features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .features = {
            },
            .pNext = &features12,
        };

        VkDeviceCreateInfo ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .ppEnabledExtensionNames = extensions,
            .enabledExtensionCount = array_length(extensions),
            .pQueueCreateInfos = queue_create_infos,
            .queueCreateInfoCount = array_length(queue_create_infos),
            .pNext = &features,
        };

        vkok(vkCreateDevice(physical_device, &ci, allocation_callbacks, &device));
    }

    VkQueue graphics_queue;
    vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);

    VkSurfaceCapabilitiesKHR original_capabilities;
    vkok(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &original_capabilities));

    VkSwapchainKHR swapchain = 0;
    
    u32 queue_family_indices[] = { graphics_queue_family_index };
    VkImageUsageFlags swapchain_image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkFormat common_image_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .surface = surface,
        .minImageCount = original_capabilities.minImageCount,
        .imageFormat = common_image_format,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = original_capabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = swapchain_image_usage_flags,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = array_length(queue_family_indices),
        .pQueueFamilyIndices = queue_family_indices,
        .preTransform = original_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = 0,
        .oldSwapchain = swapchain,
    };

    {
        VkSwapchainKHR new_swapchain;
        vkok(vkCreateSwapchainKHR(device, &swapchain_create_info, allocation_callbacks, &new_swapchain));
        swapchain = new_swapchain;
    }

    VkImage swapchain_image_buffer[16];
    VkImageView swapchain_image_view_buffer[array_length(swapchain_image_buffer)];
    u32 swapchain_image_count;
    {
        vkok(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, 0));

        if (swapchain_image_count == 0)
        {
            failed_execution();
        }

        if (swapchain_image_count > array_length(swapchain_image_buffer))
        {
            failed_execution();
        }

        vkok(vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_image_buffer));
        
        // VkImageViewUsageCreateInfo image_view_usage_create_info = {
        //     .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
        //     .pNext = 0,
        //     .usage = swapchain_create_info.imageUsage,
        // };

        for (u32 i = 0; i < swapchain_image_count; i += 1)
        {
            VkImageViewCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                // .pNext = &image_view_usage_create_info,
                .flags = 0,
                .image = swapchain_image_buffer[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = swapchain_create_info.imageFormat,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            };

            vkok(vkCreateImageView(device, &create_info, allocation_callbacks, &swapchain_image_view_buffer[i]));
        }
    }

    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &physical_device_memory_properties);

    VulkanImage render_image = vk_image_create(device, allocation_callbacks, physical_device_memory_properties, (VulkanImageCreate) {
        .width = initial_width,
        .height = initial_height,
        .mip_levels = 1,
        .format = common_image_format,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });

    VkCommandPool command_pools[frame_overlap];
    VkCommandBuffer command_buffers[frame_overlap];
    ImmediateContext immediate;
    immediate.device = device;
    immediate.queue = graphics_queue;
    {
        VkCommandPoolCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphics_queue_family_index,
        };

        for (u32 i = 0; i < frame_overlap; i += 1)
        {
            vkok(vkCreateCommandPool(device, &create_info, allocation_callbacks, &command_pools[i]));

            VkCommandBufferAllocateInfo command_buffer_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = command_pools[i],
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            vkok(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffers[i]));
        }

        vkok(vkCreateCommandPool(device, &create_info, allocation_callbacks, &immediate.command_pool));

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = immediate.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkok(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &immediate.command_buffer));
    }

    VkFence render_fences[frame_overlap];
    VkSemaphore swapchain_semaphores[frame_overlap];
    VkSemaphore render_semaphores[frame_overlap];
    {
        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };
        
        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .flags = 0,
        };

        for (u32 i = 0; i < frame_overlap; i += 1)
        {
            vkok(vkCreateFence(device, &fence_create_info, allocation_callbacks, &render_fences[i]));
            vkok(vkCreateSemaphore(device, &semaphore_create_info, allocation_callbacks, &render_semaphores[i]));
            vkok(vkCreateSemaphore(device, &semaphore_create_info, allocation_callbacks, &swapchain_semaphores[i]));
        }

        vkok(vkCreateFence(device, &fence_create_info, allocation_callbacks, &immediate.fence));
    }

    VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = 0,
        },
        // {
        //     .binding = 1,
        //     .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        //     .descriptorCount = 1,
        //     .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        //     .pImmutableSamplers = 0,
        // },
    };
    VkDescriptorSetLayout descriptor_set_layout;
    {
        VkDescriptorSetLayoutCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .bindingCount = array_length(descriptor_set_layout_bindings),
            .pBindings = descriptor_set_layout_bindings,
        };

        vkok(vkCreateDescriptorSetLayout(device, &create_info, allocation_callbacks, &descriptor_set_layout));
    }

    VkDescriptorSetLayout descriptor_set_layouts[] = { descriptor_set_layout };
    VkPipeline graphics_pipeline;
    VkPipelineLayout graphics_pipeline_layout;
    {
        VkShaderModule vertex_shader = vk_shader_module_create(arena, device, allocation_callbacks, strlit("bootstrap/shaders/font.vert"), SHADER_STAGE_VERTEX);
        VkShaderModule fragment_shader = vk_shader_module_create(arena, device, allocation_callbacks, strlit("bootstrap/shaders/font.frag"), SHADER_STAGE_FRAGMENT);

        VkPushConstantRange push_constant_ranges[] = {
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(GPUDrawPushConstants),
            }
        };

        VkPipelineLayoutCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .flags = 0,
            .setLayoutCount = array_length(descriptor_set_layouts),
            .pSetLayouts = descriptor_set_layouts,
            .pushConstantRangeCount = array_length(push_constant_ranges),
            .pPushConstantRanges = push_constant_ranges,
        };

        vkok(vkCreatePipelineLayout(device, &create_info, allocation_callbacks, &graphics_pipeline_layout));

        VkShaderModule shaders[] = {
            vertex_shader,
            fragment_shader,
        };

        VkShaderStageFlags stages[] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
        VkPipelineShaderStageCreateInfo shader_create_infos[2];

        for (u32 i = 0; i < array_length(shaders); i += 1)
        {
            shader_create_infos[i] = (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .stage = stages[i],
                .module = shaders[i],
                .pName = "main",
                .pSpecializationInfo = 0,
            };
        }

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = 0,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = 0,
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = 0,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .depthClampEnable = 0,
            .rasterizerDiscardEnable = 0,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable = 0,
            .depthBiasConstantFactor = 0,
            .depthBiasClamp = 0,
            .depthBiasSlopeFactor = 0,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = 0,
            .minSampleShading = 1.0f,
            .pSampleMask = 0,
            .alphaToCoverageEnable = 0,
            .alphaToOneEnable = 0,
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .depthTestEnable = 0,
            .depthWriteEnable = 0,
            .depthCompareOp = VK_COMPARE_OP_NEVER,
            .depthBoundsTestEnable = 0,
            .stencilTestEnable = 0,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f,
        };

        VkPipelineColorBlendAttachmentState attachments[] = {
            {
                .blendEnable = 1,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            },
        };

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = array_length(attachments),
            .pAttachments = attachments,
            .blendConstants = {},
        };

        VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .dynamicStateCount = array_length(states),
            .pDynamicStates = states,
        };

        VkFormat color_attachment_formats[] = {
            common_image_format,
        };

        VkPipelineRenderingCreateInfo rendering_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = 0,
            .viewMask = 0,
            .colorAttachmentCount = array_length(color_attachment_formats),
            .pColorAttachmentFormats = color_attachment_formats,
            .depthAttachmentFormat = 0,
            .stencilAttachmentFormat = 0,
        };

        VkGraphicsPipelineCreateInfo create_infos[] = {
            {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .pNext = &rendering_create_info,
                .flags = 0,
                .stageCount = array_length(stages),
                .pStages = shader_create_infos,
                .pVertexInputState = &vertex_input_state_create_info,
                .pInputAssemblyState = &input_assembly_state_create_info,
                .pTessellationState = 0,
                .pViewportState = &viewport_state_create_info,
                .pRasterizationState = &rasterization_state_create_info,
                .pMultisampleState = &multisample_state_create_info,
                .pDepthStencilState = &depth_stencil_state_create_info,
                .pColorBlendState = &color_blend_state_create_info,
                .pDynamicState = &dynamic_state_create_info,
                .layout = graphics_pipeline_layout,
                .renderPass = 0,
                .subpass = 0,
                .basePipelineHandle = 0,
                .basePipelineIndex = 0,
            },
        };

        VkPipelineCache pipeline_cache = 0;
        vkok(vkCreateGraphicsPipelines(device, pipeline_cache, array_length(create_infos), create_infos, allocation_callbacks, &graphics_pipeline));
    }

    auto font_path = 
#ifdef _WIN32
strlit("C:/Users/David/Downloads/Fira_Sans/FiraSans-Regular.ttf")
#elif defined(__linux__)
strlit("/usr/share/fonts/TTF/FiraSans-Regular.ttf")
#else
#endif
;
    auto texture_atlas = font_create_texture_atlas(arena, font_path);
    auto texture_atlas_image = vk_image_from_texture(device, allocation_callbacks, immediate, physical_device_memory_properties, (TextureMemory) {
        .pointer = texture_atlas.pointer,
        .width = texture_atlas.width,
        .height = texture_atlas.height,
        .channel_count = 1,
        .depth = 1,
    }, VK_FORMAT_R8_UNORM);

//     auto texture_path =
// #ifdef _WIN32
//         strlit("C:/Users/david/Pictures/buster.jpg");
// #elif defined(__linux__)
//         strlit("/home/david/Pictures/buster.jpeg");
// #else
// #endif
//     auto texture = texture_load_from_file(arena, texture_path);
//     auto texture_image = vk_image_from_texture(device, allocation_callbacks, immediate, physical_device_memory_properties, texture, VK_FORMAT_R8G8B8A8_SRGB);

    VkSampler sampler;
    {
        VkFilter sampler_filter = VK_FILTER_LINEAR;
        VkSamplerCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .magFilter = sampler_filter,
            .minFilter = sampler_filter,
            .mipmapMode = 0,
            .addressModeU = 0,
            .addressModeV = 0,
            .addressModeW = 0,
            .mipLodBias = 0,
            .anisotropyEnable = 0,
            .maxAnisotropy = 0,
            .compareEnable = 0,
            .compareOp = 0,
            .minLod = 0,
            .maxLod = 0,
            .borderColor = 0,
            .unnormalizedCoordinates = 0,
        };

        vkok(vkCreateSampler(device, &create_info, allocation_callbacks, &sampler));
    }

    VkDescriptorPool descriptor_pools[frame_overlap];
    VkDescriptorSet descriptor_sets[frame_overlap];
    for (u32 i = 0; i < frame_overlap; i += 1)
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {
                .type = descriptor_set_layout_bindings[0].descriptorType,
                .descriptorCount = array_length(descriptor_set_layout_bindings),
            },
        };
        VkDescriptorPoolCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .maxSets = 1,
            .poolSizeCount = array_length(pool_sizes),
            .pPoolSizes = pool_sizes,
        };

        vkok(vkCreateDescriptorPool(device, &create_info, allocation_callbacks, &descriptor_pools[i]));

        VkDescriptorSetAllocateInfo allocate_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = 0,
            .descriptorPool = descriptor_pools[i],
            .descriptorSetCount = array_length(descriptor_set_layouts),
            .pSetLayouts = descriptor_set_layouts,
        };

        vkok(vkAllocateDescriptorSets(device, &allocate_info, &descriptor_sets[i]));

        VkDescriptorImageInfo image_infos[] = {
            {
                .sampler = sampler,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .imageView = texture_atlas_image.view,
            },
        };

        VkWriteDescriptorSet write_descriptor_sets[] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = 0,
                .dstSet = descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = array_length(image_infos),
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = image_infos,
                .pBufferInfo = 0,
                .pTexelBufferView = 0,
            },
        };

        u32 descriptor_copy_count = 0;
        VkCopyDescriptorSet* descriptor_copies = 0;
        vkUpdateDescriptorSets(device, array_length(write_descriptor_sets), write_descriptor_sets, descriptor_copy_count, descriptor_copies);
    }

    STRUCT(Vec4)
    {
        f32 v[4];
    };
    Vec4 color = {1, 1, 1, 1};
    static_assert(sizeof(color) == 4 * sizeof(float));

    STRUCT(Vertex)
    {
        f32 x;
        f32 y;
        f32 uv_x;
        f32 uv_y;
        Vec4 color;
    };

    auto width_float = (f32)initial_width;
    auto height_float = (f32)initial_height;
    Vertex vertices[4*2];

    u32 indices[] = {
        0, 1, 2,
        1, 3, 2,
        0 + 4, 1 + 4, 2 + 4,
        1 + 4, 3 + 4, 2 + 4
    };

    {
        u8 c = 'a';
        auto character_count_per_row = texture_atlas.width / texture_atlas.char_height;
        auto row = c / character_count_per_row;
        auto column = c % character_count_per_row;
        auto pos_x = width_float / 2;
        auto pos_y = height_float / 2;
        auto uv_x = column * texture_atlas.char_height;
        auto uv_y = row * texture_atlas.char_height;

        vertices[0] = (Vertex) {
            .x = pos_x,
            .y = pos_y,
            .uv_x = (f32)uv_x / texture_atlas.width,
            .uv_y = (f32)uv_y / texture_atlas.width,
            .color = color,
        };
        vertices[1] = (Vertex) {
            .x = pos_x + texture_atlas.char_height,
            .y = pos_y,
            .uv_x = (f32)(uv_x + texture_atlas.char_height) / texture_atlas.width,
            .uv_y = (f32)uv_y / texture_atlas.width,
            .color = color,
        };
        vertices[2] = (Vertex) {
            .x = pos_x,
            .y = pos_y + texture_atlas.char_height,
            .uv_x = (f32)uv_x / texture_atlas.width,
            .uv_y = (f32)(uv_y + texture_atlas.char_height) / texture_atlas.width,
            .color = color,
        };
        vertices[3] = (Vertex) {
            .x = pos_x + texture_atlas.char_height,
            .y = pos_y + texture_atlas.char_height,
            .uv_x = (f32)(uv_x + texture_atlas.char_height) / texture_atlas.width,
            .uv_y = (f32)(uv_y + texture_atlas.char_height) / texture_atlas.width,
            .color = color,
        };
    }

    {
        u8 c = 'b';
        auto character_count_per_row = texture_atlas.width / texture_atlas.char_height;
        auto row = c / character_count_per_row;
        auto column = c % character_count_per_row;
        auto pos_x = width_float / 2 + texture_atlas.char_height;
        auto pos_y = height_float / 2;
        auto uv_x = column * texture_atlas.char_height;
        auto uv_y = row * texture_atlas.char_height;

        vertices[4] = (Vertex) {
            .x = pos_x,
            .y = pos_y,
            .uv_x = (f32)uv_x / texture_atlas.width,
            .uv_y = (f32)uv_y / texture_atlas.width,
            .color = color,
        };
        vertices[5] = (Vertex) {
            .x = pos_x + texture_atlas.char_height,
            .y = pos_y,
            .uv_x = (f32)(uv_x + texture_atlas.char_height) / texture_atlas.width,
            .uv_y = (f32)uv_y / texture_atlas.width,
            .color = color,
        };
        vertices[6] = (Vertex) {
            .x = pos_x,
            .y = pos_y + texture_atlas.char_height,
            .uv_x = (f32)uv_x / texture_atlas.width,
            .uv_y = (f32)(uv_y + texture_atlas.char_height) / texture_atlas.width,
            .color = color,
        };
        vertices[7] = (Vertex) {
            .x = pos_x + texture_atlas.char_height,
            .y = pos_y + texture_atlas.char_height,
            .uv_x = (f32)(uv_x + texture_atlas.char_height) / texture_atlas.width,
            .uv_y = (f32)(uv_y + texture_atlas.char_height) / texture_atlas.width,
            .color = color,
        };
    }

    VulkanBuffer vertex_buffer = vk_buffer_create(device, allocation_callbacks, physical_device_memory_properties, sizeof(vertices), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VulkanBuffer index_buffer = vk_buffer_create(device, allocation_callbacks, physical_device_memory_properties, sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkBufferDeviceAddressInfo device_address_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = vertex_buffer.handle,
    };
    VkDeviceAddress vertex_buffer_device_address = vkGetBufferDeviceAddress(device, &device_address_info);

    VulkanBuffer staging_buffer = vk_buffer_create(device, allocation_callbacks, physical_device_memory_properties, vertex_buffer.size + index_buffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    {

        memcpy(staging_buffer.cpu_data, vertices, sizeof(vertices));
        memcpy((u8*)staging_buffer.cpu_data + sizeof(vertices), indices, sizeof(indices));

        immediate_start(immediate);

        VkBufferCopy vertex_copies[] =
        {
            {
                .srcOffset = 0,
                .dstOffset= 0,
                .size = vertex_buffer.size,
            }
        };

        vkCmdCopyBuffer(immediate.command_buffer, staging_buffer.handle, vertex_buffer.handle, array_length(vertex_copies), vertex_copies);

        VkBufferCopy index_copies[] =
        {
            {
                .srcOffset = vertex_buffer.size,
                .dstOffset= 0,
                .size = index_buffer.size,
            }
        };

        vkCmdCopyBuffer(immediate.command_buffer, staging_buffer.handle, index_buffer.handle, array_length(index_copies), index_copies);

        immediate_end(immediate);
    }

    u32 frame_completed = 0;
    for (u32 frame_number = 0; !glfwWindowShouldClose(window); frame_number += frame_completed)
    {
        frame_completed = 0;
        glfwPollEvents();

        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        if (width == 0 || height == 0)
        {
            trap();
        }

        u32 frame_index = frame_number % frame_overlap;
        u32 swapchain_image_index;

        {
            auto timeout = ~(u64)0;

            u32 fence_count = 1;
            VkBool32 wait_all = 1;
            vkok(vkWaitForFences(device, fence_count, &render_fences[frame_index], wait_all, timeout));
            VkFence image_fence = 0;
            vkok_swapchain(vkAcquireNextImageKHR(device, swapchain, timeout, swapchain_semaphores[frame_index], image_fence, &swapchain_image_index));
            vkok(vkResetFences(device, fence_count, &render_fences[frame_index]));
        }

        VkCommandBuffer command_buffer = command_buffers[frame_index];

        VkCommandBufferResetFlags reset_flags = 0;
        vkok(vkResetCommandBuffer(command_buffer, reset_flags));

        // Record the commands
        {
            {
                VkCommandBufferBeginInfo command_buffer_begin_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };
                vkok(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));
            }

            vk_image_transition(command_buffer, render_image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

            VkRenderingAttachmentInfo color_attachments[] = {
                {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = render_image.view,
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = { .color = { .float32 = { 255.0f, 0.0f, 255.0f, 1.0f } } },
                },
            };
            
            VkRenderingInfo rendering_info = {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                .renderArea = {
                    .extent = {
                        .width = initial_width,
                        .height = initial_height,
                    },
                },
                .layerCount = 1,
                .colorAttachmentCount = array_length(color_attachments),
                .pColorAttachments = color_attachments,
            };
            
            vkCmdBeginRendering(command_buffer, &rendering_info);

            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

            // Bind texture
            u32 first_set = 0;
            u32 descriptor_set_count = 1;
            u32 dynamic_offset_count = 0;
            u32* dynamic_offsets = 0;
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, first_set, descriptor_set_count, &descriptor_sets[frame_index], dynamic_offset_count, dynamic_offsets);

            VkViewport viewports[] = {
                {
                    .x = 0,
                    .y = 0,
                    .width = initial_width,
                    .height = initial_height,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                }
            };

            u32 first_viewport = 0;
            vkCmdSetViewport(command_buffer, first_viewport, array_length(viewports), viewports);

            VkRect2D scissors[] = {
                {
                    .offset = {
                        .x = 0,
                        .y = 0,
                    },
                    .extent = {
                        .width = initial_width,
                        .height = initial_height,
                    },
                }
            };

            u32 first_scissor = 0;
            vkCmdSetScissor(command_buffer, first_scissor, array_length(scissors), scissors);

            GPUDrawPushConstants push_constants = {
                .vertex_buffer = vertex_buffer_device_address,
                .width = (f32)width,
                .height = (f32)height,
            };
            VkShaderStageFlags shader_stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
            u32 push_constant_offset = 0;
            u32 push_constant_size = sizeof(push_constants);
            vkCmdPushConstants(command_buffer, graphics_pipeline_layout, shader_stage_flags, push_constant_offset, push_constant_size, &push_constants);

            VkDeviceSize index_buffer_offset = 0;
            VkIndexType index_type = VK_INDEX_TYPE_UINT32;
            vkCmdBindIndexBuffer(command_buffer, index_buffer.handle, index_buffer_offset, index_type);

            vkCmdDrawIndexed(command_buffer, array_length(indices), 1, 0, 0, 0);
            
            vkCmdEndRendering(command_buffer);

            vk_image_transition(command_buffer, render_image.handle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

            VkImage swapchain_image = swapchain_image_buffer[swapchain_image_index];
            vk_image_transition(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            vk_image_copy(command_buffer, (VulkanCopyImageArgs) {
                .source = {
                    .handle = render_image.handle,
                    .extent = {
                        .width = initial_width,
                        .height = initial_height,
                    },
                },
                .destination = {
                    .handle = swapchain_image,
                    .extent = {
                        .width = initial_width,
                        .height = initial_height,
                    },
                },
            });
            
            vk_image_transition(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            vkok(vkEndCommandBuffer(command_buffer));
        }

        VkCommandBufferSubmitInfo command_buffer_submit_info[] = {
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = command_buffer,
                .deviceMask = 0,
            },
        };

        VkSemaphore swapchain_semaphore = swapchain_semaphores[frame_index];
        VkSemaphoreSubmitInfo wait_semaphore_submit_info[] = {
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = swapchain_semaphore,
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                .deviceIndex = 0,
                .value = 1,
            },
        };

        VkSemaphore render_semaphore = render_semaphores[frame_index];
        VkSemaphoreSubmitInfo signal_semaphore_submit_info[] = {
            {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = render_semaphore,
                .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
                .deviceIndex = 0,
                .value = 1,
            },
        };

        VkSubmitInfo2 submit_info[] = {
            {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                .flags = 0,
                .waitSemaphoreInfoCount = array_length(wait_semaphore_submit_info),
                .pWaitSemaphoreInfos = wait_semaphore_submit_info,
                .signalSemaphoreInfoCount = array_length(signal_semaphore_submit_info),
                .pSignalSemaphoreInfos = signal_semaphore_submit_info,
                .commandBufferInfoCount = array_length(command_buffer_submit_info),
                .pCommandBufferInfos = command_buffer_submit_info,
            },
        };

        VkFence render_fence = render_fences[frame_index];
        vkok(vkQueueSubmit2(graphics_queue, array_length(submit_info), submit_info, render_fence));

        VkSwapchainKHR swapchains[] = { swapchain };
        VkSemaphore wait_semaphores[] = { render_semaphore };
        VkResult results[array_length(swapchains)];

        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = array_length(wait_semaphores),
            .pWaitSemaphores = wait_semaphores,
            .swapchainCount = array_length(swapchains),
            .pSwapchains = swapchains,
            .pImageIndices = &swapchain_image_index,
            .pResults = results,
        };

        vkok_swapchain(vkQueuePresentKHR(graphics_queue, &present_info));

        frame_completed = 1;
    }

    // {
    //     VkResult result = vkDeviceWaitIdle(device);
    //     if (result != VK_SUCCESS)
    //     {
    //         failed_execution();
    //     }
    // }
    //
    // vkDestroyInstance(instance, allocation_callbacks);
}

#endif
