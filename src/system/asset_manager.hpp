//
// Created by Tijn Bertens on 18-7-2020.
//

#ifndef GAME_ASSET_MANAGER_HPP
#define GAME_ASSET_MANAGER_HPP

#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include "tiny_obj_loader.h"

enum AssetType {
    INVALID, MODEL, SHADER
};

struct AssetID {
    /**
     * What type of asset does this ID pertain to.
     */
    const AssetType type;

    /**
     * A unique ID among all assets.
     */
    const uint64_t ID;

    AssetID(AssetType type, uint64_t id);
};

enum TextureFormat {
    GRAYSCALE_8, GRAYSCALE_16, RGB_8, RGBA_8, RGB_16, RGBA_16
};

struct Texture {
    TextureFormat format;

    uint32_t width;
    uint32_t height;

    /**
     * A texture is 'invalid' if the data pointer is 0.
     */
    char *data = nullptr;
};

struct Material {
    std::string name;

    glm::vec3 albedo = {1, 1, 1};
    float roughness = 0.5f;
    float metallic = 0;

    Texture albedoTexture;
    Texture roughnessTexture;
    Texture metallicTexture;
    Texture normalMap;
};

/**
 * An index buffer that is to be rendered with a specific material
 */
struct MaterialSubMesh {
    /**
     * Material index in the material library of the parent Model.
     */
    int32_t materialIndex = -1;
    std::vector<uint32_t> indices;
};

struct Mesh {
    std::string name;

    /**
     * The mesh is divided into submeshes, one for each material.
     */
    std::vector<MaterialSubMesh> materialSubMeshes;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    glm::vec3 tangent;
    glm::vec3 biTangent;
};

struct Model {
    const AssetID assetID;

    std::vector<Vertex> vertices;
    std::vector<Material> materials;

    Mesh mesh;

    explicit Model(uint64_t ID);
};

struct Shader {
    const AssetID assetID;

    std::string vertexShaderText;
    std::string fragmentShaderText;

    explicit Shader(uint64_t ID);
};

class AssetManager {
private:
    /**
     * Used to generate unique IDs for assets.
     */
    uint64_t indexGeneratorCounter = 0;

    std::unordered_map<uint64_t, Model> models;
    std::unordered_map<uint64_t, Shader> shaders;

    uint64_t generateNewID();

    Texture loadTexture(const std::string &file);

public:
    /**
     * @param dir: name of the directory containing .obj and .mtl files.
     * @param file: name of the .obj file within {dir}.
     */
    AssetID loadObj(const std::string &dir, const std::string &file);

    AssetID loadShader(const std::string &vertexShader, const std::string &fragmentShader);

    Model *getModel(AssetID id);

    Shader *getShader(AssetID id);
};

#endif //GAME_ASSET_MANAGER_HPP
