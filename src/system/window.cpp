#include "window.hpp"

#include <cassert>

#include "../util/ls_log.hpp"
#include "graphics.hpp"

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

void Window::init_window(uint32_t width, uint32_t height, const char *title)
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
}

Window::Window(
        uint32_t width, uint32_t height, const char *title, LayoutNode *complete
) :
        layout(complete)
{
    // glfw initialization
    init_window(width, height, title);

    this->initial_width = width;
    this->initial_height = height;
    this->title = title;

    this->main_input_handler = new InputHandler(width, height);

    // id assignment, creation of renderers and additional input handlers
    uint32_t id = 0;
    initialize_sub_windows(complete, &id, {0, 0, width, height});
}

Window::~Window()
{
    delete this->main_input_handler;
    for (auto &input_handler : this->input_handlers) {
        delete input_handler;
    }
    for (auto &renderer : this->renderers) {
        delete renderer;
    }
    WindowManager::getInstance()->deregisterWindow(this);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(this->handle);
}

InputHandler *Window::get_main_input_handler()
{
    return this->main_input_handler;
}

InputHandler *Window::get_input_handler(uint32_t id)
{
    return this->input_handlers[id];
}

Renderer *Window::get_renderer(uint32_t id)
{
    return this->renderers[id];
}

void Window::initialize_sub_windows(LayoutNode *node, uint32_t *id, Alignment alignment)
{
    if (node->leaf) {
        *node->id = (*id)++;
        this->input_handlers.emplace_back(new InputHandler(alignment.width, alignment.height));
        this->renderers.emplace_back(new Renderer(alignment));
    } else {
        initialize_sub_windows(node->top_left, id, node->get_new_alignment(alignment, true));
        initialize_sub_windows(node->bottom_right, id, node->get_new_alignment(alignment, false));
    }
}

void Window::set_active_sub_window(LayoutNode *node, double x, double y, uint32_t width, uint32_t height)
{
    if (node->leaf) {
        this->active_id = *node->id;
        ls_log::log(LOG_TRACE, "made id %d active\n", this->active_id);
    } else {
        if (node->orientation == LayoutNode::HORIZONTAL) {
            if (node->fixation == LayoutNode::TOP_LEFT_FIXED) {
                uint32_t split = node->pixels;
                if (x < split) {
                    set_active_sub_window(node->top_left, x, y, split, height);
                } else {
                    set_active_sub_window(node->bottom_right, x - split, y, width - split, height);
                }
            } else {
                uint32_t split = width - node->pixels;
                if (x < split) {
                    set_active_sub_window(node->top_left, x, y, split, height);
                } else {
                    set_active_sub_window(node->bottom_right, x - split, y, width - split, height);
                }
            }
        } else {
            if (node->fixation == LayoutNode::TOP_LEFT_FIXED) {
                uint32_t split = node->pixels;
                if (y < split) {
                    set_active_sub_window(node->top_left, x, y, split, height);
                } else {
                    set_active_sub_window(node->bottom_right, x, y - split, width, height - split);
                }
            } else {
                uint32_t split = width - node->pixels;
                if (y < split) {
                    set_active_sub_window(node->top_left, x, y, split, height);
                } else {
                    set_active_sub_window(node->bottom_right, x, y - split, width, height - split);
                }
            }
        }
    }
}

void Window::resize_sub_windows(LayoutNode *node, Alignment alignment)
{
    if (node->leaf) {
        this->input_handlers[*node->id]->framebuffer_size_callback(alignment.width, alignment.height);
        this->renderers[*node->id]->set_alignment(alignment);
    } else {
        resize_sub_windows(node->top_left, node->get_new_alignment(alignment, true));
        resize_sub_windows(node->bottom_right, node->get_new_alignment(alignment, false));
    }
}

void Window::pull_input()
{
    // clear all inputs
    this->main_input_handler->clear_input();
    for (auto &input_handler : this->input_handlers) {
        input_handler->clear_input();
    }

    // poll GLFW events
    glfwPollEvents();

    // change active window based on LMB press event and current mouse position
    if (this->main_input_handler->get_mouse_button_state(InputHandler::LMB, InputHandler::PRESSED)) {
        set_active_sub_window(this->layout,
                              this->main_input_handler->get_mouse_xpos(), this->main_input_handler->get_mouse_ypos(),
                              this->main_input_handler->get_size_x(), this->main_input_handler->get_size_y());
    }

    // propagate framebuffer resize event to sub windows
    if (this->main_input_handler->is_resized()) {
        resize_sub_windows(this->layout,
                           {0, 0, this->main_input_handler->get_size_x(), this->main_input_handler->get_size_y()});
    }
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
    this->main_input_handler->key_callback(key, scancode, action, mods);
    // also update active view
    this->input_handlers[this->active_id]->key_callback(key, scancode, action, mods);
}

void Window::scroll_callback(double xoffset, double yoffset)
{
    this->main_input_handler->scroll_callback(xoffset, yoffset);
    // also update active view
    this->input_handlers[this->active_id]->scroll_callback(xoffset, yoffset);
}

void Window::mouse_button_callback(int button, int action, int mods)
{
    this->main_input_handler->mouse_button_callback(button, action, mods);
    // also update active view
    this->input_handlers[this->active_id]->mouse_button_callback(button, action, mods);
}

void Window::cursor_position_callback(double xpos, double ypos)
{
    this->main_input_handler->cursor_position_callback(xpos, ypos);
    // also update active view
    this->input_handlers[this->active_id]->cursor_position_callback(xpos, ypos);
}

void Window::framebuffer_size_callback(int width, int height)
{
    this->main_input_handler->framebuffer_size_callback(width, height);
    // also update active view
    this->input_handlers[this->active_id]->framebuffer_size_callback(width, height);
}

void Window::window_iconify_callback(int iconified)
{
    this->main_input_handler->window_iconify_callback(iconified);
    // also update active view
    this->input_handlers[this->active_id]->window_iconify_callback(iconified);
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