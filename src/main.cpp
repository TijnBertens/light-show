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

#include "graphics.hpp"
#include "util/ls_log.hpp"
#include "camera.hpp"

bool done = false;

class InputListener : public IInputListener {
public:
    void onMouseMoved(float xMove, float yMove) override
    {

    }

    void onMousePositionCallback(float xPos, float yPos) override
    {

    }

    void onMouseButton(int32_t button, int32_t action, int32_t mods) override
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT) done = true;
    }

    void onMouseScroll(float dx, float dy) override
    {

    }
};

int main()
{
    Window window(800, 600, "");

    auto *listener = new InputListener();
    window.registerInputListener(listener);

    GraphicsManager graphics_manager;
    window.getRenderer()->setGraphicsManager(&graphics_manager);

    AssetManager asset_manager;

    AssetID chandelier_id = asset_manager.loadObj(
            std::string("../res/obj/Chandelier_03"),
            std::string("Chandelier_03.obj"));
    Model *chandelier = asset_manager.getModel(chandelier_id);
    graphics_manager.loadModel(chandelier);

    ls_log::log(LOG_INFO, "done loading model!\n");

    AssetID shader_id = asset_manager.loadShader(
            std::string("../res/shader/pbr.vert"),
            std::string("../res/shader/pbr.frag"));
    Shader *shader = asset_manager.getShader(shader_id);
    graphics_manager.loadShader(shader);

    Camera camera(800.f / 600.f, glm::radians(70.f), .1f, 100.f);

    while (!window.shouldClose() && !done) {
        window.getRenderer()->clearScreen();

        window.getRenderer()->setCameraPosition(camera.get_camera_position());
        window.getRenderer()->setView(camera.get_view_matrix());
        window.getRenderer()->setPerspective(camera.get_proj_matrix());

        window.getRenderer()->useShader(shader_id);
        window.getRenderer()->renderModel(chandelier_id, glm::identity<glm::mat4>());

        window.swapBuffers();
        window.pollEvents();
    }

    return EXIT_SUCCESS;
}