#pragma once
#include <vulkan/vulkan.h>
#include "Common.h"

#define DestroyHandleBy(Func) if (handle) { Func(GraphicsBase::Base()->Device(), handle, nullptr); handle = VK_NULL_HANDLE; }

class GraphicsBase {
private:
	uint32_t apiVersion = VK_API_VERSION_1_0;

	VkInstance instance;
	VkSurfaceKHR surface;

	std::vector<const char*> instanceLayers;
	std::vector<const char*> instanceExtensions;

	VkDebugUtilsMessengerEXT debugMessenger;

	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
	std::vector<VkPhysicalDevice> availablePhysicalDevices;

	VkDevice device;
	uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
	uint32_t queueFamilyIndex_presentation = VK_QUEUE_FAMILY_IGNORED;
	uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
	VkQueue queue_graphics;
	VkQueue queue_presentation;
	VkQueue queue_compute;
	std::vector<const char*> deviceExtensions;
	std::vector<void(*)()> callbacks_createDevice;
	std::vector<void(*)()> callbacks_destroyDevice;

	std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;
	VkSwapchainKHR swapchain;
	std::vector <VkImage> swapchainImages;
	std::vector <VkImageView> swapchainImageViews;
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	std::vector<void(*)()> callbacks_createSwapchain;
	std::vector<void(*)()> callbacks_destroySwapchain;
	uint32_t currentImageIndex = 0;

	VkResult CreateDebugMessenger();
public:
	GraphicsBase() = default;
	GraphicsBase(GraphicsBase&&) = delete;
	~GraphicsBase();
	static GraphicsBase* Base() {
		static GraphicsBase base;
		return &base;
	}

	//Getter
	uint32_t ApiVersion() const {
		return apiVersion;
	}
	VkInstance Instance() const {
		return instance;
	}
	const std::vector<const char*>& InstanceLayers() const {
		return instanceLayers;
	}
	const std::vector<const char*>& InstanceExtensions() const {
		return instanceExtensions;
	}
	void InstanceLayers(const std::vector<const char*>& layerNames) {
		instanceLayers = layerNames;
	}
	void InstanceExtensions(const std::vector<const char*>& extensionNames) {
		instanceExtensions = extensionNames;
	}

	VkSurfaceKHR Surface() const {
		return surface;
	}
	void Surface(VkSurfaceKHR surface) {
		if (!this->surface)
			this->surface = surface;
	}

	VkPhysicalDevice PhysicalDevice() const {
		return physicalDevice;
	}
	const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const {
		return physicalDeviceProperties;
	}
	const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties() const {
		return physicalDeviceMemoryProperties;
	}
	VkPhysicalDevice AvailablePhysicalDevice(uint32_t index) const {
		return availablePhysicalDevices[index];
	}
	uint32_t AvailablePhysicalDeviceCount() const {
		return uint32_t(availablePhysicalDevices.size());
	}
	VkDevice Device() const {
		return device;
	}
	uint32_t QueueFamilyIndex_Graphics() const {
		return queueFamilyIndex_graphics;
	}
	uint32_t QueueFamilyIndex_Presentation() const {
		return queueFamilyIndex_presentation;
	}
	uint32_t QueueFamilyIndex_Compute() const {
		return queueFamilyIndex_compute;
	}
	VkQueue Queue_Graphics() const {
		return queue_graphics;
	}
	VkQueue Queue_Presentation() const {
		return queue_presentation;
	}
	VkQueue Queue_Compute() const {
		return queue_compute;
	}
	const std::vector<const char*>& DeviceExtensions() const {
		return deviceExtensions;
	}
	void DeviceExtensions(const std::vector<const char*>& extensionNames) {
		deviceExtensions = extensionNames;
	}

	const VkFormat& AvailableSurfaceFormat(uint32_t index) const {
		return availableSurfaceFormats[index].format;
	}
	const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index) const {
		return availableSurfaceFormats[index].colorSpace;
	}
	uint32_t AvailableSurfaceFormatCount() const {
		return uint32_t(availableSurfaceFormats.size());
	}
	VkSwapchainKHR Swapchain() const {
		return swapchain;
	}
	VkImage SwapchainImage(uint32_t index) const {
		return swapchainImages[index];
	}
	VkImageView SwapchainImageView(uint32_t index) const {
		return swapchainImageViews[index];
	}
	uint32_t SwapchainImageCount() const {
		return uint32_t(swapchainImages.size());
	}
	const VkSwapchainCreateInfoKHR& SwapchainCreateInfo() const {
		return swapchainCreateInfo;
	}
	
	void AddCallback_CreateDevice(void(*function)()) {
		callbacks_createDevice.push_back(function);
	}
	void AddCallback_DestroyDevice(void(*function)()) {
		callbacks_destroyDevice.push_back(function);
	}

	void AddCallback_CreateSwapchain(void(*function)()) {
		callbacks_createSwapchain.push_back(function);
	}
	void AddCallback_DestroySwapchain(void(*function)()) {
		callbacks_destroySwapchain.push_back(function);
	}

	uint32_t CurrentImageIndex() const {
		return currentImageIndex;
	}


	static void AddLayerOrExtension(std::vector<const char*>& container, const char* name);
	VkResult UseLatestApiVersion();
	void AddInstanceLayer(const char* layerName);
	void AddInstanceExtension(const char* extensionName);
	void AddDeviceExtension(const char* extensionName);
	VkResult CreateInstance(VkInstanceCreateFlags flags = 0);
	VkResult CheckInstanceLayers(std::span<const char*> layersToCheck);
	VkResult CheckInstanceExtensions(std::span<const char*> extensionsToCheck, const char* layerName = nullptr);

	VkResult GetPhysicalDevices();
	VkResult GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, bool enableGraphicsQueue, bool enableComputeQueue, uint32_t(&queueFamilyIndices)[3]);
	VkResult DeterminePhysicalDevice(uint32_t deviceIndex = 0, bool enableGraphicsQueue = true, bool enableComputeQueue = true);
	VkResult CreateDevice(VkDeviceCreateFlags flags = 0);
	VkResult RecreateDevice(VkDeviceCreateFlags flags = 0);
	VkResult WaitIdle();

	VkResult CreateSwapchain(VkExtent2D size, bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0);
	VkResult RecreateSwapchain();
	VkResult GetSurfaceFormats();
	VkResult SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat);
	VkResult CreateSwapchain_Internal();
	VkResult SwapImage(VkSemaphore semaphore_imageIsAvailable);

	VkResult SubmitCommandBuffer_Graphics(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE) const;
	VkResult SubmitCommandBuffer_Graphics(
		VkCommandBuffer commandBuffer,
		VkSemaphore semaphore_imageIsAvailable = VK_NULL_HANDLE,
		VkSemaphore semaphore_renderingIsOver = VK_NULL_HANDLE,
		VkFence fence = VK_NULL_HANDLE,
		VkPipelineStageFlags waitDstStage_imageIsAvailable = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT) const;
	VkResult SubmitCommandBuffer_Graphics(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE) const;
	VkResult SubmitCommandBuffer_Compute(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE) const;
	VkResult SubmitCommandBuffer_Compute(VkCommandBuffer commandBuffer, VkFence fence = VK_NULL_HANDLE) const;
};