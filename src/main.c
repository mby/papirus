#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GLFWwindow *window = NULL;

VkInstance instance = NULL;

VkSurfaceKHR surface = NULL;

VkPhysicalDevice physicalDevice = NULL;
int graphicsQueueIndex = -1, presentQueueIndex = -1;

VkDevice device = NULL;

void initWindow()
{
    if (!glfwInit())
    {
        printf("Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    if (!glfwVulkanSupported())
    {
        printf("Vulkan is not supported\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(640, 480, "Papirus", NULL, NULL);
    if (!window)
    {
        printf("Failed to create window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
}

void createInstance()
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Papirus";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // include VK_KHR_PORTABILITY_subset
    const char **extensions = calloc(glfwExtensionCount + 1, sizeof(char *));
    int i;
    for (i = 0; i < glfwExtensionCount; i++)
    {
        extensions[i] = glfwExtensions[i];
    }
    extensions[i++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    createInfo.enabledExtensionCount = glfwExtensionCount + 1;
    createInfo.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        printf("Failed to create instance: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

void createSurface()
{
    VkResult result = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if (result != VK_SUCCESS)
    {
        printf("Failed to create surface: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

void pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

    if (deviceCount == 0)
    {
        printf("Failed to find GPUs with Vulkan support\n");
        exit(EXIT_FAILURE);
    }

    // TODO: pick the best device
    physicalDevice = devices[0];

    // find queue indices
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties *queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++)
    {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueIndex = i;
        }

        if (presentSupport)
        {
            presentQueueIndex = i;
        }

        if (graphicsQueueIndex != -1 && presentQueueIndex != -1)
        {
            break;
        }
    }

    if (graphicsQueueIndex == -1 || presentQueueIndex == -1)
    {
        printf("Failed to find a suitable queue families\n");
        exit(EXIT_FAILURE);
    }

    free(queueFamilies);
    free(devices);
}

void createLogicalDevice()
{
    VkDeviceQueueCreateInfo *queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
    int queueIndices[] = {graphicsQueueIndex, presentQueueIndex};
    float queuePriority = 1.0f;

    for (int i = 0; i < 2; i++)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    // TODO
    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 2;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.pEnabledFeatures = &deviceFeatures;

    VkResult result = vkCreateDevice(physicalDevice, &createInfo, NULL, &device);
    if (result != VK_SUCCESS)
    {
        printf("Failed to create logical device: %d\n", result);
        exit(EXIT_FAILURE);
    }
}

void initVulkan()
{
    createInstance();
    createSurface();

    pickPhysicalDevice();
    createLogicalDevice();
}

int main()
{
    initWindow();
    initVulkan();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
