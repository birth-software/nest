#include <std/render.h>
#include <std/shader_compilation.h>
#include <std/string.h>
#include <std/image_loader.h>

#include <volk.h>

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

STRUCT(Renderer)
{
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    u32 graphics_queue_family_index;
    VkSampler sampler;
    ImmediateContext immediate;
    VkPhysicalDeviceMemoryProperties memory_properties;
};

#define MAX_SWAPCHAIN_IMAGE_COUNT (16)
#define MAX_FRAMES (2)

STRUCT(WindowFrame)
{
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSemaphore swapchain_semaphore;
    VkSemaphore render_semaphore;
    VkFence render_fence;
};

STRUCT(RenderWindow)
{
    GraphicsWindow* graphics_window;
    VkSwapchainKHR swapchain;
    VulkanImage render_image;
    u32 width;
    u32 height;
    u32 last_width;
    u32 last_height;
    u32 frame_index;
    u32 swapchain_image_index;
    u32 swapchain_image_count;
    VkImage swapchain_images[MAX_SWAPCHAIN_IMAGE_COUNT];
    VkImageView swapchain_image_views[MAX_SWAPCHAIN_IMAGE_COUNT];
    VkFormat swapchain_image_format;
    WindowFrame frames[MAX_FRAMES];
};

global_variable Renderer renderer_memory;
global_variable RenderWindow renderer_window_memory;

Renderer* renderer_initialize()
{
    Renderer* result = &renderer_memory;
    vkok(volkInitialize());

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


        VkValidationFeatureEnableEXT enabled_validation_features[] =
        {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
        };

        VkDebugUtilsMessengerCreateInfoEXT msg_ci = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = 0,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = 0,
        };

        u8 enable_shader_debug_printf = 0;

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
            .pNext = enable_shader_debug_printf ? (const void*)&validation_features : (const void*)&msg_ci,
        };

        vkok(vkCreateInstance(&ci, result->allocator, &result->instance));
        volkLoadInstance(result->instance);

        VkDebugUtilsMessengerEXT messenger;
        vkok(vkCreateDebugUtilsMessengerEXT(result->instance, &msg_ci, result->allocator, &messenger));
    }


    {
        u32 physical_device_count;
        VkPhysicalDevice physical_devices[256];
        vkok(vkEnumeratePhysicalDevices(result->instance, &physical_device_count, 0));

        if (physical_device_count == 0)
        {
            failed_execution();
        }

        if (physical_device_count > array_length(physical_devices))
        {
            failed_execution();
        }

        vkok(vkEnumeratePhysicalDevices(result->instance, &physical_device_count, physical_devices));

        result->physical_device = physical_devices[0];
    }

    u32 graphics_queue_family_index;
    {
        u32 present_queue_family_index;
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(result->physical_device, &properties);

        u32 queue_count;
        vkGetPhysicalDeviceQueueFamilyProperties(result->physical_device, &queue_count, 0);

        VkQueueFamilyProperties queue_properties[64];
        if (queue_count > array_length(queue_properties))
        {
            failed_execution();
        }

        vkGetPhysicalDeviceQueueFamilyProperties(result->physical_device, &queue_count, queue_properties);

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

        present_queue_family_index = 0;
        // for (present_queue_family_index = 0; present_queue_family_index < queue_count; present_queue_family_index += 1)
        // {
        //     VkBool32 support;
        //     vkok(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, present_queue_family_index, surface, &support));
        //     if (support)
        //     {
        //         break;
        //     }
        // }

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

        vkok(vkCreateDevice(result->physical_device, &ci, result->allocator, &result->device));
    }

    vkGetDeviceQueue(result->device, graphics_queue_family_index, 0, &result->graphics_queue);

    result->immediate.device = result->device;
    result->immediate.queue = result->graphics_queue;

    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = graphics_queue_family_index,
    };
    vkok(vkCreateCommandPool(result->device, &create_info, result->allocator, &result->immediate.command_pool));

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = result->immediate.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkok(vkAllocateCommandBuffers(result->device, &command_buffer_allocate_info, &result->immediate.command_buffer));
    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    vkok(vkCreateFence(result->device, &fence_create_info, result->allocator, &result->immediate.fence));

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

        vkok(vkCreateSampler(result->device, &create_info, result->allocator, &result->sampler));
    }

    return result;
}

