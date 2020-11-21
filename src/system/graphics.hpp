//
// Created by Tijn Bertens on 28-9-2019.
//

#ifndef GAME_GRAPHICS_HPP
#define GAME_GRAPHICS_HPP

#include <cstdio>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include "../util/ls_log.hpp"
#include "asset_manager.hpp"
#include "window.hpp"

struct IndexBuffer {
    int32_t materialIndex;

    uint32_t numIndices;
    GLuint indexBuffer;
};

struct InstanceTransformBuffer {
    GLuint buffer;
    uint32_t elementCount;

    static InstanceTransformBuffer create(std::vector<glm::mat4> *transforms);

    void destroy();

    void updateData(std::vector<glm::mat4> *transforms);
};

struct GPUMaterial {
    glm::vec3 albedo;
    float roughness;
    float metallic;

    GLint albedoTexture;
    GLint roughnessTexture;
    GLint metallicTexture;
    GLint normalTexture;
};

/**
 * Holds a vertex array object on the gpu.
 */
struct VertexArrayObject {
    uint32_t numVertices;
    GLuint vertexBuffer;

    std::vector<IndexBuffer> materialIndexBuffers;
    std::vector<GPUMaterial> materials;

    void unload();

    static VertexArrayObject create(Model *model);
};

/**
 * Holds a shader program on the GPU.
 */
struct ShaderProgram {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;

    static bool checkShaderCompilation(GLuint shader);

    static bool checkProgramLinking(GLuint program);

    static ShaderProgram createShaderProgram(const char *vertexText, const char *fragmentText);
};

/**
 * Top level manager for graphics. Keeps track of loaded assets etc...
 */
struct GraphicsManager {
private:
    std::unordered_map<uint64_t, VertexArrayObject> loadedModels;
    std::unordered_map<uint64_t, ShaderProgram> loadedShaders;

public:
    VertexArrayObject *getVAO(AssetID assetId);

    ShaderProgram *getShaderProgram(AssetID assetId);

    void loadModel(Model *model);

    void loadShader(Shader *shader);
};

/**
 * Renderer object that provides functionality to render graphics objects to a window.
 */
struct Renderer {
private:
    GraphicsManager *graphicsManager = nullptr;

    ShaderProgram activeShader;

    glm::vec3 cameraPosition;
    glm::mat4 activeViewMatrix;
    glm::mat4 activePerspectiveMatrix;

    Alignment alignment;

public:
    Renderer(Alignment alignment);

    void set_alignment(Alignment alignment);

    void clearScreen();

    void setCameraPosition(glm::vec3 pos);

    void setView(glm::mat4 viewMatrix);

    void setPerspective(glm::mat4 perspectiveMatrix);

    void useShader(AssetID id);

    void renderModel(AssetID id, glm::mat4 transform);

    void renderModelInstanced(AssetID id, InstanceTransformBuffer transforms);

    void setGraphicsManager(GraphicsManager *graphicsManager);
};

#endif //GAME_GRAPHICS_HPP

