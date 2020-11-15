//
// Created by Tijn Bertens on 18-7-2020.
//

#include "asset_manager.hpp"

#define TINYOBJLOADER_IMPLEMENTATION

#include "tiny_obj_loader.h"
#include "util/ls_log.hpp"

#include <stb_image.h> // NB: required define is in main.cpp

AssetID::AssetID(AssetType type, uint64_t id) : type(type), ID(id) {}

Model::Model(uint64_t ID) : assetID(MODEL, ID) {}

Shader::Shader(uint64_t ID) : assetID(SHADER, ID) {}

Texture AssetManager::loadTexture(const std::string &file) {
    Texture tex = {};

    bool is16bit = stbi_is_16_bit(file.c_str());

    if (is16bit) {
        // 16 bit channels

        int width, height, numChannels;

        stbi_set_flip_vertically_on_load(true);
        uint8_t *data = (uint8_t *) stbi_load_16(file.c_str(), &width, &height, &numChannels, 0);

        // check if the texture could be loaded
        if(data == nullptr) {
            tex.data = nullptr;
            return tex;
        }

        //TODO: handle all formats
        assert(numChannels == 1 || numChannels == 2 || numChannels == 4);
        tex.format = (numChannels == 1) ? GRAYSCALE_16 : (numChannels == 3) ? RGB_16 : RGBA_16;
        tex.width = width;
        tex.height = height;
        tex.data = (char *) data;
    } else {
        // 8 bit channels

        int width, height, numChannels;

        stbi_set_flip_vertically_on_load(true);
        uint8_t *data = stbi_load(file.c_str(), &width, &height, &numChannels, 0);

        // check if the texture could be loaded
        if(data == nullptr) {
            tex.data = nullptr;
            return tex;
        }

        //TODO: handle all formats
        assert(numChannels == 1 || numChannels == 3 || numChannels == 4);
        tex.format = (numChannels == 1) ? GRAYSCALE_8 : (numChannels == 3) ? RGB_8 : RGBA_8;
        tex.width = width;
        tex.height = height;
        tex.data = (char *) data;
    }

    return tex;
}

