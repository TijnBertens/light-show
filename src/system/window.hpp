#ifndef LIGHT_SHOW_WINDOW_HPP
#define LIGHT_SHOW_WINDOW_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "input.hpp"

#include "vector"

class Renderer;

/** Designed to be similar to {glViewport}. */
struct Alignment {
public:
    /**
     * Specifies the lower left corner of the sub-window.
     * The lower left corner of the parent window is (0,0). */
    uint32_t x;
    uint32_t y;
    /**
     * Specifies the size of the sub-window. */
    uint32_t width;
    uint32_t height;
};

/**
 * Represents a split node where the window is split in two,
 * where one of the partitions is of fixed size. */
struct LayoutNode {
    /** The dimension in which the split happens. */
    enum Orientation {
        HORIZONTAL, VERTICAL
    };

    /** Which part of the split is of fixed size. */
    enum Fixation {
        TOP_LEFT_FIXED, BOTTOM_RIGHT_FIXED
    };

    /** Whether this split node is an actual sub window or a split. */
    bool leaf;

    /** If {leaf}, contains a pointer to the handle for this sub window. */
    uint32_t *id;

    /** If not {leaf}, pointer to node on top or left of split. */
    LayoutNode *top_left;

    /** If not {leaf}, pointer to node on bottom or right of split. */
    LayoutNode *bottom_right;

    /** If not {leaf}, dimension of the split. */
    Orientation orientation;

    /** If not {leaf}, which part of the split is fixed. */
    Fixation fixation;

    /** If not {leaf}, size of the fixed portion of the split. */
    uint32_t pixels;

    explicit LayoutNode(
            uint32_t *id
    ) :
    // orientation, fixation, and pixels not used for a leaf node
            leaf(true), id(id), top_left(nullptr), bottom_right(nullptr),
            orientation(HORIZONTAL), fixation(TOP_LEFT_FIXED), pixels(0)
    {};

    LayoutNode(
            Orientation orientation, Fixation fixation, uint32_t pixels, LayoutNode *top_left, LayoutNode *bottom_right
    ) :
            leaf(false), id(nullptr), top_left(top_left), bottom_right(bottom_right),
            orientation(orientation), fixation(fixation), pixels(pixels)
    {};

    ~LayoutNode()
    {
        delete top_left;
        delete bottom_right;
    }

    Alignment get_new_alignment(Alignment alignment, bool is_top_left)
    {
        if (orientation == HORIZONTAL) {
            if (fixation == TOP_LEFT_FIXED) {
                if (is_top_left) {
                    return {alignment.x, alignment.y, pixels, alignment.height};
                } else {
                    return {alignment.x + pixels, alignment.y, alignment.width - pixels, alignment.height};
                }
            } else {
                if (is_top_left) {
                    return {alignment.x, alignment.y, alignment.width - pixels, alignment.height};
                } else {
                    return {alignment.x + alignment.width - pixels, alignment.y, pixels, alignment.height};
                }
            }
        } else { // if (orientation == VERTICAL)
            if (fixation == TOP_LEFT_FIXED) {
                if (is_top_left) {
                    return {alignment.x, alignment.y + alignment.height - pixels, alignment.width, pixels};
                } else {
                    return {alignment.x, alignment.y, alignment.width, alignment.height - pixels};
                }
            } else {
                if (is_top_left) {
                    return {alignment.x, alignment.y + pixels, alignment.width, alignment.height - pixels};
                } else {
                    return {alignment.x, alignment.y, alignment.width, pixels};
                }
            }
        }
    }
};

/**
 * Represents a physical window in which graphics are displayed.
 */
struct Window {
private:
    LayoutNode *layout;

    GLFWwindow *handle;
    uint32_t initial_width;
    uint32_t initial_height;
    const char *title;

    InputHandler *main_input_handler;

    std::vector<InputHandler *> input_handlers;
    std::vector<Renderer *> renderers;

    uint32_t active_id = 0; // init to zero, why not

    void init_window(uint32_t width, uint32_t height, const char *title);

public:
    Window(uint32_t width, uint32_t height, const char *title, LayoutNode *complete);

    virtual ~Window();

    void swapBuffers();

    /** Poll the GLFW flag indicating whether the window should close. */
    bool shouldClose();

    /** Set GLFW flag to indicate that the window should close. */
    void close();

    GLFWwindow *getHandle();

    /** Returns input handler for the entire window. */
    InputHandler *get_main_input_handler();

    /** Returns input handler for a specific sub window, by its handle. */
    InputHandler *get_input_handler(uint32_t id);

    /** Returns renderer for a specific sub window, by its handle. */
    Renderer *get_renderer(uint32_t id);

    /** Assigns an id, input handler and renderer for each sub window based on {layout}. */
    void initialize_sub_windows(LayoutNode *node, uint32_t *id, Alignment alignment);

    /** Selects the active sub window based on mouse position. */
    void set_active_sub_window(LayoutNode *node, double x, double y, uint32_t width, uint32_t height);

    /** Propagate framebuffer resizing to the sub windows. */
    void resize_sub_windows(LayoutNode *node, Alignment alignment);

    /**
     * Clears input handlers, polls GLFW to receive new events.
     * Selects the active sub window and takes appropriate action for sub windows on resize event. */
    void pull_input();

    /** Begin GLFW callbacks, without {GLFWwindow} pointer (since this is obtained from instance). */

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
