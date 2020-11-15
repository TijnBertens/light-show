#include "window.hpp"

/** Begin global static GLFW callbacks (in source file since static functions). */

static void global_error_callback(int error, const char *description)
{
    ls_log::log(LOG_ERROR, "GLFW error with code %d:\n", error);
    ls_log::log(LOG_ERROR, description);
}

static void global_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    WindowManager::getInstance()->key_callback(window, key, scancode, action, mods);
}

static void global_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    WindowManager::getInstance()->scroll_callback(window, xoffset, yoffset);
}

static void global_mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    WindowManager::getInstance()->mouse_button_callback(window, button, action, mods);
}

static void global_cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    WindowManager::getInstance()->cursor_position_callback(window, xpos, ypos);
}

static void global_framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    WindowManager::getInstance()->framebuffer_size_callback(window, width, height);
}

static void global_window_iconify_callback(GLFWwindow *window, int iconified)
{

    WindowManager::getInstance()->window_iconify_callback(window, iconified);
}

/** End global static GLFW callbacks. */

Window::Window(uint32_t width, uint32_t height, const char *title)
{
    //TODO: find a place to initialize/load GL.
    WindowManager::getInstance()->registerWindow(this);

    //TODO: temp AA
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!window) {
        printf("Could not open glfw window.\n");
        glfwTerminate();
        assert(false);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        printf("Glad could not load GL.\n");
        glfwTerminate();
        assert(false);
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor(241.f / 255.f, 250.f / 255.f, 238.f / 255.f, 1);

    glfwSetKeyCallback(window, global_key_callback);
    glfwSetScrollCallback(window, global_scroll_callback);
    glfwSetMouseButtonCallback(window, global_mouse_button_callback);
    glfwSetCursorPosCallback(window, global_cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, global_framebuffer_size_callback);
    glfwSetWindowIconifyCallback(window, global_window_iconify_callback);

    this->handle = window;
    this->initial_width = width;
    this->initial_height = height;
    this->title = title;

    this->renderer = new Renderer();
    this->input_handler = new InputHandler(width, height);
}

Window::~Window()
{
    delete this->input_handler;
    delete this->renderer;
    WindowManager::getInstance()->deregisterWindow(this);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(this->handle);
}

Renderer *Window::getRenderer()
{
    return this->renderer;
}

InputHandler *Window::get_input_handler()
{
    return this->input_handler;
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(this->handle);
}

void Window::close()
{
    glfwSetWindowShouldClose(this->handle, GLFW_TRUE);
}

GLFWwindow *Window::getHandle()
{
    return this->handle;
}

void Window::key_callback(int key, int scancode, int action, int mods)
{
    // per documentation: "The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE."
    if (action == GLFW_PRESS) {
        this->input_handler->set_key_state(key, InputHandler::PRESSED, true);
        this->input_handler->set_key_state(key, InputHandler::DOWN, true);
    } else { // if (action == GLFW_RELEASE) {
        this->input_handler->set_key_state(key, InputHandler::RELEASED, true);
        this->input_handler->set_key_state(key, InputHandler::DOWN, false);
    }
}

void Window::scroll_callback(double xoffset, double yoffset)
{
    this->input_handler->set_scroll_offset(xoffset, yoffset);
}

void Window::mouse_button_callback(int button, int action, int mods)
{
    // per documentation: "The action is one of GLFW_PRESS or GLFW_RELEASE."
    if (action == GLFW_PRESS) {
        this->input_handler->set_mouse_button_state(button, InputHandler::PRESSED, true);
        this->input_handler->set_mouse_button_state(button, InputHandler::DOWN, true);
    } else { // if (action == GLFW_RELEASE) {
        this->input_handler->set_mouse_button_state(button, InputHandler::RELEASED, true);
        this->input_handler->set_mouse_button_state(button, InputHandler::DOWN, false);
    }
}

void Window::cursor_position_callback(double xpos, double ypos)
{
    this->input_handler->set_cursor_position(xpos, ypos);
}

void Window::framebuffer_size_callback(int width, int height)
{
    this->input_handler->set_size(width, height);
    this->input_handler->set_resized(true);
}

void Window::window_iconify_callback(int iconified)
{
    this->input_handler->set_iconified((bool) iconified);
}

WindowManager *WindowManager::singletonInstance = 0;

WindowManager::WindowManager()
{
    // per documentation: "This function may be called before glfwInit."
    glfwSetErrorCallback(global_error_callback);

    if (!glfwInit()) {
        ls_log::log(LOG_ERROR, "Could not initialize glfw.\n");
        assert(false);
    }
}

Window *WindowManager::get_window_pointer(GLFWwindow *window)
{
    for (auto &win : this->windows) {
        if (window == win->getHandle()) {
            return win;
        }
    }

    ls_log::log(LOG_ERROR, "cannot find Window instance for GLFWwindow handle\n");

    return nullptr;
}

void WindowManager::registerWindow(Window *window)
{
    assert(std::find(windows.begin(), windows.end(), window) == windows.end());
    this->windows.emplace_back(window);
}

void WindowManager::deregisterWindow(Window *window)
{
    std::remove(windows.begin(), windows.end(), window);
}

WindowManager *WindowManager::getInstance()
{
    if (!singletonInstance) {
        singletonInstance = new WindowManager();
    }

    return singletonInstance;
}

void WindowManager::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    this->get_window_pointer(window)->key_callback(key, scancode, action, mods);
}

void WindowManager::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    this->get_window_pointer(window)->scroll_callback(xoffset, yoffset);
}

void WindowManager::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    this->get_window_pointer(window)->mouse_button_callback(button, action, mods);
}

void WindowManager::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    this->get_window_pointer(window)->cursor_position_callback(xpos, ypos);
}

void WindowManager::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    this->get_window_pointer(window)->framebuffer_size_callback(width, height);
}

void WindowManager::window_iconify_callback(GLFWwindow *window, int iconified)
{
    this->get_window_pointer(window)->window_iconify_callback(iconified);
}