#pragma once

#include "ResourceManager.hpp"

#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/NameComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/World.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

class ModelLoader {
public:
  static std::vector<Entity> load(World &world, const std::string &path,
                                  uint32_t shaderProgram,
                                  const glm::vec3 &position = glm::vec3(0.0f),
                                  const glm::vec3 &scale = glm::vec3(1.0f)) {
    std::vector<Entity> entities;
    std::string directory = path.substr(0, path.find_last_of('/'));

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
                  aiProcess_OptimizeMeshes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
      return entities;
    }

    processNode(world, scene->mRootNode, scene, directory, shaderProgram,
                position, scale, entities);

    return entities;
  }

private:
  static void processNode(World &world, aiNode *node, const aiScene *scene,
                          const std::string &directory, uint32_t shaderProgram,
                          const glm::vec3 &position, const glm::vec3 &scale,
                          std::vector<Entity> &entities) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      Entity entity = processMesh(world, mesh, scene, directory, shaderProgram);

      auto *transform = world.getComponent<TransformComponent>(entity);
      if (transform) {
        transform->position = position;
        transform->scale = scale;
      }

      entities.push_back(entity);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(world, node->mChildren[i], scene, directory, shaderProgram,
                  position, scale, entities);
    }
  }

  static Entity processMesh(World &world, aiMesh *mesh, const aiScene *scene,
                            const std::string &directory,
                            uint32_t shaderProgram) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string meshName = mesh->mName.C_Str();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      Vertex vertex;
      vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                  mesh->mVertices[i].z);

      if (mesh->HasNormals()) {
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                  mesh->mNormals[i].z);
      }

      if (mesh->mTextureCoords[0]) {
        vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
                                     mesh->mTextureCoords[0][i].y);
      } else {
        vertex.TexCoords = glm::vec2(0.0f);
      }

      vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace &face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        indices.push_back(face.mIndices[j]);
      }
    }

    auto &resources = ResourceManager::instance();
    MeshData meshData = resources.createIndexedMesh(vertices, indices);

    Entity entity = world.createEntity();

    TransformComponent transform;
    world.addComponent(entity, transform);

    NameComponent name(meshName);
    world.addComponent(entity, name);

    MeshComponent meshComp;
    meshComp.vao = meshData.vao;
    meshComp.vertexCount = meshData.vertexCount;
    meshComp.indexCount = meshData.indexCount;
    world.addComponent(entity, meshComp);

    MaterialComponent material;
    material.shaderProgram = shaderProgram;

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];

      if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        std::string texPath = directory + '/' + str.C_Str();
        material.textures[0] = resources.loadTexture(texPath);
        material.useTextures = true;
      }

      if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
        std::string texPath = directory + '/' + str.C_Str();
        material.textures[1] = resources.loadTexture(texPath);
      }
    }

    world.addComponent(entity, material);

    return entity;
  }
};
