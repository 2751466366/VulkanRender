#include "GlfwWindowsWrapper.h"

bool GlfwWindowsWrapper::InitializeWindow(
	VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate)
{
	if (!glfwInit()) {
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
		return false;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, isResizable);

	//pWindow = glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
	pMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);

	pWindow = fullScreen ?
		glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr) :
		glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);
	if(!pWindow) {
		std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
		glfwTerminate();
		return false;
	}

	uint32_t extensionCount = 0;
	const char** extensionNames;
	extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
	if (!extensionNames) {
		std::cout << std::format("[ InitializeWindow ]\nVulkan is not available on this machine!\n");
		glfwTerminate();
		return false;
	}
	for (size_t i = 0; i < extensionCount; i++)
		GraphicsBase::Base()->AddInstanceExtension(extensionNames[i]);

	GraphicsBase::Base()->AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	GraphicsBase::Base()->UseLatestApiVersion();
	if (GraphicsBase::Base()->CreateInstance())
		return false;

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (VkResult result = glfwCreateWindowSurface(GraphicsBase::Base()->Instance(), pWindow, nullptr, &surface)) {
		std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
		glfwTerminate();
		return false;
	}
	GraphicsBase::Base()->Surface(surface);

	if (//获取物理设备，并使用列表中的第一个物理设备，这里不考虑以下任意函数失败后更换物理设备的情况
		GraphicsBase::Base()->GetPhysicalDevices() ||
		//一个true一个false，暂时不需要计算用的队列
		GraphicsBase::Base()->DeterminePhysicalDevice(0, true, false) ||
		//创建逻辑设备
		GraphicsBase::Base()->CreateDevice())
		return false;

	if (GraphicsBase::Base()->CreateSwapchain(size, limitFrameRate))
		return false;

	return true;
}

void GlfwWindowsWrapper::TitleFps()
{
	static double time0 = glfwGetTime();
	static double time1;
	static double dt;
	static int dframe = -1;
	static std::stringstream info;
	time1 = glfwGetTime();
	dframe++;
	if ((dt = time1 - time0) >= 1) {
		info.precision(1);
		info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
		glfwSetWindowTitle(pWindow, info.str().c_str());
		info.str("");//别忘了在设置完窗口标题后清空所用的stringstream
		time0 = time1;
		dframe = 0;
	}
}

void GlfwWindowsWrapper::TerminateWindow()
{
	GraphicsBase::Base()->WaitIdle();
	glfwTerminate();
}