RenderWindow* renderer_window_initialize(Renderer* renderer, GraphicsWindow* window)
{
    RenderWindow* result = &renderer_window_memory;
    result->graphics_window = window;
    VkSurfaceKHR surface;
    {
#ifdef VK_USE_PLATFORM_WIN32_KHR
        VkWin32SurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .pNext = 0,
            .flags = 0,
            .hinstance = os_windows_get_module_handle(),
            .hwnd = graphics_win32_window_get(window),
        };
        vkok(vkCreateWin32SurfaceKHR(renderer->instance, &create_info, renderer->allocator, &surface));
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        VkXlibSurfaceCreateInfoKHR create_info = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .pNext = 0,
            .flags = 0,
            .dpy = graphics_x11_display_get(),
            .window = graphics_x11_window_get(window),
        };
        vkok(vkCreateXlibSurfaceKHR(renderer->instance, &create_info, renderer->allocator, &surface));
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
            VK_KHR_COCOA_SURFACE_EXTENSION_NAME,
#endif
    }

    VkSurfaceCapabilitiesKHR original_capabilities;
    vkok(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(renderer->physical_device, surface, &original_capabilities));

    u32 queue_family_indices[] = { renderer->graphics_queue_family_index };
    VkImageUsageFlags swapchain_image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    result->swapchain_image_format = VK_FORMAT_B8G8R8A8_UNORM;
    result->width = original_capabilities.currentExtent.width;
    result->height = original_capabilities.currentExtent.height;
    result->last_width = result->width;
    result->last_height = result->height;
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .surface = surface,
        .minImageCount = original_capabilities.minImageCount,
        .imageFormat = result->swapchain_image_format,
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
        .oldSwapchain = result->swapchain,
    };

    {
        VkSwapchainKHR new_swapchain;
        vkok(vkCreateSwapchainKHR(renderer->device, &swapchain_create_info, renderer->allocator, &new_swapchain));
        result->swapchain = new_swapchain;
    }

    {
        vkok(vkGetSwapchainImagesKHR(renderer->device, result->swapchain, &result->swapchain_image_count, 0));

        if (result->swapchain_image_count == 0)
        {
            failed_execution();
        }

        if (result->swapchain_image_count > array_length(result->swapchain_images))
        {
            failed_execution();
        }

        vkok(vkGetSwapchainImagesKHR(renderer->device, result->swapchain, &result->swapchain_image_count, result->swapchain_images));
        
        // VkImageViewUsageCreateInfo image_view_usage_create_info = {
        //     .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO,
        //     .pNext = 0,
        //     .usage = swapchain_create_info.imageUsage,
        // };

        for (u32 i = 0; i < result->swapchain_image_count; i += 1)
        {
            VkImageViewCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                // .pNext = &image_view_usage_create_info,
                .flags = 0,
                .image = result->swapchain_images[i],
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

            vkok(vkCreateImageView(renderer->device, &create_info, renderer->allocator, &result->swapchain_image_views[i]));
        }
    }

    vkGetPhysicalDeviceMemoryProperties(renderer->physical_device, &renderer->memory_properties);

    auto initial_window_size = graphics_window_size_get(window);

    result->render_image = vk_image_create(renderer->device, renderer->allocator, renderer->memory_properties, (VulkanImageCreate) {
        .width = initial_window_size.width,
        .height = initial_window_size.height,
        .mip_levels = 1,
        .format = result->swapchain_image_format,
        .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    });
    
    VkCommandPoolCreateInfo command_pool_create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = renderer->graphics_queue_family_index,
    };

    VkFenceCreateInfo fence_create_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VkSemaphoreCreateInfo semaphore_create_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .flags = 0,
    };

    for (u32 i = 0; i < MAX_FRAMES; i += 1)
    {
        WindowFrame* frame = &result->frames[i];
        vkok(vkCreateCommandPool(renderer->device, &command_pool_create_info, renderer->allocator, &frame->command_pool));

        VkCommandBufferAllocateInfo command_buffer_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = frame->command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        vkok(vkAllocateCommandBuffers(renderer->device, &command_buffer_allocate_info, &frame->command_buffer));
        vkok(vkCreateFence(renderer->device, &fence_create_info, renderer->allocator, &frame->render_fence));
        vkok(vkCreateSemaphore(renderer->device, &semaphore_create_info, renderer->allocator, &frame->render_semaphore));
        vkok(vkCreateSemaphore(renderer->device, &semaphore_create_info, renderer->allocator, &frame->swapchain_semaphore));
    }
    
    return result;
}

#define MAX_DESCRIPTOR_SETS (16)
#define MAX_PUSH_CONSTANT_RANGES (16)

STRUCT(PipelineLayout)
{
    VkPipelineLayout handle;
    u32 descriptor_set_count;
    VkDescriptorSet descriptor_sets[MAX_DESCRIPTOR_SETS];
    u32 push_constant_range_count;
    VkPushConstantRange push_constant_ranges[MAX_DESCRIPTOR_SETS];
};

