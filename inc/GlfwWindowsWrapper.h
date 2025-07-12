#pragma once
#include "GraphicsBase.h"
#include <GLFW/glfw3.h>

class GlfwWindowsWrapper {
public:
	GLFWwindow* pWindow;
	GLFWmonitor* pMonitor;
	const char* windowTitle = "GlfwWindowsWrapper";

	GlfwWindowsWrapper() = default;
	~GlfwWindowsWrapper() = default;
	static GlfwWindowsWrapper* GetInstance() {
		static GlfwWindowsWrapper instance;
		return &instance;
	}

	bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true);
	void TitleFps();
	void TerminateWindow();
};