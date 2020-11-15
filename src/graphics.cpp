//
// Created by Tijn Bertens on 28-9-2019.
//

#include <glm/gtc/type_ptr.hpp>
#include "graphics.hpp"

void globalErrorCallback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

void globalCursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
    WindowManager::getInstance()->cursorPositionCallback(window, xpos, ypos);
}

void globalMouseButtonCallback(GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
    WindowManager::getInstance()->mouseButtonCallback(window, button, action, mods);
}

void globalMouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    WindowManager::getInstance()->mouseScrollCallback(window, xoffset, yoffset);
}

InstanceTransformBuffer InstanceTransformBuffer::create(std::vector<glm::mat4> *transforms) {
    InstanceTransformBuffer result = {};

    GLuint buffer;
    glGenBuffers(1, &buffer);

    assert(buffer);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, transforms->size() * sizeof(glm::mat4), &(*transforms)[0], GL_STATIC_DRAW);

    result.buffer = buffer;
    result.elementCount = transforms->size();

    return result;
}

void InstanceTransformBuffer::destroy() {
    glDeleteBuffers(1, &buffer);
}

void InstanceTransformBuffer::updateData(std::vector<glm::mat4> *transforms) {
    if (transforms->size() != this->elementCount) {
        printf("Instance transform buffer updated with wrong element count. Got %u while expecting %u. \n",
               transforms->size(), this->elementCount
        );
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, transforms->size() * sizeof(glm::mat4), &(*transforms)[0], GL_STATIC_DRAW);
}

GLint createTexture(const Texture *tex) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float aniso = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

    GLint internalFormat;
    GLenum pixelFormat;
    GLenum dataType;
    if (tex->format == GRAYSCALE_8) {
        internalFormat = GL_LUMINANCE8;
        pixelFormat = GL_RED;
        dataType = GL_UNSIGNED_BYTE;
    } else if (tex->format == GRAYSCALE_16) {
        internalFormat = GL_LUMINANCE16;
        pixelFormat = GL_RED;
        dataType = GL_UNSIGNED_SHORT;
    } else if (tex->format == RGB_8) {
        internalFormat = GL_RGB;
        pixelFormat = GL_RGB;
        dataType = GL_UNSIGNED_BYTE;
    } else if (tex->format == RGBA_8) {
        internalFormat = GL_RGBA;
        pixelFormat = GL_RGBA;
        dataType = GL_UNSIGNED_BYTE;
    } else if (tex->format == RGB_16) {
        internalFormat = GL_RGB16;
        pixelFormat = GL_RGB;
        dataType = GL_UNSIGNED_SHORT;
    } else if (tex->format == RGBA_16) {
        internalFormat = GL_RGBA;
        pixelFormat = GL_RGBA;
        dataType = GL_UNSIGNED_SHORT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tex->width, tex->height, 0, pixelFormat, dataType, tex->data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

VertexArrayObject VertexArrayObject::create(Model *model) {
    //TODO: robustness
    VertexArrayObject result = {};

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, model->vertices.size() * sizeof(Vertex), model->vertices.data(), GL_STATIC_DRAW);

    result.numVertices = model->vertices.size();
    result.vertexBuffer = vertexBuffer;

    for (const auto &materialSubMesh: model->mesh.materialSubMeshes) {
        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, materialSubMesh.indices.size() * sizeof(uint32_t), &materialSubMesh.indices[0],
                     GL_STATIC_DRAW);

        IndexBuffer buffer = {};
        buffer.materialIndex = materialSubMesh.materialIndex;
        buffer.numIndices = materialSubMesh.indices.size();
        buffer.indexBuffer = indexBuffer;
        result.materialIndexBuffers.emplace_back(buffer);
    }

    for (const auto &material: model->materials) {
        GPUMaterial newMaterial = {};
        newMaterial.albedo = material.albedo;
        newMaterial.roughness = material.roughness;
        newMaterial.metallic = material.metallic;

        if (material.albedoTexture.data) {
            newMaterial.albedoTexture = createTexture(&material.albedoTexture);
        }
        if (material.roughnessTexture.data) {
            newMaterial.roughnessTexture = createTexture(&material.roughnessTexture);
        }
        if (material.metallicTexture.data) {
            newMaterial.metallicTexture = createTexture(&material.metallicTexture);
        }
        if (material.normalMap.data) {
            newMaterial.normalTexture = createTexture(&material.normalMap);
        }

        result.materials.emplace_back(newMaterial);
    }

    return result;
}