STRUCT(Pipeline)
{
    VkPipeline handle;
    PipelineLayoutIndex layout;
};

#define MAX_PIPELINES (16)
global_variable PipelineLayout pipeline_layouts[MAX_PIPELINES];
global_variable Pipeline pipelines[MAX_PIPELINES];

STRUCT(GPUDrawPushConstants)
{
    VkDeviceAddress vertex_buffer;
    f32 width;
    f32 height;
};

#define MAX_SHADER_MODULE_COUNT_PER_PIPELINE (16)
fn VkShaderStageFlags vulkan_shader_stage_from_path(String shader_source_path)
{
    VkShaderStageFlags shader_stage;
    if (string_ends_with(shader_source_path, strlit(".vert")))
    {
        shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
    }
    else if (string_ends_with(shader_source_path, strlit(".frag")))
    {
        shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    else
    {
        failed_execution();
    }

    return shader_stage;
}

fn VkShaderStageFlags vulkan_shader_stage(ShaderStage shader_stage)
{
    VkShaderStageFlags result;

    switch (shader_stage)
    {
    case SHADER_STAGE_VERTEX:
        result = VK_SHADER_STAGE_VERTEX_BIT;
        break;
    case SHADER_STAGE_FRAGMENT:
        result = VK_SHADER_STAGE_FRAGMENT_BIT;
        break;
    }

    return result;
}

fn ShaderStage shader_stage_from_path(String shader_source_path)
{
    ShaderStage shader_stage;
    if (string_ends_with(shader_source_path, strlit(".vert")))
    {
        shader_stage = SHADER_STAGE_VERTEX;
    }
    else if (string_ends_with(shader_source_path, strlit(".frag")))
    {
        shader_stage = SHADER_STAGE_FRAGMENT;
    }
    else
    {
        failed_execution();
    }

    return shader_stage;
}

fn VkDescriptorType vulkan_descriptor_type(DescriptorType type)
{
    VkDescriptorType result;

    switch (type)
    {
        case DESCRIPTOR_TYPE_IMAGE_PLUS_SAMPLER:
            result = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
        case DESCRIPTOR_TYPE_COUNT:
            unreachable();
        }

    return result;
}

PipelineIndex renderer_graphics_pipelines_create(Renderer* renderer, Arena* arena, GraphicsPipelinesCreate create_data)
{
    auto graphics_pipeline_count = create_data.pipelines.length;
    assert(graphics_pipeline_count);
    auto pipeline_layout_count = create_data.layouts.length;
    assert(pipeline_layout_count);
    assert(pipeline_layout_count <= graphics_pipeline_count);
    auto shader_count = create_data.shader_sources.length;

    if (graphics_pipeline_count > MAX_PIPELINES)
    {
        failed_execution();
    }

    VkPipeline pipeline_handles[MAX_PIPELINES];
    VkPipelineShaderStageCreateInfo shader_create_infos[MAX_SHADER_MODULE_COUNT_PER_PIPELINE];
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_infos[MAX_PIPELINES];

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

    // TODO: abstract away
    VkFormat common_image_format = VK_FORMAT_B8G8R8A8_UNORM;
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

    auto* shader_modules = arena_allocate(arena, VkShaderModule, shader_count);

    for (u64 i = 0; i < shader_count; i += 1)
    {
        String shader_source_path = create_data.shader_sources.pointer[i];

        ShaderStage shader_stage = shader_stage_from_path(shader_source_path);

        auto binary = compile_shader(arena, shader_source_path, shader_stage);

        VkShaderModuleCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = binary.length,
            .pCode = (u32*)binary.pointer,
        };

        vkok(vkCreateShaderModule(renderer->device, &create_info, renderer->allocator, &shader_modules[i]));
    }

    // auto* pipeline_layouts = arena_allocate(arena, VkPipelineLayout, pipeline_layout_count);

    for (u64 i = 0; i < pipeline_layout_count; i += 1)
    {
        auto create = create_data.layouts.pointer[i];
        auto descriptor_set_layout_count = create.descriptor_set_layouts.length;
        auto push_constant_range_count = create.push_constant_ranges.length;
        pipeline_layouts[i].descriptor_set_count = descriptor_set_layout_count;
        pipeline_layouts[i].push_constant_range_count = push_constant_range_count;

#define MAX_DESCRIPTOR_SET_LAYOUT_COUNT (16)
        VkDescriptorSetLayout descriptor_set_layouts[MAX_DESCRIPTOR_SET_LAYOUT_COUNT];
        if (descriptor_set_layout_count > MAX_DESCRIPTOR_SET_LAYOUT_COUNT)
        {
            failed_execution();
        }

        u16 descriptor_type_counter[DESCRIPTOR_TYPE_COUNT] = {};

        for (u64 i = 0; i < descriptor_set_layout_count; i += 1)
        {
            auto set_layout_create = create.descriptor_set_layouts.pointer[i];
            auto binding_count = set_layout_create.bindings.length;

#define MAX_DESCRIPTOR_SET_LAYOUT_BINDING_COUNT (16)
            VkDescriptorSetLayoutBinding bindings[MAX_DESCRIPTOR_SET_LAYOUT_BINDING_COUNT];
            
            for (u64 i = 0; i < binding_count; i += 1)
            {
                auto binding_descriptor = set_layout_create.bindings.pointer[i];
                auto* counter_ptr = &descriptor_type_counter[binding_descriptor.type];
                auto old_counter = *counter_ptr;
                *counter_ptr = old_counter + binding_descriptor.count;

                VkDescriptorType descriptor_type = vulkan_descriptor_type(binding_descriptor.type);

                VkShaderStageFlags shader_stage = vulkan_shader_stage(binding_descriptor.stage);

                bindings[i] = (VkDescriptorSetLayoutBinding) {
                    .binding = binding_descriptor.binding,
                    .descriptorType = descriptor_type,
                    .descriptorCount = binding_descriptor.count,
                    .stageFlags = shader_stage,
                    .pImmutableSamplers = 0,
                };
            }

            VkDescriptorSetLayoutCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .bindingCount = binding_count,
                .pBindings = bindings,
            };

            vkok(vkCreateDescriptorSetLayout(renderer->device, &create_info, renderer->allocator, &descriptor_set_layouts[i]));
        }

        {
            VkDescriptorPoolSize pool_sizes[DESCRIPTOR_TYPE_COUNT];
            u32 pool_size_count = 0;

            for (DescriptorType i = 0; i < DESCRIPTOR_TYPE_COUNT; i += 1)
            {
                auto count = descriptor_type_counter[i];
                if (count)
                {
                    auto* pool_size = &pool_sizes[pool_size_count];
                    pool_size_count += 1;

                    *pool_size = (VkDescriptorPoolSize) {
                        .type = vulkan_descriptor_type(i),
                        .descriptorCount = count,
                    };
                }
            }

            VkDescriptorPoolCreateInfo create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .maxSets = descriptor_set_layout_count,
                .poolSizeCount = pool_size_count,
                .pPoolSizes = pool_sizes,
            };

            VkDescriptorPool descriptor_pool;
            vkok(vkCreateDescriptorPool(renderer->device, &create_info, renderer->allocator, &descriptor_pool));

            VkDescriptorSetAllocateInfo allocate_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = 0,
                .descriptorPool = descriptor_pool,
                .descriptorSetCount = descriptor_set_layout_count,
                .pSetLayouts = descriptor_set_layouts,
            };

            vkok(vkAllocateDescriptorSets(renderer->device, &allocate_info, pipeline_layouts[i].descriptor_sets));
        }

        if (push_constant_range_count > MAX_PUSH_CONSTANT_RANGES)
        {
            failed_execution();
        }

        for (u64 i = 0; i < push_constant_range_count; i += 1)
        {
            auto push_constant_descriptor = create.push_constant_ranges.pointer[i];
            pipeline_layouts[i].push_constant_ranges[i] = (VkPushConstantRange) {
                .stageFlags = vulkan_shader_stage(push_constant_descriptor.stage),
                .offset = push_constant_descriptor.offset,
                .size = push_constant_descriptor.size,
            };
        }

        VkPipelineLayoutCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = 0,
            .flags = 0,
            .setLayoutCount = descriptor_set_layout_count,
            .pSetLayouts = descriptor_set_layouts,
            .pushConstantRangeCount = push_constant_range_count,
            .pPushConstantRanges = pipeline_layouts[i].push_constant_ranges,
        };

        vkok(vkCreatePipelineLayout(renderer->device, &create_info, renderer->allocator, &pipeline_layouts[i].handle));
    }

    for (u64 i = 0; i < graphics_pipeline_count; i += 1)
    {
        auto create = create_data.pipelines.pointer[i];
        auto pipeline_shader_count = create.shader_source_indices.length;
        if (pipeline_shader_count > MAX_SHADER_MODULE_COUNT_PER_PIPELINE)
        {
            failed_execution();
        }

        for (u64 i = 0; i < pipeline_shader_count; i += 1)
        {
            auto shader_index = create.shader_source_indices.pointer[i];
            auto shader_source_path = create_data.shader_sources.pointer[shader_index];

            shader_create_infos[i] = (VkPipelineShaderStageCreateInfo) {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = 0,
                .flags = 0,
                .stage = vulkan_shader_stage_from_path(shader_source_path),
                .module = shader_modules[i],
                .pName = "main",
                .pSpecializationInfo = 0,
            };
        }

        graphics_pipeline_create_infos[i] = (VkGraphicsPipelineCreateInfo)
        {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_create_info,
            .flags = 0,
            .stageCount = shader_count,
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
            .layout = pipeline_layouts[create.layout_index].handle,
            .renderPass = 0,
            .subpass = 0,
            .basePipelineHandle = 0,
            .basePipelineIndex = 0,
        };
    }

    VkPipelineCache pipeline_cache = 0;
    vkok(vkCreateGraphicsPipelines(renderer->device, pipeline_cache, graphics_pipeline_count, graphics_pipeline_create_infos, renderer->allocator, pipeline_handles));

    for (u32 i = 0; i < graphics_pipeline_count; i += 1)
    {
        pipelines[i] = (Pipeline) {
            .handle = pipeline_handles[i],
            .layout = { .value = create_data.pipelines.pointer[i].layout_index },
        };
    }

    return (PipelineIndex){ .value = 0 };
}

