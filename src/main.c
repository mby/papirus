#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

int main()
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
    GLFWwindow *window = glfwCreateWindow(640, 480, "Papirus", NULL, NULL);
    if (!window)
    {
        printf("Failed to create window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