void VertexArrayObject::unload() {
    glDeleteBuffers(1, &vertexBuffer);

    for (auto &indexBuffer: materialIndexBuffers) {
        glDeleteBuffers(1, &indexBuffer.indexBuffer);
    }

    //TODO: unload textures
}

Renderer::Renderer() {

}

void Renderer::clearScreen() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderModel(AssetID id, glm::mat4 transform) {
    assert(graphicsManager);

    VertexArrayObject *vao = graphicsManager->getVAO(id);

    GLint vPosPosition = glGetAttribLocation(activeShader.program, "vPos");
    GLint vNormPosition = glGetAttribLocation(activeShader.program, "vNorm");
    GLint vTexcoordPosition = glGetAttribLocation(activeShader.program, "vTex");
    GLint tangentPosition = glGetAttribLocation(activeShader.program, "tangent");
    GLint biTangentPosition = glGetAttribLocation(activeShader.program, "biTangent");

    GLint modelPosition = glGetUniformLocation(activeShader.program, "ModelM");
    GLint viewPosition = glGetUniformLocation(activeShader.program, "ViewM");
    GLint projPosition = glGetUniformLocation(activeShader.program, "ProjectionM");
    GLint camPosPosition = glGetUniformLocation(activeShader.program, "cameraPosition");

    GLint albedoConstantPosition = glGetUniformLocation(activeShader.program, "albedoConstant");
    GLint roughnessConstantPosition = glGetUniformLocation(activeShader.program, "roughnessConstant");
    GLint metallicConstantPosition = glGetUniformLocation(activeShader.program, "metallicConstant");

    GLint useAlbedoTexturePosition = glGetUniformLocation(activeShader.program, "useAlbedoTexture");
    GLint useRoughnessTexturePosition = glGetUniformLocation(activeShader.program, "useRoughnessTexture");
    GLint useMetallicTexturePosition = glGetUniformLocation(activeShader.program, "useMetallicTexture");
    GLint useNormalTexturePosition = glGetUniformLocation(activeShader.program, "useNormalTexture");

    GLint albedoTexPosition = glGetUniformLocation(activeShader.program, "albedoTexture");
    GLint roughnessTexPosition = glGetUniformLocation(activeShader.program, "roughnessTexture");
    GLint metallicTexPosition = glGetUniformLocation(activeShader.program, "metallicTexture");
    GLint normalTexPosition = glGetUniformLocation(activeShader.program, "normalTexture");

    glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBuffer);

    glEnableVertexAttribArray(vPosPosition);
    glVertexAttribPointer(vPosPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(vNormPosition);
    glVertexAttribPointer(vNormPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    glEnableVertexAttribArray(vTexcoordPosition);
    glVertexAttribPointer(vTexcoordPosition, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));

    glEnableVertexAttribArray(tangentPosition);
    glVertexAttribPointer(tangentPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tangent));

    glEnableVertexAttribArray(biTangentPosition);
    glVertexAttribPointer(biTangentPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, biTangent));

    glm::mat4 model = transform;
    glm::mat4 view = activeViewMatrix;
    glm::mat4 projection = activePerspectiveMatrix;

    glUniformMatrix4fv(modelPosition, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewPosition, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projPosition, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(camPosPosition, 1, glm::value_ptr(cameraPosition));

    glUniform1i(albedoTexPosition, 0);
    glUniform1i(roughnessTexPosition, 1);
    glUniform1i(metallicTexPosition, 2);
    glUniform1i(normalTexPosition, 3);

    for (const auto &indexBuffer: vao->materialIndexBuffers) {
        GPUMaterial &material = vao->materials[indexBuffer.materialIndex];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.roughnessTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.metallicTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, material.normalTexture);

        glUniform3fv(albedoConstantPosition, 1, glm::value_ptr(material.albedo));
        glUniform1f(roughnessConstantPosition, material.roughness);
        glUniform1f(metallicConstantPosition, material.metallic);

        glUniform1i(useAlbedoTexturePosition, (material.albedoTexture == 0) ? 0 : 1);
        glUniform1i(useRoughnessTexturePosition, (material.roughnessTexture == 0) ? 0 : 1);
        glUniform1i(useMetallicTexturePosition, (material.metallicTexture == 0) ? 0 : 1);
        glUniform1i(useNormalTexturePosition, (material.normalTexture == 0) ? 0 : 1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.indexBuffer);

        glDrawElements(GL_TRIANGLES, indexBuffer.numIndices, GL_UNSIGNED_INT, nullptr);
    }
}