fn WindowFrame* window_frame(RenderWindow* window)
{
    return &window->frames[window->frame_index % MAX_FRAMES];
}

// TODO: this might be the wrong abstraction layer
fn void queue_submit(Renderer* renderer, RenderWindow* window)
{
    auto* frame = window_frame(window);

    VkCommandBufferSubmitInfo command_buffer_submit_info[] = {
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = frame->command_buffer,
            .deviceMask = 0,
        },
    };

    VkSemaphoreSubmitInfo wait_semaphore_submit_info[] = {
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frame->swapchain_semaphore,
            .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .deviceIndex = 0,
            .value = 1,
        },
    };

    VkSemaphoreSubmitInfo signal_semaphore_submit_info[] = {
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frame->render_semaphore,
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

    vkok(vkQueueSubmit2(renderer->graphics_queue, array_length(submit_info), submit_info, frame->render_fence));
}

fn void queue_present(Renderer* renderer, RenderWindow* window)
{
    auto* frame = window_frame(window);
    const VkSwapchainKHR swapchains[] = { window->swapchain };
    const u32 swapchain_image_indices[] = { window->swapchain_image_index };
    const VkSemaphore wait_semaphores[] = { frame->render_semaphore };
    VkResult results[array_length(swapchains)];

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = array_length(wait_semaphores),
        .pWaitSemaphores = wait_semaphores,
        .swapchainCount = array_length(swapchains),
        .pSwapchains = swapchains,
        .pImageIndices = swapchain_image_indices,
        .pResults = results,
    };

    vkok_swapchain(vkQueuePresentKHR(renderer->graphics_queue, &present_info));

    for (u32 i = 0; i < array_length(results); i += 1)
    {
        vkok_swapchain(results[i]);
    }
}

