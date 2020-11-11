#include <iostream>

#include <glm/mat4x4.hpp>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES // additionally required for mingw-w64 https://github.com/vurtun/nuklear/issues/770
#include "nuklear.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h> // should be before GLFW include
#include <GLFW/glfw3.h>

int main()
{
    // todo remove GLFW and glad test code
    GLFWwindow *window_handle = NULL;

    if (glfwInit() == GLFW_FALSE) {
        return EXIT_FAILURE;
    }

    if ((window_handle = glfwCreateWindow(800, 600, "", NULL, NULL)) == NULL) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window_handle);

    if (gladLoadGL() == 0) {
        glfwDestroyWindow(window_handle);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwDestroyWindow(window_handle);
    glfwTerminate();
    return EXIT_SUCCESS;
}
