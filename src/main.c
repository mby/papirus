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

VkSwapchainKHR swapChain = NULL;
VkFormat swapChainFormat;
VkExtent2D extent;
VkImage *swapChainImages = NULL;
uint32_t swapChainImageCount = 0;
VkImageView *swapChainImageViews = NULL;
uint32_t swapChainImageViewCount = 0;

void initWindow()
{
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}

	if (!glfwVulkanSupported()) {
		printf("Vulkan is not supported\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(640, 480, "Papirus", NULL, NULL);
	if (!window) {
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
	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// include VK_KHR_PORTABILITY_subset
	const char **extensions =
		calloc(glfwExtensionCount + 1, sizeof(char *));
	int i;
	for (i = 0; i < glfwExtensionCount; i++) {
		extensions[i] = glfwExtensions[i];
	}
	extensions[i++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	createInfo.enabledExtensionCount = glfwExtensionCount + 1;
	createInfo.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateInstance(&createInfo, NULL, &instance);
	if (result != VK_SUCCESS) {
		printf("Failed to create instance: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void createSurface()
{
	VkResult result =
		glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (result != VK_SUCCESS) {
		printf("Failed to create surface: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
	VkPhysicalDevice *devices =
		malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	if (deviceCount == 0) {
		printf("Failed to find GPUs with Vulkan support\n");
		exit(EXIT_FAILURE);
	}

	// TODO: pick the best device
	physicalDevice = devices[0];

	// find queue indices
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
						 &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilies =
		malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		physicalDevice, &queueFamilyCount, queueFamilies);

	for (int i = 0; i < queueFamilyCount; i++) {
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
						     &presentSupport);

		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueIndex = i;
		}

		if (presentSupport) {
			presentQueueIndex = i;
		}

		if (graphicsQueueIndex != -1 && presentQueueIndex != -1) {
			break;
		}
	}

	if (graphicsQueueIndex == -1 || presentQueueIndex == -1) {
		printf("Failed to find a suitable queue families\n");
		exit(EXIT_FAILURE);
	}

	free(queueFamilies);
	free(devices);
}

void createLogicalDevice()
{
	VkDeviceQueueCreateInfo *queueCreateInfos =
		malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
	int queueIndices[] = { graphicsQueueIndex, presentQueueIndex };
	float queuePriority = 1.0f;

	for (int i = 0; i < 2; i++) {
		queueCreateInfos[i].sType =
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
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

	static const char *deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	VkResult result =
		vkCreateDevice(physicalDevice, &createInfo, NULL, &device);
	if (result != VK_SUCCESS) {
		printf("Failed to create logical device: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void createSwapChain()
{
	// choose a surface format
	VkSurfaceFormatKHR surfaceFormat;
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
						     &formatCount, NULL);

		VkSurfaceFormatKHR *formats =
			malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
						     &formatCount, formats);

		for (int i = 0; i < formatCount; i++) {
			if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			    formats[i].colorSpace ==
				    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = formats[i];
				break;
			}
		}

		if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
			printf("Failed to find a suitable surface format\n");
			exit(EXIT_FAILURE);
		}

		free(formats);
		swapChainFormat = surfaceFormat.format;
	}

	// choose a present mode
	VkPresentModeKHR presetMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, surface, &presentModeCount, NULL);

		VkPresentModeKHR *presentModes =
			malloc(sizeof(VkPresentModeKHR) * presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
							  surface,
							  &presentModeCount,
							  presentModes);

		for (int i = 0; i < presentModeCount; i++) {
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				presetMode = presentModes[i];
				break;
			}
		}
	}

	// choose a swap extent
	VkSurfaceCapabilitiesKHR capabilities;
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			physicalDevice, surface, &capabilities);

		if (capabilities.currentExtent.width != UINT32_MAX) {
			extent = capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			extent.width = width;
			extent.height = height;
		}
	}

	// create the swap chain
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = capabilities.minImageCount + 1;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueIndices[] = { graphicsQueueIndex, presentQueueIndex };
	if (graphicsQueueIndex != presentQueueIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presetMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result =
		vkCreateSwapchainKHR(device, &createInfo, NULL, &swapChain);
	if (result != VK_SUCCESS) {
		printf("Failed to create swap chain: %d\n", result);
		exit(EXIT_FAILURE);
	}

	// create the swap chain images
	vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, NULL);
	swapChainImages = malloc(sizeof(VkImage) * swapChainImageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount,
				swapChainImages);
}

void createImageViews()
{
	swapChainImageViews = malloc(sizeof(VkImageView) * swapChainImageCount);

	for (int i = 0; i < swapChainImageCount; i++) {
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask =
			VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(device, &createInfo, NULL,
						    &swapChainImageViews[i]);
		if (result != VK_SUCCESS) {
			printf("Failed to create image view: %d\n", result);
			exit(EXIT_FAILURE);
		}
	}
}

void createRenderPass()
{
	// TODO: figure this out
}

void createGraphicsPipeline()
{
	// TODO: figure this out
}

void initVulkan()
{
	createInstance();
	createSurface();

	pickPhysicalDevice();
	createLogicalDevice();

	createSwapChain();
	createImageViews();

	createRenderPass();
	createGraphicsPipeline();
}

int main()
{
	initWindow();
	initVulkan();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	for (int i = 0; i < swapChainImageCount; i++) {
		vkDestroyImageView(device, swapChainImageViews[i], NULL);
	}
	free(swapChainImageViews);
	free(swapChainImages);

	vkDestroySwapchainKHR(device, swapChain, NULL);
	vkDestroyDevice(device, NULL);
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
