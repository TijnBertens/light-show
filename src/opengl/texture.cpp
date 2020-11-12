#include "texture.hpp"

#include <stb_image.h> // NB: required define is in main.cpp

#include "../util/ls_log.hpp"
#include "../util/util.hpp"

int Texture::create_tex_from_file(
        Texture *tex, const char *tex_file,
        uint32_t channel_count, uint32_t bit_depth,
        ChannelType channel_type, WrapType wrap_type)
{
    char *buffer = nullptr;
    size_t size;
    Util::read_file(&buffer, &size, tex_file);

    int rval = create_tex_from_mem(tex, buffer, size, channel_count, bit_depth, channel_type, wrap_type);

    delete buffer;

    return rval;
}

int Texture::create_tex_from_mem(
        Texture *tex, const char *tex_data, size_t tex_len,
        uint32_t channel_count, uint32_t bit_depth,
        ChannelType channel_type, WrapType wrap_type
)
{
    glGenTextures(1, &tex->tex_id);
    glBindTexture(GL_TEXTURE_2D, tex->tex_id);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    if (wrap_type == REPEAT) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } else if (wrap_type == CLAMP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        ls_log::log(LOG_WARN, "unrecognized wrapping type, guessing GL_REPEAT\n");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    // load and generate the texture
    int found_width, found_height, found_channel_count;
    // NB: first pixel is top-left corner, flip it with this call
    stbi_set_flip_vertically_on_load(1);
    void *data;
    // todo could combine channel type and bit depth into a single enum variable
    if (bit_depth == 8 && channel_type == INTEGER) {
        // unsigned char *
        default_depth:
        data = stbi_load_from_memory(
                (const unsigned char *) tex_data, tex_len, &found_width, &found_height, &found_channel_count,
                channel_count);
    } else if (bit_depth == 16 && channel_type == INTEGER) {
        // unsigned short *
        data = stbi_load_16_from_memory(
                (const unsigned char *) tex_data, tex_len, &found_width, &found_height, &found_channel_count,
                channel_count);
    } else if (bit_depth == 32 && channel_type == FLOATING_POINT) {
        // float *
        data = stbi_loadf_from_memory(
                (const unsigned char *) tex_data, tex_len, &found_width, &found_height, &found_channel_count,
                channel_count);
    } else {
        ls_log::log(LOG_WARN, "no stbi function found for specified bit depth, guessing 8 bit depth\n");
        goto default_depth;
    }


    if (found_channel_count != (int) channel_count) {
        ls_log::log(LOG_WARN,
                    "specified channel count (%d) not equal to found channel count (%d)\n",
                    channel_count, found_channel_count, tex->tex_id);
    }

    // keep on moving forward with assumed channel count since this is the size of the buffer
    // since stbi may complain but will always allocate the number of channels the user specifies
    if (data) {
        GLenum format;
        GLenum internal_format;
        if (channel_count == 2) {
            format = GL_RG;
            internal_format = GL_RG;
        } else if (channel_count == 3) {
            format = GL_RGB;
            internal_format = GL_RGB;
        } else if (channel_count == 4) {
            format = GL_RGBA;
            internal_format = GL_RGBA;
        } else {
            ls_log::log(LOG_WARN, "unknown texture format, guessing GL_RGBA\n");
            format = GL_RGBA;
            internal_format = GL_RGBA;
        }

        GLenum type;
        if (bit_depth == 8 && channel_type == INTEGER) {
            type = GL_UNSIGNED_BYTE;
        } else if (bit_depth == 16 && channel_type == INTEGER) {
            type = GL_UNSIGNED_SHORT;
        } else if (bit_depth == 32 && channel_type == FLOATING_POINT) {
            type = GL_FLOAT;
        } else {
            ls_log::log(LOG_WARN, "unknown texture type, guessing GL_UNSIGNED_BYTE\n");
            type = GL_UNSIGNED_BYTE;
        }

        // NB: first pixel is lower-left corner
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, found_width, found_height, 0, format, type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        ls_log::log(LOG_ERROR, "failed to load texture\n");
        stbi_image_free(data);

        return EXIT_FAILURE;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return EXIT_SUCCESS;
}

void Texture::bind_tex(Texture *tex, uint32_t texture_unit)
{
    // todo could theoretically not result in a valid texture unit
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, tex->tex_id);
}

void Texture::unbind_tex(Texture *tex, uint32_t texture_unit)
{
    // todo could theoretically not result in a valid texture unit
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::delete_tex(Texture *tex)
{
    glDeleteTextures(1, &tex->tex_id);
}