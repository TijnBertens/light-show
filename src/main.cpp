#include <iostream>

#include <glm/mat4x4.hpp>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES // additionally required for mingw-w64 https://github.com/vurtun/nuklear/issues/770

#include "nuklear.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include <glad/glad.h> // should be before GLFW include
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>

#include "system/graphics.hpp"
#include "util/ls_log.hpp"
#include "system/camera.hpp"
#include "system/window.hpp"

void update(Window *window, Camera *camera);

void render(Window *window, Camera *camera, AssetID shader_id, AssetID model_id);

int main()
{
    Window window(800, 600, "");

    GraphicsManager graphics_manager;
    window.getRenderer()->setGraphicsManager(&graphics_manager);

    AssetManager asset_manager;

    AssetID model_id = asset_manager.loadObj(
            std::string("../res/obj/Chandelier_03"),
            std::string("Chandelier_03.obj"));
    Model *chandelier = asset_manager.getModel(model_id);
    graphics_manager.loadModel(chandelier);

    ls_log::log(LOG_INFO, "done loading model!\n");

    AssetID shader_id = asset_manager.loadShader(
            std::string("../res/shader/pbr.vert"),
            std::string("../res/shader/pbr.frag"));
    Shader *shader = asset_manager.getShader(shader_id);
    graphics_manager.loadShader(shader);

    Camera camera(
            (float) window.get_input_handler()->get_size_x() / (float) window.get_input_handler()->get_size_y(),
            glm::radians(70.f), .1f, 100.f);

    while (!window.shouldClose()) {
        window.get_input_handler()->pull_input();
        update(&window, &camera);
        render(&window, &camera, shader_id, model_id);
    }

    return EXIT_SUCCESS;
}

void update(Window *window, Camera *camera)
{
    // if ESC is pressed, close the window
    if (window->get_input_handler()->get_key_state(InputHandler::ESCAPE, InputHandler::PRESSED)) {
        window->close();
    }

    // update camera zoom
    camera->add_zoom((float) window->get_input_handler()->get_yoffset());

    // update camera position
    if (window->get_input_handler()->get_mouse_button_state(InputHandler::RMB, InputHandler::DOWN)) {
        camera->translate(
                window->get_input_handler()->get_mouse_xoffset(),
                window->get_input_handler()->get_mouse_yoffset());
    }

    // update camera rotation
    if (window->get_input_handler()->get_mouse_button_state(InputHandler::MMB, InputHandler::DOWN)) {
        camera->rotate(
                window->get_input_handler()->get_mouse_xoffset(),
                window->get_input_handler()->get_mouse_yoffset());
    }

    // react on window resizing
    if (window->get_input_handler()->is_resized()) {
        uint32_t size_x = window->get_input_handler()->get_size_x();
        uint32_t size_y = window->get_input_handler()->get_size_y();

        // update camera aspect ratio
        camera->set_aspect((float) size_x / (float) size_y);

        // update gl viewport
        glViewport(0, 0, size_x, size_y);
    }
}

void render(Window *window, Camera *camera, AssetID shader_id, AssetID model_id)
{
    window->getRenderer()->clearScreen();

    window->getRenderer()->setCameraPosition(camera->get_camera_position());
    window->getRenderer()->setView(camera->get_view_matrix());
    window->getRenderer()->setPerspective(camera->get_proj_matrix());

    window->getRenderer()->useShader(shader_id);
    window->getRenderer()->renderModel(model_id, glm::identity<glm::mat4>());

    window->swapBuffers();
}