#include "input.hpp"

#include <cstdlib>
#include <cstring>

#include "util/ls_log.hpp"

InputHandler::InputHandler(GLFWwindow *p_window, uint32_t p_size_x, uint32_t p_size_y
) :
        window_handle(p_window), framebuffer_size_x(p_size_x), framebuffer_size_y(p_size_y)
{
    // initialize the bit vectors with the correct amount of memory
    pressed = (uint32_t *) calloc(VECTOR_COUNT, sizeof(uint32_t));
    down = (uint32_t *) calloc(VECTOR_COUNT, sizeof(uint32_t));
    released = (uint32_t *) calloc(VECTOR_COUNT, sizeof(uint32_t));
}

InputHandler::~InputHandler()
{
    free(pressed);
    free(down);
    free(released);
}

void InputHandler::pull_input()
{
    /** reset the inputs */
    // scroll offset
    scroll_x_offset = 0.;
    scroll_y_offset = 0.;

    // cursor position
    mouse_xoffset = 0.;
    mouse_yoffset = 0.;

    // keyboard and mouse buttons
    // pressed/released events are reset, down events are maintained
    memset(pressed, 0, VECTOR_COUNT * sizeof(uint32_t));
    memset(released, 0, VECTOR_COUNT * sizeof(uint32_t));

    // framebuffer
    framebuffer_resized = false;

    /** populate with new inputs */
    glfwPollEvents();
}

/**
 * Scroll offset.
 */

double InputHandler::get_xoffset() const
{
    return scroll_x_offset;
}

double InputHandler::get_yoffset() const
{
    return scroll_y_offset;
}

void InputHandler::set_scroll_offset(double p_xoffset, double p_yoffset)
{
    scroll_x_offset += p_xoffset;
    scroll_y_offset += p_yoffset;
}

/**
 * Cursor position.
 */

double InputHandler::get_mouse_xpos() const
{
    return mouse_xpos;
}

double InputHandler::get_mouse_ypos() const
{
    return mouse_ypos;
}

double InputHandler::get_mouse_xoffset() const
{
    return mouse_xoffset;
}

double InputHandler::get_mouse_yoffset() const
{
    return mouse_yoffset;
}

void InputHandler::set_cursor_position(double xpos, double ypos)
{
    mouse_xpos = xpos;
    mouse_ypos = ypos;

    mouse_xoffset = xpos - mouse_xpos_last;
    mouse_yoffset = ypos - mouse_ypos_last;

    mouse_xpos_last = xpos;
    mouse_ypos_last = ypos;
}

/**
 * Keyboard and mouse buttons.
 */

uint32_t *InputHandler::get_vector_from_state(key_state_t p_state)
{
    switch (p_state) {
        case PRESSED:
            return pressed;
        case RELEASED:
            return released;
        case DOWN:
            return down;
        default:
            ls_log::log(LOG_ERROR, "unknown key state %d\n", p_state);
            return nullptr;
    }
}

void InputHandler::set_bit_state(uint32_t p_bit, key_state_t p_state, bool p_set)
{
    uint32_t *vector = get_vector_from_state(p_state);
    if (p_set) {
        vector[p_bit / 32] |= 1u << (p_bit % 32);
    } else {
        vector[p_bit / 32] &= ~(1u << (p_bit % 32));
    }
}

bool InputHandler::get_bit_state(uint32_t p_bit, key_state_t p_state)
{
    uint32_t *vector = get_vector_from_state(p_state);
    return vector[p_bit / 32] & (1u << (p_bit % 32));
}

void InputHandler::set_key_state(int p_value, key_state_t p_state, bool p_set)
{
    // {p_value} may take {GLFW_KEY_UNKNOWN} which is -1, which cannot be indexed
    if (p_value >= (signed) KEY_VALUES || p_value < 0) {
        ls_log::log(LOG_WARN, "cannot set key state for unknown value %d\n", p_value);
        return;
    }

    // safely promote signed integer to unsigned integer
    set_bit_state((uint32_t) p_value, p_state, p_set);
}

bool InputHandler::get_key_state(key_value_t p_value, key_state_t p_state)
{
    auto val = (int) p_value; // cast enum to integer
    // {p_value} may take {GLFW_KEY_UNKNOWN} which is -1, which cannot be indexed
    if (val >= (signed) KEY_VALUES || val < 0) {
        ls_log::log(LOG_WARN, "cannot get key state for unknown value %d\n", val);
        return false;
    }

    // safely promote signed integer to unsigned integer
    return get_bit_state((uint32_t) p_value, p_state);
}

void InputHandler::set_mouse_button_state(int p_value, key_state_t p_state, bool p_set)
{
    if (p_value >= (signed) MBN_VALUES) {
        ls_log::log(LOG_WARN, "cannot set mouse button state for unknown value %d\n", p_value);
        return;
    }

    // safely promote signed integer to unsigned integer (since mouse button value cannot be < 0 )
    set_bit_state((uint32_t) p_value + KEY_VALUES, p_state, p_set);
}

bool InputHandler::get_mouse_button_state(mouse_button_value_t p_value, key_state_t p_state)
{
    auto val = (int) p_value; // cast enum to integer
    if (val >= (signed) MBN_VALUES) {
        ls_log::log(LOG_WARN, "cannot get  mouse button state for unknown value %d\n", val);
        return false;
    }

    // safely promote signed integer to unsigned integer (since mouse button value cannot be < 0 )
    return get_bit_state((uint32_t) p_value + KEY_VALUES, p_state);
}

/**
 * Framebuffer.
 */

uint32_t InputHandler::get_size_x() const
{
    return framebuffer_size_x;
}

void InputHandler::set_size(uint32_t size_x, uint32_t size_y)
{
    InputHandler::framebuffer_size_x = size_x;
    InputHandler::framebuffer_size_y = size_y;
}

uint32_t InputHandler::get_size_y() const
{
    return framebuffer_size_y;
}

bool InputHandler::is_resized() const
{
    return framebuffer_resized;
}

void InputHandler::set_resized(bool resized)
{
    InputHandler::framebuffer_resized = resized;
}

bool InputHandler::is_iconified() const
{
    return framebuffer_iconified;
}

void InputHandler::set_iconified(bool iconified)
{
    InputHandler::framebuffer_iconified = iconified;
}