AssetID AssetManager::loadObj(const std::string &dir, const std::string &file) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;

    // todo: nol: removed warn due to deprecation of parameter
    // todo: nol: moved {mtl_basedir} to parameters
    std::string new_dir = dir + "\\"; // dir requires a concatenated /
    std::string file_name = dir + "\\" + file;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file_name.c_str(), new_dir.c_str());

    if (!ret) {
        ls_log::log(LOG_ERROR, err.c_str());
        return {INVALID, 0};
    }

    Model result(generateNewID());

    //NOTE: the structure of the OBJ file has to altered in order to fit the desired indexing format. Instead
    //      of indexing position/normal/uv individually, we need a single index to a vertex. To this end, all
    //      unique combinations of pos/norm/uv are condensed into individual vertices, and indices are saved as
    //      indices into this set of unique vertices.

    std::vector<std::tuple<int32_t, int32_t, int32_t>> uniqueVertices;

    //TODO: what about the names of the individual submeshes as described by the obj file?
    result.mesh.name = file;

    //TODO: probably need an extra mesh for material index -1
    for (int i = 0; i < materials.size(); i++) {
        MaterialSubMesh subMesh = {};
        subMesh.materialIndex = i;
        result.mesh.materialSubMeshes.emplace_back(subMesh);
    }

    for (auto &shape: shapes) {
        glm::vec3 tangent;
        glm::vec3 biTangent;

        // convert indexing
        for (uint32_t i = 0; i < shape.mesh.indices.size(); i++) {
            auto &index = shape.mesh.indices[i];

            std::tuple<int32_t, int32_t, int32_t> vertexIndices = std::make_tuple(index.vertex_index, index.normal_index,
                                                                      index.texcoord_index);

            // NOTE: this could be optimized quite easily, by using a better data structure
            int32_t vertexIndex = -1;
            for (uint32_t j = 0; j < uniqueVertices.size(); j++) {
                if (uniqueVertices[j] == vertexIndices) {
                    vertexIndex = j;
                    break;
                }
            }

            // compute the tangent and bi-tangent for every triangle
            if (i % 3 == 0) {
                glm::vec3 &v0 = (glm::vec3 &) attrib.vertices[shape.mesh.indices[i].vertex_index * 3];
                glm::vec3 &v1 = (glm::vec3 &) attrib.vertices[shape.mesh.indices[i + 1].vertex_index * 3];
                glm::vec3 &v2 = (glm::vec3 &) attrib.vertices[shape.mesh.indices[i + 2].vertex_index * 3];

                glm::vec2 &uv0 = (glm::vec2 &) attrib.texcoords[shape.mesh.indices[i].texcoord_index * 2];
                glm::vec2 &uv1 = (glm::vec2 &) attrib.texcoords[shape.mesh.indices[i + 1].texcoord_index * 2];
                glm::vec2 &uv2 = (glm::vec2 &) attrib.texcoords[shape.mesh.indices[i + 2].texcoord_index * 2];

                glm::vec3 dPos1 = v1 - v0;
                glm::vec3 dPos2 = v2 - v0;

                glm::vec2 dUV1 = uv1 - uv0;
                glm::vec2 dUV2 = uv2 - uv0;

                float r = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);

                tangent = (dPos1 * dUV2.y - dPos2 * dUV1.y) * r;
                biTangent = (dPos2 * dUV1.x - dPos1 * dUV2.x) * r;
            }

            int32_t faceMaterialIndex = shape.mesh.material_ids[i / 3];
            if (vertexIndex == -1) {
                uniqueVertices.emplace_back(
                        std::make_tuple(index.vertex_index, index.normal_index, index.texcoord_index));

                Vertex v = {};
                v.position = {attrib.vertices[index.vertex_index * 3],
                              attrib.vertices[index.vertex_index * 3 + 1],
                              attrib.vertices[index.vertex_index * 3 + 2]};
                v.normal = {attrib.normals[index.normal_index * 3],
                            attrib.normals[index.normal_index * 3 + 1],
                            attrib.normals[index.normal_index * 3 + 2]};
                v.uv = {attrib.texcoords[index.texcoord_index * 2],
                        attrib.texcoords[index.texcoord_index * 2 + 1]};
                v.tangent = tangent;
                v.biTangent = biTangent;

                result.vertices.emplace_back(v);

                //TODO: robustness when materialIndex = -1
                result.mesh.materialSubMeshes[faceMaterialIndex].indices.emplace_back(uniqueVertices.size() - 1);
            } else {
                result.vertices[vertexIndex].tangent = result.vertices[vertexIndex].tangent + tangent;
                result.vertices[vertexIndex].biTangent = result.vertices[vertexIndex].tangent + biTangent;

                result.mesh.materialSubMeshes[faceMaterialIndex].indices.emplace_back(vertexIndex);
            }
        }
    }

    for (const auto &mat: materials) {
        Material newMaterial = {};
        newMaterial.name = mat.name;

        bool usesAlbedoTexture = mat.diffuse_texname != "";
        bool usesRoughnessTexture = mat.specular_highlight_texname != "";
        // todo: nol: {reflection_texname} is deprecated
        bool usesMetallicTexture = mat.specular_texname != "";
        bool usesNormalTexture = mat.bump_texname != "";

        if(usesAlbedoTexture) {
            newMaterial.albedoTexture = loadTexture("../res/objects/" + mat.diffuse_texname);
        } else {
            newMaterial.albedo.x = mat.diffuse[0];
            newMaterial.albedo.y = mat.diffuse[1];
            newMaterial.albedo.z = mat.diffuse[2];
        }

        if(usesRoughnessTexture) {
            newMaterial.roughnessTexture = loadTexture("../res/objects/" + mat.specular_highlight_texname);
        } else {
            //TODO: gruesome hack for blender, instead should use PBR extension but blender doesn't support that
            //https://developer.blender.org/diffusion/BA/browse/master/io_scene_obj/export_obj.py
            newMaterial.roughness = -(sqrtf(mat.shininess) / 30 - 1);
        }

        if(usesMetallicTexture) {
            // todo: nol: {reflection_texname} is deprecated see above, this branch is never taken
//            newMaterial.metallicTexture = loadTexture("../res/objects/" + mat.reflection_texname);
        } else {
            //TODO: gruesome hack for blender, instead should use PBR extension but blender doesn't support that
            if(mat.ambient[0] == 1.0 && mat.ambient[1] == 1.0 && mat.ambient[2] == 1.0) {
                newMaterial.metallic = 0;
            } else {
                newMaterial.metallic = mat.ambient[0];
            }
        }

        if(usesNormalTexture) {
            newMaterial.normalMap = loadTexture("../res/objects/" + mat.bump_texname);
        }

        result.materials.emplace_back(newMaterial);
    }

    this->models.emplace(result.assetID.ID, result);
    return result.assetID;
}

uint64_t AssetManager::generateNewID() {
    return indexGeneratorCounter++;
}

AssetID AssetManager::loadShader(const std::string &vertexShader, const std::string &fragmentShader) {
    std::ifstream vertexFileStream(vertexShader);
    std::string vertexFileContent((std::istreambuf_iterator<char>(vertexFileStream)),
                                  (std::istreambuf_iterator<char>()));

    std::ifstream fragmentFileStream(fragmentShader);
    std::string fragmentFileContent((std::istreambuf_iterator<char>(fragmentFileStream)),
                                    (std::istreambuf_iterator<char>()));

    Shader result(generateNewID());
    result.vertexShaderText = vertexFileContent;
    result.fragmentShaderText = fragmentFileContent;

    shaders.emplace(result.assetID.ID, result);
    return result.assetID;
}

Model *AssetManager::getModel(AssetID id) {
    assert(id.type == MODEL);
    //TODO: containment check
    return &models.at(id.ID);
}

Shader *AssetManager::getShader(AssetID id) {
    assert(id.type == SHADER);
    //TODO: containment check
    return &shaders.at(id.ID);
}