void Renderer::setView(glm::mat4 viewMatrix) {
    this->activeViewMatrix = viewMatrix;
}

void Renderer::setPerspective(glm::mat4 perspectiveMatrix) {
    this->activePerspectiveMatrix = perspectiveMatrix;
}

void Renderer::useShader(AssetID shaderID) {
    assert(graphicsManager);
    assert(shaderID.type == SHADER);

    ShaderProgram *shaderProgram = this->graphicsManager->getShaderProgram(shaderID);

    if (shaderProgram) {
        this->activeShader = *shaderProgram;
        glUseProgram(shaderProgram->program);
    }
}

void Renderer::setGraphicsManager(GraphicsManager *graphicsManager) {
    this->graphicsManager = graphicsManager;
}

void Renderer::setCameraPosition(glm::vec3 pos) {
    this->cameraPosition = pos;
}

void Renderer::renderModelInstanced(AssetID id, InstanceTransformBuffer transforms) {
    assert(graphicsManager);

    VertexArrayObject *vao = graphicsManager->getVAO(id);

    GLint vPosPosition = glGetAttribLocation(activeShader.program, "vPos");
    GLint vNormPosition = glGetAttribLocation(activeShader.program, "vNorm");
    GLint vTexcoordPosition = glGetAttribLocation(activeShader.program, "vTex");
    GLint tangentPosition = glGetAttribLocation(activeShader.program, "tangent");
    GLint biTangentPosition = glGetAttribLocation(activeShader.program, "biTangent");
    GLint modelPosition = glGetAttribLocation(activeShader.program, "ModelM");

    GLint viewPosition = glGetUniformLocation(activeShader.program, "ViewM");
    GLint projPosition = glGetUniformLocation(activeShader.program, "ProjectionM");
    GLint camPosPosition = glGetUniformLocation(activeShader.program, "cameraPosition");

    GLint albedoConstantPosition = glGetUniformLocation(activeShader.program, "albedoConstant");
    GLint roughnessConstantPosition = glGetUniformLocation(activeShader.program, "roughnessConstant");
    GLint metallicConstantPosition = glGetUniformLocation(activeShader.program, "metallicConstant");

    GLint useAlbedoTexturePosition = glGetUniformLocation(activeShader.program, "useAlbedoTexture");
    GLint useRoughnessTexturePosition = glGetUniformLocation(activeShader.program, "useRoughnessTexture");
    GLint useMetallicTexturePosition = glGetUniformLocation(activeShader.program, "useMetallicTexture");
    GLint useNormalTexturePosition = glGetUniformLocation(activeShader.program, "useNormalTexture");

    GLint albedoTexPosition = glGetUniformLocation(activeShader.program, "albedoTexture");
    GLint roughnessTexPosition = glGetUniformLocation(activeShader.program, "roughnessTexture");
    GLint metallicTexPosition = glGetUniformLocation(activeShader.program, "metallicTexture");
    GLint normalTexPosition = glGetUniformLocation(activeShader.program, "normalTexture");

    // Bind vertex attribute data

    glBindBuffer(GL_ARRAY_BUFFER, vao->vertexBuffer);

    glEnableVertexAttribArray(vPosPosition);
    glVertexAttribPointer(vPosPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, position));

    glEnableVertexAttribArray(vNormPosition);
    glVertexAttribPointer(vNormPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, normal));

    glEnableVertexAttribArray(vTexcoordPosition);
    glVertexAttribPointer(vTexcoordPosition, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, uv));

    glEnableVertexAttribArray(tangentPosition);
    glVertexAttribPointer(tangentPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *) offsetof(Vertex, tangent));

    glEnableVertexAttribArray(biTangentPosition);
    glVertexAttribPointer(biTangentPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void *) offsetof(Vertex, biTangent));

    // Bind instance transform buffer to the 4 vectors making up the model matrix

    glBindBuffer(GL_ARRAY_BUFFER, transforms.buffer);

    glEnableVertexAttribArray(modelPosition);
    glVertexAttribPointer(modelPosition, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *) 0);

    glEnableVertexAttribArray(modelPosition + 1);
    glVertexAttribPointer(modelPosition + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *) (1 * sizeof(glm::vec4)));

    glEnableVertexAttribArray(modelPosition + 2);
    glVertexAttribPointer(modelPosition + 2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *) (2 * sizeof(glm::vec4)));

    glEnableVertexAttribArray(modelPosition + 3);
    glVertexAttribPointer(modelPosition + 3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void *) (3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(modelPosition + 0, 1);
    glVertexAttribDivisor(modelPosition + 1, 1);
    glVertexAttribDivisor(modelPosition + 2, 1);
    glVertexAttribDivisor(modelPosition + 3, 1);

    // Bind uniforms

    glm::mat4 view = activeViewMatrix;
    glm::mat4 projection = activePerspectiveMatrix;

    glUniformMatrix4fv(viewPosition, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projPosition, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(camPosPosition, 1, glm::value_ptr(cameraPosition));

    glUniform1i(albedoTexPosition, 0);
    glUniform1i(roughnessTexPosition, 1);
    glUniform1i(metallicTexPosition, 2);
    glUniform1i(normalTexPosition, 3);

    for (const auto &indexBuffer: vao->materialIndexBuffers) {
        GPUMaterial &material = vao->materials[indexBuffer.materialIndex];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.albedoTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.roughnessTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, material.metallicTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, material.normalTexture);

        glUniform3fv(albedoConstantPosition, 1, glm::value_ptr(material.albedo));
        glUniform1f(roughnessConstantPosition, material.roughness);
        glUniform1f(metallicConstantPosition, material.metallic);

        glUniform1i(useAlbedoTexturePosition, (material.albedoTexture == 0) ? 0 : 1);
        glUniform1i(useRoughnessTexturePosition, (material.roughnessTexture == 0) ? 0 : 1);
        glUniform1i(useMetallicTexturePosition, (material.metallicTexture == 0) ? 0 : 1);
        glUniform1i(useNormalTexturePosition, (material.normalTexture == 0) ? 0 : 1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.indexBuffer);

        glDrawElementsInstanced(GL_TRIANGLES, indexBuffer.numIndices, GL_UNSIGNED_INT, nullptr,
                                transforms.elementCount);
    }
}

