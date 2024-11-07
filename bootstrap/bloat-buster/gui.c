#include <bloat-buster/gui.h>
#if BB_CI == 0

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <volk.h>

fn u8 vk_layer_is_supported(String layer_name)
{
    assert(layer_name.pointer[layer_name.length] == 0);

    VkLayerProperties layers[256];
    u32 layer_count;

    VkResult result = vkEnumerateInstanceLayerProperties(&layer_count, 0);

    if (result != VK_SUCCESS)
    {
        failed_execution();
    }

    if (layer_count > array_length(layers))
    {
        failed_execution();
    }

    result = vkEnumerateInstanceLayerProperties(&layer_count, layers);

    if (result != VK_SUCCESS)
    {
        failed_execution();
    }

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

fn VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    unused(message_severity);
    unused(message_type);
    unused(user_data);
    print("Validation message ({cstr}): {cstr}\n", callback_data->pMessageIdName, callback_data->pMessage);

    return VK_FALSE;
}

#define frame_overlap (2)

fn void vk_transition_image(VkCommandBuffer command_buffer, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout)
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

    {
        VkResult result = volkInitialize();
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }
    }

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

        auto debug_utils_extension = strlit(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        const char* extensions[] = {
            string_to_c(debug_utils_extension),
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
        };

        VkDebugUtilsMessengerCreateInfoEXT msg_ci = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
            .pfnUserCallback = debug_callback,
            .pUserData = 0,
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
            .pNext = &msg_ci,
        };

        VkResult result = vkCreateInstance(&ci, allocation_callbacks, &instance);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }

        volkLoadInstance(instance);
        VkDebugUtilsMessengerEXT messenger;
        if (vkCreateDebugUtilsMessengerEXT(instance, &msg_ci, allocation_callbacks, &messenger) != VK_SUCCESS)
        {
            failed_execution();
        }
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
        VkXlibSurfaceCreateInfoKHR ci = {
            .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
            .dpy = glfwGetX11Display(),
            .window = glfwGetX11Window(window),
        };
        VkResult result = vkCreateXlibSurfaceKHR(instance, &ci, allocation_callbacks, &surface);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }
    }

    VkPhysicalDevice physical_device;
    {
        u32 physical_device_count;
        VkPhysicalDevice physical_devices[256];
        if (vkEnumeratePhysicalDevices(instance, &physical_device_count, 0) != VK_SUCCESS)
        {
            failed_execution();
        }

        if (physical_device_count == 0)
        {
            failed_execution();
        }

        if (physical_device_count > array_length(physical_devices))
        {
            failed_execution();
        }

        if (vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices) != VK_SUCCESS)
        {
            failed_execution();
        }

        physical_device = physical_devices[0];
    }

    VkDevice device;
    u32 graphics_queue_family_index;
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

        VkResult result = vkCreateDevice(physical_device, &ci, allocation_callbacks, &device);

        if (result != VK_SUCCESS)
        {
            failed_execution();
        }
    }

    VkSurfaceCapabilitiesKHR original_capabilities;
    {
        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &original_capabilities);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }
    }

    VkSwapchainKHR swapchain = 0;
    VkSwapchainCreateInfoKHR swapchain_create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = 0,
        .flags = 0,
        .surface = surface,
        .minImageCount = original_capabilities.minImageCount,
        .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
        .imageColorSpace = 0,
        .imageExtent = original_capabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = 0,
        .preTransform = original_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = 0,
        .oldSwapchain = swapchain,
    };

    {

        VkSwapchainKHR new_swapchain;
        VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, allocation_callbacks, &new_swapchain);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }

        swapchain = new_swapchain;
    }

    VkImage swapchain_image_buffer[16];
    u32 swapchain_image_count;
    {
        VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, 0);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }

        if (swapchain_image_count == 0)
        {
            failed_execution();
        }

        if (swapchain_image_count > array_length(swapchain_image_buffer))
        {
            failed_execution();
        }

        result = vkGetSwapchainImagesKHR(device, swapchain, &swapchain_image_count, swapchain_image_buffer);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }
    }

    VkCommandPool command_pools[frame_overlap];
    VkCommandBuffer command_buffers[frame_overlap];
    {
        VkCommandPoolCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphics_queue_family_index,
        };

        for (u32 i = 0; i < frame_overlap; i += 1)
        {
            VkResult result = vkCreateCommandPool(device, &create_info, allocation_callbacks, &command_pools[i]);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }

            VkCommandBufferAllocateInfo command_buffer_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = command_pools[i],
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &command_buffers[i]);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }
        }
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
            VkResult result = vkCreateFence(device, &fence_create_info, allocation_callbacks, &render_fences[i]);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }

            result = vkCreateSemaphore(device, &semaphore_create_info, allocation_callbacks, &render_semaphores[i]);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }

            result = vkCreateSemaphore(device, &semaphore_create_info, allocation_callbacks, &swapchain_semaphores[i]);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }
        }
    }

    VkQueue graphics_queue;
    vkGetDeviceQueue(device, graphics_queue_family_index, 0, &graphics_queue);

    for (u32 frame_number = 0; !glfwWindowShouldClose(window); frame_number += 1)
    {
        glfwPollEvents();

        int width;
        int height;
        glfwGetWindowSize(window, &width, &height);

        if (width == 0 || height == 0)
        {
            continue;
        }

        u32 frame_index = frame_number % frame_overlap;
        u32 swapchain_image_index;

        {
            auto timeout = ~(u64)0;

            {
                u32 fence_count = 1;
                {
                    VkBool32 wait_all = 1;
                    VkResult result = vkWaitForFences(device, fence_count, &render_fences[frame_index], wait_all, timeout);
                    if (result != VK_SUCCESS)
                    {
                        failed_execution();
                    }
                }

                {
                    VkResult result = vkResetFences(device, fence_count, &render_fences[frame_index]);
                    if (result != VK_SUCCESS)
                    {
                        failed_execution();
                    }
                }
            }

            {
                VkSurfaceCapabilitiesKHR capabilities;
                {
                    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);
                    if (result != VK_SUCCESS)
                    {
                        failed_execution();
                    }
                }

                assert(capabilities.minImageCount ==          original_capabilities.minImageCount);
                assert(capabilities.maxImageCount ==          original_capabilities.maxImageCount);
                assert(capabilities.currentExtent.width == original_capabilities.currentExtent.width);
                assert(capabilities.currentExtent.height == original_capabilities.currentExtent.height);
                assert(capabilities.minImageExtent.width ==         original_capabilities.minImageExtent.width);
                assert(capabilities.minImageExtent.height ==         original_capabilities.minImageExtent.height);
                assert(capabilities.maxImageExtent.width ==         original_capabilities.maxImageExtent.width);
                assert(capabilities.maxImageExtent.height ==         original_capabilities.maxImageExtent.height);
                assert(capabilities.maxImageArrayLayers ==    original_capabilities.maxImageArrayLayers);
                assert(capabilities.supportedTransforms ==    original_capabilities.supportedTransforms);
                assert(capabilities.currentTransform ==       original_capabilities.currentTransform);
                assert(capabilities.supportedCompositeAlpha ==original_capabilities.supportedCompositeAlpha);
                assert(capabilities.supportedUsageFlags ==    original_capabilities.supportedUsageFlags)

                assert(capabilities.currentExtent.width == initial_width);
                assert(capabilities.currentExtent.height == initial_height);
                assert(capabilities.currentTransform == swapchain_create_info.preTransform);

                print("Window size: {u32}x{u32}\nCapabilities: {u32}x{u32}\n", width, height, capabilities.currentExtent.width, capabilities.currentExtent.height);

                VkFence image_fence = 0;
                VkResult result = vkAcquireNextImageKHR(device, swapchain, timeout, swapchain_semaphores[frame_index], image_fence, &swapchain_image_index);
                // if (result != VK_SUBOPTIMAL_KHR)
                // {
                print("Acquire result: {u32}\n", result);
                if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
                {
                    failed_execution();
                }
                // }
            }
        }

        VkCommandBuffer command_buffer = command_buffers[frame_index];

        VkCommandBufferResetFlags reset_flags = 0;
        VkResult result = vkResetCommandBuffer(command_buffer, reset_flags);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }

        // Record the commands
        {
            {
                VkCommandBufferBeginInfo command_buffer_begin_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                };
                VkResult result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
                if (result != VK_SUCCESS)
                {
                    failed_execution();
                }
            }

            VkImage swapchain_image = swapchain_image_buffer[swapchain_image_index];
            vk_transition_image(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

            VkClearColorValue color = { .float32 = { 255.0f, 255.0f, 255.0f, 1.0f } };
            VkImageSubresourceRange clear_ranges[] = {
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = VK_REMAINING_MIP_LEVELS,
                    .baseArrayLayer = 0,
                    .layerCount = VK_REMAINING_ARRAY_LAYERS,
                },
            };

            vkCmdClearColorImage(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, &color, array_length(clear_ranges), clear_ranges);

            vk_transition_image(command_buffer, swapchain_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

            VkResult result = vkEndCommandBuffer(command_buffer);
            if (result != VK_SUCCESS)
            {
                failed_execution();
            }
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
        result = vkQueueSubmit2(graphics_queue, array_length(submit_info), submit_info, render_fence);
        if (result != VK_SUCCESS)
        {
            failed_execution();
        }

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

        result = vkQueuePresentKHR(graphics_queue, &present_info);
        print("Present result: {u32}\n", result);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            failed_execution();
        }
        // }
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
