#ifndef OPENGL_TEXTURE_HPP
#define OPENGL_TEXTURE_HPP

#include <glad/glad.h>

class Texture {
public:
    enum ChannelType {
        INTEGER, FLOATING_POINT
    };

    enum WrapType {
        REPEAT, CLAMP
    };
private:
    /** Id of the texture. */
    GLuint tex_id;
public:
    /** Reads a file from disk into memory, calls {create_tex_from_mem} on that memory and deallocates the memory. */
    static int create_tex_from_file(
            Texture *tex, const char *tex_file,
            uint32_t channel_count, uint32_t bit_depth,
            ChannelType channel_type, WrapType wrap_type);

    /** Uses stbi_image.h to read in texture data from memory and create a {GL_TEXTURE_2D} from it. */
    static int create_tex_from_mem(
            Texture *tex, const char *tex_data, size_t tex_len,
            uint32_t channel_count, uint32_t bit_depth,
            ChannelType channel_type, WrapType wrap_type);

    static void bind_tex(Texture *tex, uint32_t texture_unit);

    /** Unbinds the current texture. */
    static void unbind_tex(Texture *tex, uint32_t texture_unit);

    static void delete_tex(Texture *tex);
};

#endif //OPENGL_TEXTURE_HPP