Window::Window(uint32_t width, uint32_t height, const char *title) {
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

    glfwSetCursorPosCallback(window, globalCursorPositionCallback);
    glfwSetMouseButtonCallback(window, globalMouseButtonCallback);
    glfwSetScrollCallback(window, globalMouseScrollCallback);

    this->handle = window;
    this->width = width;
    this->height = height;
    this->title = title;

    this->renderer = new Renderer();
}

void Window::swapBuffers() {
    glfwSwapBuffers(this->handle);
}

void Window::pollEvents() {
    glfwPollEvents();
}

Renderer *Window::getRenderer() {
    return this->renderer;
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(this->handle);
}

GLFWwindow *Window::getHandle() {
    return this->handle;
}

void Window::cursorPositionCallback(double xPos, double yPos) {
    for (auto &listener: this->inputListeners) {
        listener->onMousePositionCallback(xPos, yPos);
    }

    float relX = (float) (xPos - prevMouseX);
    float relY = (float) (yPos - prevMouseY);

    if (relX != 0 || relY != 0) {
        for (auto &listener: this->inputListeners) {
            listener->onMouseMoved(relX, relY);
        }
    }

    prevMouseX = (float) xPos;
    prevMouseY = (float) yPos;
}

void Window::mouseButtonCallback(int32_t button, int32_t action, int32_t mods) {
    for (auto &listener: this->inputListeners) {
        listener->onMouseButton(button, action, mods);
    }
}

