#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct ppr_App {
	GLFWwindow *window;

	VkInstance instance;

	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice;
	int graphicsQueueIndex, presentQueueIndex;

	VkDevice device;

	VkSwapchainKHR swapChain;
	VkFormat swapChainFormat;
	VkExtent2D extent;
	VkImage *swapChainImages;
	uint32_t swapChainImageCount;
	VkImageView *swapChainImageViews;
	uint32_t swapChainImageViewCount;
};

static void initWindow(ppr_App app, const char *title, int width, int height)
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
	app->window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!app->window) {
		printf("Failed to create window\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
}

void createInstance(ppr_App app)
{
	VkApplicationInfo appInfo = { 0 };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Papirus";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions =
		glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	// include VK_KHR_PORTABILITY_subset
	const int extensionsCount = glfwExtensionCount + 1;
	const char **extensions = calloc(extensionsCount, sizeof(char *));
	int i;
	for (i = 0; i < glfwExtensionCount; i++) {
		extensions[i] = glfwExtensions[i];
	}

#ifdef __APPLE__
	extensions[i++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

	createInfo.enabledExtensionCount = i;
	createInfo.ppEnabledExtensionNames = extensions;

	VkResult result = vkCreateInstance(&createInfo, NULL, &app->instance);
	if (result != VK_SUCCESS) {
		printf("Failed to create instance: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void createSurface(ppr_App app)
{
	VkResult result = glfwCreateWindowSurface(app->instance, app->window,
						  NULL, &app->surface);
	if (result != VK_SUCCESS) {
		printf("Failed to create surface: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void pickPhysicalDevice(ppr_App app)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(app->instance, &deviceCount, NULL);
	VkPhysicalDevice *devices =
		malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices);

	if (deviceCount == 0) {
		printf("Failed to find GPUs with Vulkan support\n");
		exit(EXIT_FAILURE);
	}

	// TODO: pick the best device
	app->physicalDevice = devices[0];

	// find queue indices
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(app->physicalDevice,
						 &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilies =
		malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(
		app->physicalDevice, &queueFamilyCount, queueFamilies);

	for (int i = 0; i < queueFamilyCount; i++) {
		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			app->physicalDevice, i, app->surface, &presentSupport);

		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			app->graphicsQueueIndex = i;
		}

		if (presentSupport) {
			app->presentQueueIndex = i;
		}

		if (app->graphicsQueueIndex != -1 &&
		    app->presentQueueIndex != -1) {
			break;
		}
	}

	if (app->graphicsQueueIndex == -1 || app->presentQueueIndex == -1) {
		printf("Failed to find a suitable queue families\n");
		exit(EXIT_FAILURE);
	}

	free(queueFamilies);
	free(devices);
}

void createLogicalDevice(ppr_App app)
{
	VkDeviceQueueCreateInfo *queueCreateInfos =
		malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
	int queueIndices[] = { app->graphicsQueueIndex,
			       app->presentQueueIndex };
	float queuePriority = 1.0f;

	for (int i = 0; i < 2; i++) {
		queueCreateInfos[i] = (VkDeviceQueueCreateInfo){ 0 };
		queueCreateInfos[i].sType =
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[i].queueFamilyIndex = queueIndices[i];
		queueCreateInfos[i].queueCount = 1;
		queueCreateInfos[i].pQueuePriorities = &queuePriority;
	}

	// TODO
	VkPhysicalDeviceFeatures deviceFeatures = { 0 };

	VkDeviceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = 2;
	createInfo.pQueueCreateInfos = queueCreateInfos;
	createInfo.pEnabledFeatures = &deviceFeatures;

	static const char *deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	VkResult result = vkCreateDevice(app->physicalDevice, &createInfo, NULL,
					 &app->device);
	if (result != VK_SUCCESS) {
		printf("Failed to create logical device: %d\n", result);
		exit(EXIT_FAILURE);
	}
}

void createSwapChain(ppr_App app)
{
	// choose a surface format
	VkSurfaceFormatKHR surfaceFormat;
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			app->physicalDevice, app->surface, &formatCount, NULL);

		VkSurfaceFormatKHR *formats =
			malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(app->physicalDevice,
						     app->surface, &formatCount,
						     formats);

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
		app->swapChainFormat = surfaceFormat.format;
	}

	// choose a present mode
	VkPresentModeKHR presetMode = VK_PRESENT_MODE_FIFO_KHR;
	{
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(app->physicalDevice,
							  app->surface,
							  &presentModeCount,
							  NULL);

		VkPresentModeKHR *presentModes =
			malloc(sizeof(VkPresentModeKHR) * presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(app->physicalDevice,
							  app->surface,
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
			app->physicalDevice, app->surface, &capabilities);

		if (capabilities.currentExtent.width != UINT32_MAX) {
			app->extent = capabilities.currentExtent;
		} else {
			int width, height;
			glfwGetFramebufferSize(app->window, &width, &height);

			app->extent.width = width;
			app->extent.height = height;
		}
	}

	// create the swap chain
	VkSwapchainCreateInfoKHR createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = app->surface;
	createInfo.minImageCount = capabilities.minImageCount + 1;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = app->extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueIndices[] = { app->graphicsQueueIndex,
				    app->presentQueueIndex };
	if (app->graphicsQueueIndex != app->presentQueueIndex) {
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

	VkResult result = vkCreateSwapchainKHR(app->device, &createInfo, NULL,
					       &app->swapChain);
	if (result != VK_SUCCESS) {
		printf("Failed to create swap chain: %d\n", result);
		exit(EXIT_FAILURE);
	}

	// create the swap chain images
	vkGetSwapchainImagesKHR(app->device, app->swapChain,
				&app->swapChainImageCount, NULL);
	app->swapChainImages =
		malloc(sizeof(VkImage) * app->swapChainImageCount);
	vkGetSwapchainImagesKHR(app->device, app->swapChain,
				&app->swapChainImageCount,
				app->swapChainImages);
}

void createImageViews(ppr_App app)
{
	app->swapChainImageViews =
		malloc(sizeof(VkImageView) * app->swapChainImageCount);

	for (int i = 0; i < app->swapChainImageCount; i++) {
		VkImageViewCreateInfo createInfo = { 0 };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = app->swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = app->swapChainFormat;
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

		VkResult result =
			vkCreateImageView(app->device, &createInfo, NULL,
					  &app->swapChainImageViews[i]);
		if (result != VK_SUCCESS) {
			printf("Failed to create image view: %d\n", result);
			exit(EXIT_FAILURE);
		}
	}
}

static void createRenderPass(ppr_App app)
{
	// TODO: figure this out
}

static void createGraphicsPipeline(ppr_App app)
{
	// TODO: figure this out
}

static void initVulkan(ppr_App app)
{
	createInstance(app);
	createSurface(app);

	pickPhysicalDevice(app);
	createLogicalDevice(app);

	createSwapChain(app);
	createImageViews(app);

	createRenderPass(app);
	createGraphicsPipeline(app);
}

ppr_App ppr_new(const char *title, int width, int height)
{
	ppr_App app = malloc(sizeof(struct ppr_App));
	memset(app, 0, sizeof(struct ppr_App));

	initWindow(app, title, width, height);
	initVulkan(app);

	return app;
}

void ppr_free(ppr_App app)
{
	for (int i = 0; i < app->swapChainImageCount; i++) {
		vkDestroyImageView(app->device, app->swapChainImageViews[i],
				   NULL);
	}
	free(app->swapChainImageViews);
	free(app->swapChainImages);

	vkDestroySwapchainKHR(app->device, app->swapChain, NULL);
	vkDestroyDevice(app->device, NULL);
	vkDestroySurfaceKHR(app->instance, app->surface, NULL);
	vkDestroyInstance(app->instance, NULL);
	glfwDestroyWindow(app->window);
	glfwTerminate();
}

int ppr_isRunning(ppr_App app)
{
	return !glfwWindowShouldClose(app->window);
}

void ppr_pollEvents(ppr_App app)
{
	glfwPollEvents();
}
