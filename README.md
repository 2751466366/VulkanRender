# Description

This project implements IBL (Image-Based Lighting) rendering effects based on Vulkan.

# Running

<img src="./asset/running.png"/>

## Features

- **IBL Rendering**: Implements Image-Based Lighting for realistic lighting effects.
- **Vulkan Graphics Pipeline**: Utilizes Vulkan for efficient and flexible rendering.
- **BVH Acceleration**: Constructs a Bounding Volume Hierarchy using Morton encoding.
- **Swapchain Management**: Handles swapchain creation and destruction with callbacks.


## Key Directories

- **`asset/`**: Contains assets such as textures and images (e.g., `running.png`).
- **`dll/`**: Includes required dynamic libraries (e.g., `glfw3.dll`, `vulkan-1.dll`).
- **`inc/`**: Header files for various components, including rendering, camera, and lighting.
- **`lib/`**: Static libraries used in the project.
- **`shaders/`**: SPIR-V shader files for the Vulkan pipeline.
- **`src/`**: Source code for the project.
- **`x64/`**: Compiled binaries and output files.

## Dependencies

The project relies on the following libraries:

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- [GLFW](https://www.glfw.org/)
- [Assimp](https://github.com/assimp/assimp)


## Build Instructions

1, Open the Solution: Open vsVulkan.sln in Visual Studio.
2, Build the Project: Select the appropriate build configuration (e.g., Debug/Release) and build the solution.