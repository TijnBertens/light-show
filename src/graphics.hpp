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

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "asset_manager.hpp"

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
    static ShaderProgram createShaderProgram(const char *vertexText,const char *fragmentText);
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

public:
    Renderer();

    void clearScreen();

    void setCameraPosition(glm::vec3 pos);
    void setView(glm::mat4 viewMatrix);
    void setPerspective(glm::mat4 perspectiveMatrix);
    void useShader(AssetID id);
    void renderModel(AssetID id, glm::mat4 transform);
    void renderModelInstanced(AssetID id, InstanceTransformBuffer transforms);

    void setGraphicsManager(GraphicsManager *graphicsManager);
};

/**
 * Interface for input handling.
 */
class IInputListener {
public:
    virtual void onMouseMoved(float xMove, float yMove) = 0;
    virtual void onMousePositionCallback(float xPos, float yPos) = 0;
    virtual void onMouseButton(int32_t button, int32_t action, int32_t mods) = 0;
    virtual void onMouseScroll(float dx, float dy) = 0;
};

/**
 * Represents a physical window in which graphics are displayed.
 */
struct Window {
private:
    GLFWwindow *handle;
    uint32_t width;
    uint32_t height;
    const char *title;

    Renderer *renderer;
    std::vector<IInputListener *> inputListeners;

    float prevMouseX = 0;
    float prevMouseY = 0;

public:
    Window(uint32_t width, uint32_t height, const char *title);

    void swapBuffers();
    void pollEvents();
    bool shouldClose();

    GLFWwindow *getHandle();
    void cursorPositionCallback(double xpos, double ypos);
    void mouseButtonCallback(int32_t button, int32_t action, int32_t mods);
    void mouseScrollCallback(float xoffset, float yoffset);

    Renderer *getRenderer();

    void registerInputListener(IInputListener *listener);
    void unregisterInputListener(IInputListener *listener);

    void destroy();
};

/**
 * Top level manager class that allows for multiple windows to be instantiated and controlled individually.
 */
struct WindowManager {
private:
    static WindowManager *singletonInstance;

    std::vector<Window *> windows;
    WindowManager();

public:
    void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    void mouseButtonCallback(GLFWwindow* window, int32_t button, int32_t action, int32_t mods);
    void mouseScrollCallback(GLFWwindow* window, float xoffset, float yoffset);

    void registerWindow(Window *window);
    void deregisterWindow(Window *window);

    static WindowManager *getInstance();
};


/*
 * global callbacks
 */

void globalErrorCallback(int error, const char *description);
void globalCursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
void globalMouseButtonCallback(GLFWwindow *window, int32_t button, int32_t action, int32_t mods);
void globalMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);


#endif //GAME_GRAPHICS_HPP

