#pragma once

#include "ResourceManager.hpp"

#include "../animation/Animation.hpp"
#include "../animation/BoneInfo.hpp"
#include "../components/AnimationComponent.hpp"
#include "../components/MaterialComponent.hpp"
#include "../components/MeshComponent.hpp"
#include "../components/NameComponent.hpp"
#include "../components/ParentComponent.hpp"
#include "../components/SkeletonComponent.hpp"
#include "../components/TransformComponent.hpp"
#include "../ecs/World.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <iostream>
#include <map>
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

  static std::vector<Entity>
  loadWithAnimation(World &world, const std::string &path,
                    uint32_t animatedShaderProgram,
                    const glm::vec3 &position = glm::vec3(0.0f),
                    const glm::vec3 &scale = glm::vec3(1.0f)) {
    std::vector<Entity> entities;
    std::string directory = path.substr(0, path.find_last_of('/'));

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
            aiProcess_OptimizeMeshes |
            aiProcess_LimitBoneWeights); // Limit to 4 bones per vertex

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
      return entities;
    }

    // Check if model has animations
    bool hasAnimations = scene->mNumAnimations > 0;

    if (!hasAnimations) {
      std::cerr << "WARNING: No animations found in " << path << std::endl;
      return load(world, path, animatedShaderProgram, position, scale);
    }

    // Shared data for all meshes in this model
    std::map<std::string, BoneInfo> boneInfoMap;
    int boneCounter = 0;

    processNodeWithAnimation(world, scene->mRootNode, scene, directory,
                            animatedShaderProgram, position, scale, entities,
                            boneInfoMap, boneCounter);

    // Create a parent entity to hold the animation component
    Entity animatedModel = world.createEntity();

    TransformComponent transform;
    transform.position = position;
    transform.scale = scale;
    world.addComponent(animatedModel, transform);

    NameComponent name(path.substr(path.find_last_of('/') + 1));
    world.addComponent(animatedModel, name);

    // Add animation component with all animations from the file
    AnimationComponent animComp;
    animComp.boneInfoMap = boneInfoMap;
    animComp.boneCounter = boneCounter;

    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
      aiAnimation *anim = scene->mAnimations[i];
      std::string animName = anim->mName.C_Str();
      if (animName.empty()) {
        animName = "Animation_" + std::to_string(i);
      }

      // Pass the boneInfoMap so animation uses the same bone IDs as the mesh vertices
      auto animation = std::make_shared<Animation>(path, scene, i, boneInfoMap, boneCounter);
      animComp.addAnimation(animName, animation);

      std::cout << "Loaded animation: " << animName << std::endl;
    }

    world.addComponent(animatedModel, std::move(animComp));

    // Link all mesh entities to the parent animation entity
    for (Entity meshEntity : entities) {
      ParentComponent parentComp(animatedModel);
      world.addComponent(meshEntity, parentComp);
    }

    entities.push_back(animatedModel);

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
        std::string texFilename = str.C_Str();
        // Extract just the filename from embedded path (handle both / and \)
        size_t lastSlash = texFilename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
          texFilename = texFilename.substr(lastSlash + 1);
        }
        std::string texPath = directory + '/' + texFilename;
        material.textures[0] = resources.loadTexture(texPath);
        material.useTextures = true;
      }

      if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
        std::string texFilename = str.C_Str();
        // Extract just the filename from embedded path (handle both / and \)
        size_t lastSlash = texFilename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
          texFilename = texFilename.substr(lastSlash + 1);
        }
        std::string texPath = directory + '/' + texFilename;
        material.textures[1] = resources.loadTexture(texPath);
      }
    }

    world.addComponent(entity, material);

    return entity;
  }

  // Animation helper methods
  static void processNodeWithAnimation(World &world, aiNode *node,
                                      const aiScene *scene,
                                      const std::string &directory,
                                      uint32_t shaderProgram,
                                      const glm::vec3 &position,
                                      const glm::vec3 &scale,
                                      std::vector<Entity> &entities,
                                      std::map<std::string, BoneInfo> &boneInfoMap,
                                      int &boneCounter) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      Entity entity = processMeshWithAnimation(world, mesh, scene, directory,
                                              shaderProgram, boneInfoMap,
                                              boneCounter);

      auto *transform = world.getComponent<TransformComponent>(entity);
      if (transform) {
        transform->position = position;
        transform->scale = scale;
      }

      entities.push_back(entity);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNodeWithAnimation(world, node->mChildren[i], scene, directory,
                              shaderProgram, position, scale, entities,
                              boneInfoMap, boneCounter);
    }
  }

  static Entity processMeshWithAnimation(
      World &world, aiMesh *mesh, const aiScene *scene,
      const std::string &directory, uint32_t shaderProgram,
      std::map<std::string, BoneInfo> &boneInfoMap, int &boneCounter) {
    std::vector<AnimatedVertex> vertices;
    std::vector<uint32_t> indices;
    std::string meshName = mesh->mName.C_Str();

    vertices.resize(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      vertices[i].Position = glm::vec3(mesh->mVertices[i].x,
                                      mesh->mVertices[i].y,
                                      mesh->mVertices[i].z);

      if (mesh->HasNormals()) {
        vertices[i].Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                      mesh->mNormals[i].z);
      }

      if (mesh->mTextureCoords[0]) {
        vertices[i].TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
                                         mesh->mTextureCoords[0][i].y);
      } else {
        vertices[i].TexCoords = glm::vec2(0.0f);
      }
    }

    extractBoneWeightForVertices(vertices, mesh, boneInfoMap, boneCounter);

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace &face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        indices.push_back(face.mIndices[j]);
      }
    }

    auto &resources = ResourceManager::instance();
    MeshData meshData = resources.createIndexedAnimatedMesh(vertices, indices);

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

    SkeletonComponent skeleton(boneCounter);
    world.addComponent(entity, skeleton);

    MaterialComponent material;
    material.shaderProgram = shaderProgram;

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];

      if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        std::string texFilename = str.C_Str();
        // Extract just the filename from embedded path (handle both / and \)
        size_t lastSlash = texFilename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
          texFilename = texFilename.substr(lastSlash + 1);
        }

        // Fix incorrect texture assignment: Dragon_Bump_Col2 should not be used as diffuse
        if (texFilename == "Dragon_Bump_Col2.jpg") {
          texFilename = "Dragon_ground_color.jpg";
        }

        std::string texPath = directory + '/' + texFilename;
        material.textures[0] = resources.loadTexture(texPath);
        material.useTextures = true;
      }

      if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
        std::string texFilename = str.C_Str();
        // Extract just the filename from embedded path (handle both / and \)
        size_t lastSlash = texFilename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
          texFilename = texFilename.substr(lastSlash + 1);
        }
        std::string texPath = directory + '/' + texFilename;
        material.textures[1] = resources.loadTexture(texPath);
      }
    }

    world.addComponent(entity, material);

    return entity;
  }

  static void extractBoneWeightForVertices(
      std::vector<AnimatedVertex> &vertices, aiMesh *mesh,
      std::map<std::string, BoneInfo> &boneInfoMap, int &boneCounter) {
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
      int boneID = -1;
      std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

      if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
        BoneInfo newBoneInfo;
        newBoneInfo.id = boneCounter;
        newBoneInfo.offset =
            convertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
        boneInfoMap[boneName] = newBoneInfo;
        boneID = boneCounter;
        boneCounter++;
      } else {
        boneID = boneInfoMap[boneName].id;
      }

      assert(boneID != -1);

      auto weights = mesh->mBones[boneIndex]->mWeights;
      int numWeights = mesh->mBones[boneIndex]->mNumWeights;

      for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
        int vertexId = weights[weightIndex].mVertexId;
        float weight = weights[weightIndex].mWeight;
        assert(vertexId < vertices.size());
        setVertexBoneData(vertices[vertexId], boneID, weight);
      }
    }
  }

  static void setVertexBoneData(AnimatedVertex &vertex, int boneID,
                                float weight) {
    for (int i = 0; i < 4; ++i) {
      if (vertex.BoneIDs[i] < 0) {
        vertex.Weights[i] = weight;
        vertex.BoneIDs[i] = boneID;
        break;
      }
    }
  }

  static glm::mat4 convertMatrixToGLMFormat(const aiMatrix4x4 &from) {
    glm::mat4 to;
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
  }
};