GraphicsWindowSize renderer_window_frame_begin(Renderer* renderer, RenderWindow* window)
{
    auto* frame = window_frame(window);
    auto timeout = ~(u64)0;

    u32 fence_count = 1;
    VkBool32 wait_all = 1;
    vkok(vkWaitForFences(renderer->device, fence_count, &frame->render_fence, wait_all, timeout));
    VkFence image_fence = 0;
    vkok_swapchain(vkAcquireNextImageKHR(renderer->device, window->swapchain, timeout, frame->swapchain_semaphore, image_fence, &window->swapchain_image_index));
    vkok(vkResetFences(renderer->device, fence_count, &frame->render_fence));

    VkCommandBufferResetFlags reset_flags = 0;
    vkok(vkResetCommandBuffer(frame->command_buffer, reset_flags));

    window->last_width = window->width;
    window->last_height = window->height;
    auto window_size = graphics_window_size_get(window->graphics_window);
    window->width = window_size.width;
    window->height = window_size.height;

    return window_size;
}

void renderer_window_frame_end(Renderer* renderer, RenderWindow* window)
{
    queue_submit(renderer, window);

    queue_present(renderer, window);

    window->frame_index += 1;
}

void window_command_begin(RenderWindow* window)
{
    auto* frame = window_frame(window);
    VkCommandBufferBeginInfo command_buffer_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkok(vkBeginCommandBuffer(frame->command_buffer, &command_buffer_begin_info));
}

void window_command_end(RenderWindow* window)
{
    auto* frame = window_frame(window);
    vkok(vkEndCommandBuffer(frame->command_buffer));
}