void Window::mouseScrollCallback(float xoffset, float yoffset) {
    for (auto &listener: this->inputListeners) {
        listener->onMouseScroll(xoffset, yoffset);
    }
}

void Window::registerInputListener(IInputListener *listener) {
    this->inputListeners.emplace_back(listener);
}

void Window::unregisterInputListener(IInputListener *listener) {
    std::remove(this->inputListeners.begin(), this->inputListeners.end(), listener);
}

void Window::destroy() {
    delete this->renderer;

    //TODO: destroy window
}

WindowManager *WindowManager::singletonInstance = 0;

WindowManager::WindowManager() {
    if (!glfwInit()) {
        printf("Could not initialize glfw.\n");
        assert(false);
    }
    glfwSetErrorCallback(globalErrorCallback);
}

void WindowManager::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos) {
    for (auto &win : this->windows) {
        if (window == win->getHandle()) {
            win->cursorPositionCallback(xpos, ypos);
            break;
        }
    }
}

void WindowManager::mouseButtonCallback(GLFWwindow *window, int32_t button, int32_t action, int32_t mods) {
    for (auto &win : this->windows) {
        if (window == win->getHandle()) {
            win->mouseButtonCallback(button, action, mods);
            break;
        }
    }
}

void WindowManager::mouseScrollCallback(GLFWwindow *window, float xoffset, float yoffset) {
    for (auto &win : this->windows) {
        if (window == win->getHandle()) {
            win->mouseScrollCallback(xoffset, yoffset);
            break;
        }
    }
}

void WindowManager::registerWindow(Window *window) {
    assert(std::find(windows.begin(), windows.end(), window) == windows.end());
    this->windows.emplace_back(window);
}

void WindowManager::deregisterWindow(Window *window) {
    std::remove(windows.begin(), windows.end(), window);
}

WindowManager *WindowManager::getInstance() {
    if (!singletonInstance) {
        singletonInstance = new WindowManager();
    }

    return singletonInstance;
}

bool ShaderProgram::checkShaderCompilation(GLuint shader) {
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // Get error log
        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        ls_log::log(LOG_ERROR, "%s\n", &errorLog[0]);

        glDeleteShader(shader);
        return false;
    }

    return true;
}

bool ShaderProgram::checkProgramLinking(GLuint program) {
    GLint isCompiled = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // Get error log
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

        ls_log::log(LOG_ERROR, "%s\n", &errorLog[0]);

        glDeleteProgram(program);
        return false;
    }

    return true;
}

ShaderProgram ShaderProgram::createShaderProgram(const char *vertexText, const char *fragmentText) {
    ShaderProgram result = {};

    GLuint vertexShader, fragmentShader;

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexText, NULL);
    glCompileShader(vertexShader);

    if (!checkShaderCompilation(vertexShader)) {
        result.vertexShader = 0;
        result.fragmentShader = 0;
        result.program = 0;

        return result;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentText, NULL);
    glCompileShader(fragmentShader);

    if (!checkShaderCompilation(fragmentShader)) {
        result.vertexShader = 0;
        result.fragmentShader = 0;
        result.program = 0;

        return result;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    if (!checkProgramLinking(program)) {
        result.vertexShader = 0;
        result.fragmentShader = 0;
        result.program = 0;

        return result;
    }


    result.vertexShader = vertexShader;
    result.fragmentShader = fragmentShader;
    result.program = program;

    return result;
}

VertexArrayObject *GraphicsManager::getVAO(AssetID assetId) {
    assert(assetId.type == MODEL);
    return &loadedModels.at(assetId.ID);
}

ShaderProgram *GraphicsManager::getShaderProgram(AssetID assetId) {
    assert(assetId.type == SHADER);
    return &loadedShaders.at(assetId.ID);
}

void GraphicsManager::loadModel(Model *model) {
    //TODO: robustness
    VertexArrayObject vao = VertexArrayObject::create(model);
    loadedModels.emplace(model->assetID.ID, vao);
}

void GraphicsManager::loadShader(Shader *shader) {
    //TODO: robustness?
    ShaderProgram s = ShaderProgram::createShaderProgram(shader->vertexShaderText.c_str(),
                                                         shader->fragmentShaderText.c_str());
    loadedShaders.emplace(shader->assetID.ID, s);
}

