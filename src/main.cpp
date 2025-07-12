#include "GlfwGeneral.hpp"
#include "EasyVulkan.hpp"

#define WIDTH 1280
#define HEIGHT 720
int main()
{
	if (!InitializeWindow({ WIDTH, HEIGHT }))
		return -1;
    while (!glfwWindowShouldClose(pWindow)) {

        /*渲染过程，待填充*/

        glfwPollEvents();
        TitleFps();
    }
    TerminateWindow();
    return 0;
}