void window_render_begin(RenderWindow* window)
{
    auto* frame = window_frame(window);
    vk_image_transition(frame->command_buffer, window->render_image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkViewport viewports[] = {
        {
            .x = 0,
            .y = 0,
            .width = window->width,
            .height = window->height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        }
    };

    u32 first_viewport = 0;
    vkCmdSetViewport(frame->command_buffer, first_viewport, array_length(viewports), viewports);

    VkRect2D scissors[] = {
        {
            .offset = {
                .x = 0,
                .y = 0,
            },
            .extent = {
                .width = window->width,
                .height = window->height,
            },
        }
    };

    u32 first_scissor = 0;
    vkCmdSetScissor(frame->command_buffer, first_scissor, array_length(scissors), scissors);

    VkRenderingAttachmentInfo color_attachments[] = {
        {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = window->render_image.view,
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
                .width = window->width,
                .height = window->height,
            },
        },
        .layerCount = 1,
        .colorAttachmentCount = array_length(color_attachments),
        .pColorAttachments = color_attachments,
    };

    vkCmdBeginRendering(frame->command_buffer, &rendering_info);
}

void window_render_end(RenderWindow* window)
{
    auto* frame = window_frame(window);
    vkCmdEndRendering(frame->command_buffer);

    vk_image_transition(frame->command_buffer, window->render_image.handle, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VkImage swapchain_image = window->swapchain_images[window->swapchain_image_index];
    vk_image_transition(frame->command_buffer, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vk_image_copy(frame->command_buffer, (VulkanCopyImageArgs) {
        .source = {
            .handle = window->render_image.handle,
            .extent = {
                .width = window->width,
                .height = window->height,
            },
        },
        .destination = {
            .handle = swapchain_image,
            .extent = {
                .width = window->width,
                .height = window->height,
            },
        },
    });

    vk_image_transition(frame->command_buffer, swapchain_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

STRUCT(VulkanBuffer)
{
    VkBuffer handle;
    VkDeviceMemory gpu_data;
    u64 address;
    VkDeviceSize size;
};

fn VulkanBuffer vk_buffer_create(VkDevice device, const VkAllocationCallbacks* allocation_callbacks, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkDeviceSize buffer_size, VkBufferUsageFlags usage_flags, VkMemoryPropertyFlags memory_flags)
{
    VulkanBuffer result = {
        .size = buffer_size,
    };

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
    vkok(vkCreateBuffer(device, &create_info, allocation_callbacks, &result.handle));

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device, result.handle, &memory_requirements);

    u8 use_device_address_bit = !!(create_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
    result.gpu_data = vk_allocate_memory(device, allocation_callbacks, physical_device_memory_properties, memory_requirements, memory_flags, use_device_address_bit);

    VkDeviceSize memory_offset = 0;
    vkok(vkBindBufferMemory(device, result.handle, result.gpu_data, memory_offset));

    u8 map_memory = !!(memory_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    assert(((map_memory | use_device_address_bit) == 0) | (map_memory == !use_device_address_bit));
    if (map_memory)
    {
        void* data = 0;
        VkDeviceSize offset = 0;
        VkMemoryMapFlags map_flags = 0;
        vkok(vkMapMemory(device, result.gpu_data, offset, memory_requirements.size, map_flags, &data));
        result.address = (u64)data;
    }

    if (use_device_address_bit)
    {
        VkBufferDeviceAddressInfo device_address_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .pNext = 0,
            .buffer = result.handle,
        };
        result.address = vkGetBufferDeviceAddress(device, &device_address_info);
    }

    return result;
}

#define MAX_TEXTURES (16)
STRUCT(VulkanTexture)
{
    VulkanImage image;
    VkSampler sampler;
};
global_variable VulkanTexture textures[MAX_TEXTURES];
global_variable u32 texture_count;

fn VkFormat vk_texture_format(TextureFormat format)
{
    VkFormat result;
    switch (format)
    {
    case R8_UNORM:
        result = VK_FORMAT_R8_UNORM;
        break;
    case R8G8B8A8_SRGB:
        result = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }

    return result;
}

fn u32 format_channel_count(TextureFormat format)
{
    switch (format)
    {
    case R8_UNORM:
        return 1;
    case R8G8B8A8_SRGB:
        return 4;
    }
}

TextureIndex renderer_texture_create(Renderer* renderer, TextureMemory texture_memory)
{
    assert(texture_memory.depth == 1);

    auto texture_index = texture_count;
    texture_count += 1;
    auto* texture = &textures[texture_index];
    texture->image = vk_image_create(renderer->device, renderer->allocator, renderer->memory_properties, (VulkanImageCreate) {
        .width = texture_memory.width,
        .height = texture_memory.height,
        .mip_levels = 1,
        .format = vk_texture_format(texture_memory.format),
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    });
    texture->sampler = renderer->sampler;

    auto image_size = (u64)texture_memory.depth * texture_memory.width * texture_memory.height * format_channel_count(texture_memory.format);
    VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags buffer_memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    auto transfer_buffer = vk_buffer_create(renderer->device, renderer->allocator, renderer->memory_properties, image_size, buffer_usage_flags, buffer_memory_flags);
    memcpy((void*)transfer_buffer.address, texture_memory.pointer, image_size);

    immediate_start(renderer->immediate);

    vk_image_transition(renderer->immediate.command_buffer, texture->image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

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
            .imageOffset = {
                .x = 0,
                .y = 0,
                .z = 0,
            },
            .imageExtent = {
                .width = texture_memory.width,
                .height = texture_memory.height,
                .depth = texture_memory.depth,
            },
        }
    };

    vkCmdCopyBufferToImage(renderer->immediate.command_buffer, transfer_buffer.handle, texture->image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, array_length(copy_regions), copy_regions);

    vk_image_transition(renderer->immediate.command_buffer, texture->image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    immediate_end(renderer->immediate);

    return (TextureIndex) { .value = texture_index };
}

PipelineLayoutIndex renderer_pipeline_get_layout(PipelineIndex pipeline)
{
    return pipelines[pipeline.value].layout;
}

void renderer_update_pipeline_resources(Renderer* renderer, PipelineIndex pipeline_index, Slice(DescriptorSetUpdate) descriptor_updates)
{
    if (descriptor_updates.length > MAX_DESCRIPTOR_SET_UPDATE_COUNT)
    {
        failed_execution();
    }
    
    Pipeline* pipeline = &pipelines[pipeline_index.value];
    PipelineLayout* layout = &pipeline_layouts[pipeline->layout.value];

    VkWriteDescriptorSet write_descriptor_sets[MAX_DESCRIPTOR_SET_UPDATE_COUNT];
    VkDescriptorImageInfo images[MAX_DESCRIPTOR_SET_UPDATE_COUNT][MAX_TEXTURE_UPDATE_COUNT];

    for (u64 i = 0; i < descriptor_updates.length; i += 1)
    {
        DescriptorSetUpdate* in_update = &descriptor_updates.pointer[i];
        VkWriteDescriptorSet* out_update = &write_descriptor_sets[i];
        *out_update = (VkWriteDescriptorSet)
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = 0,
            .dstSet = layout->descriptor_sets[in_update->set.value],
            .dstBinding = in_update->binding,
            .dstArrayElement = 0,
            .descriptorCount = in_update->descriptor_count,
            .descriptorType = vulkan_descriptor_type(in_update->type),
            .pImageInfo = 0,
            .pBufferInfo = 0,
            .pTexelBufferView = 0,
        };

        switch (in_update->type)
        {
        case DESCRIPTOR_TYPE_IMAGE_PLUS_SAMPLER:
            {
                auto* image_pointer = images[i];
                out_update->pImageInfo = image_pointer;
                for (u32 i = 0; i < in_update->descriptor_count; i += 1)
                {
                    VkDescriptorImageInfo* descriptor_image = &image_pointer[i];
                    VulkanTexture* texture = &textures[in_update->textures[i].value];
                    *descriptor_image = (VkDescriptorImageInfo) {
                        .sampler = texture->sampler,
                        .imageView = texture->image.view,
                        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // TODO: specify
                    };
                }
            } break;
        case DESCRIPTOR_TYPE_COUNT:
            unreachable();
        }
    }

    u32 descriptor_copy_count = 0;
    VkCopyDescriptorSet* descriptor_copies = 0;
    vkUpdateDescriptorSets(renderer->device, descriptor_updates.length, write_descriptor_sets, descriptor_copy_count, descriptor_copies);
}

#define MAX_BUFFER_COUNT (32)
global_variable VulkanBuffer buffers[MAX_BUFFER_COUNT];
global_variable u32 buffer_count;

BufferIndex renderer_buffer_create(Renderer* renderer, u64 size, BufferType type)
{
    auto buffer_index = buffer_count;
    if (unlikely(buffer_index == MAX_BUFFER_COUNT))
    {
        failed_execution();
    }

    u8 is_dst = (type == BUFFER_TYPE_VERTEX) | (type == BUFFER_TYPE_INDEX);
    u8 is_src = type == BUFFER_TYPE_STAGING;

    VkBufferUsageFlags usage = 
        (VK_BUFFER_USAGE_TRANSFER_DST_BIT * is_dst) |
        (VK_BUFFER_USAGE_TRANSFER_SRC_BIT * is_src) |
        ((VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) * (type == BUFFER_TYPE_VERTEX)) |
        (VK_BUFFER_USAGE_INDEX_BUFFER_BIT * (type == BUFFER_TYPE_INDEX));
    VkMemoryPropertyFlags memory_flags =
        (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT * is_dst) |
        ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) * is_src);
    buffers[buffer_index] = vk_buffer_create(renderer->device, renderer->allocator, renderer->memory_properties, size, usage, memory_flags);
    buffer_count = buffer_index + 1;

    return (BufferIndex){ .value = buffer_index };
}

void renderer_copy_to_host(BufferIndex buffer_index, Slice(HostBufferCopy) regions)
{
    VulkanBuffer* buffer = &buffers[buffer_index.value];
    auto* buffer_pointer = (u8*)buffer->address;

    for (u64 i = 0; i < regions.length; i += 1)
    {
        auto region = regions.pointer[i];
        memcpy(buffer_pointer + region.destination_offset, region.source.pointer, region.source.length);
    }
}

void renderer_copy_to_local(Renderer* renderer, Slice(LocalBufferCopy) copies)
{
    immediate_start(renderer->immediate);

    for (u64 i = 0; i < copies.length; i += 1)
    {
        auto copy = copies.pointer[i];
        auto* source_buffer = &buffers[copy.source.value];
        auto* destination_buffer = &buffers[copy.destination.value];

        VkBufferCopy2 buffer_copies[MAX_LOCAL_BUFFER_COPY_COUNT];

        for (u64 i = 0; i < copy.regions.length; i += 1)
        {
            auto copy_region = copy.regions.pointer[i];
            buffer_copies[i] = (VkBufferCopy2) {
                .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
                .pNext = 0,
                .srcOffset = copy_region.source_offset,
                .dstOffset = copy_region.destination_offset,
                .size = copy_region.size,
            };
        }

        VkCopyBufferInfo2 info = {
            .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
            .pNext = 0,
            .srcBuffer = source_buffer->handle,
            .dstBuffer = destination_buffer->handle,
            .regionCount = copy.regions.length,
            .pRegions = buffer_copies,
        };

        vkCmdCopyBuffer2(renderer->immediate.command_buffer, &info);
    }

    immediate_end(renderer->immediate);
}

void window_bind_pipeline(RenderWindow* window, PipelineIndex pipeline_index)
{
    auto* frame = window_frame(window); 
    auto* pipeline = &pipelines[pipeline_index.value];
    vkCmdBindPipeline(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
}

void window_bind_index_buffer(RenderWindow* window, BufferIndex index_buffer, u64 offset, IndexType index_type)
{
    auto* frame = window_frame(window); 
    auto* buffer = &buffers[index_buffer.value];
    VkIndexType vk_index_type;
    switch (index_type)
    {
    case INDEX_TYPE_U16:
        vk_index_type = VK_INDEX_TYPE_UINT16;
        break;
    case INDEX_TYPE_U32:
        vk_index_type = VK_INDEX_TYPE_UINT32;
        break;
    }
    vkCmdBindIndexBuffer(frame->command_buffer, buffer->handle, offset, vk_index_type);
}

void window_bind_pipeline_descriptor_sets(RenderWindow* window, PipelineIndex pipeline_index)
{
    auto* pipeline = &pipelines[pipeline_index.value];
    auto* pipeline_layout = &pipeline_layouts[pipeline->layout.value];
    auto* frame = window_frame(window);

    u32 dynamic_offset_count = 0;
    u32* dynamic_offsets = 0;
    u32 first_set = 0;
    vkCmdBindDescriptorSets(frame->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->handle, first_set, pipeline_layout->descriptor_set_count, pipeline_layout->descriptor_sets, dynamic_offset_count, dynamic_offsets);
}

void window_push_constants(RenderWindow* window, PipelineIndex pipeline_index, SliceP(void) memories)
{
    auto* pipeline = &pipelines[pipeline_index.value];
    auto* pipeline_layout = &pipeline_layouts[pipeline->layout.value];
    auto* frame = window_frame(window);

    if (memories.length != pipeline_layout->push_constant_range_count)
    {
        failed_execution();
    }

    for (u64 i = 0; i < memories.length; i += 1)
    {
        auto* memory = memories.pointer[i];
        auto push_constant_range = pipeline_layout->push_constant_ranges[i];
        vkCmdPushConstants(frame->command_buffer, pipeline_layout->handle, push_constant_range.stageFlags, push_constant_range.offset, push_constant_range.size, memory);
    }
}

u64 buffer_address(BufferIndex buffer_index)
{
    auto* buffer = &buffers[buffer_index.value];
    return buffer->address;
}

void window_draw_indexed(RenderWindow* window, u32 index_count, u32 instance_count, u32 first_index, s32 vertex_offset, u32 first_instance)
{
    auto* frame = window_frame(window);
    vkCmdDrawIndexed(frame->command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}
