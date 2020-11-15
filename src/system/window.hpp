#ifndef LIGHT_SHOW_WINDOW_HPP
#define LIGHT_SHOW_WINDOW_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "input.hpp"
#include "graphics.hpp"

/**
 * Represents a physical window in which graphics are displayed.
 */
struct Window {
private:
    GLFWwindow *handle;
    uint32_t initial_width;
    uint32_t initial_height;
    const char *title;
    Renderer *renderer;
    InputHandler *input_handler;
public:
    Window(uint32_t width, uint32_t height, const char *title);

    virtual ~Window();

    void swapBuffers();

    /** Poll the GLFW flag indicating whether the window should close. */
    bool shouldClose();

    /** Set GLFW flag to indicate that the window should close. */
    void close();

    GLFWwindow *getHandle();

    Renderer *getRenderer();

    InputHandler *get_input_handler();

    /** Begin GLFW callbacks, without {GLFWwindow} pointer (since this is obtaind from instance). */

    void key_callback(int key, int scancode, int action, int mods);

    void scroll_callback(double xoffset, double yoffset);

    void mouse_button_callback(int button, int action, int mods);

    void cursor_position_callback(double xpos, double ypos);

    void framebuffer_size_callback(int width, int height);

    void window_iconify_callback(int iconified);

    /** End GLFW callbacks. */
};

/**
 * Top level manager class that allows for multiple windows to be instantiated and controlled individually.
 */
struct WindowManager {
private:
    static WindowManager *singletonInstance;

    std::vector<Window *> windows;

    WindowManager();

    /** Obtain a {Window} pointer from a {GLFWwindow} based on the contents of {windows}. */
    Window *get_window_pointer(GLFWwindow *window);

public:
    void registerWindow(Window *window);

    void deregisterWindow(Window *window);

    static WindowManager *getInstance();

    /** Begin GLFW callbacks. */

    void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

    void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    void window_iconify_callback(GLFWwindow *window, int iconified);

    /** End GLFW callbacks. */
};

#endif //LIGHT_SHOW_WINDOW_HPP
