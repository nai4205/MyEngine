#ifndef MODEL_HPP
#define MODEL_HPP

#include "../GameObject.hpp"
#include "Material.hpp"
#include "MeshIndexed.hpp"
#include "ModelTextureLoader.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Model {
public:
  Model(const std::string &path, std::shared_ptr<Shader> shader)
      : modelShader(shader) {
    loadModel(path);
  }

  const std::vector<GameObject> &getGameObjects() const { return gameObjects; }

  std::vector<GameObject> moveGameObjects() { return std::move(gameObjects); }

private:
  std::vector<GameObject> gameObjects;
  std::shared_ptr<Shader> modelShader;
  std::string directory;
  std::unordered_map<std::string, Texture>
      texturesLoaded; // Cache loaded textures

  void loadModel(const std::string &path) {
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
                  aiProcess_SplitLargeMeshes | aiProcess_OptimizeMeshes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
      return;
    }

    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
  }

  void processNode(aiNode *node, const aiScene *scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
      aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
      gameObjects.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
      processNode(node->mChildren[i], scene);
    }
  }

  GameObject processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      Vertex vertex;

      vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y,
                                  mesh->mVertices[i].z);

      if (mesh->HasNormals()) {
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                  mesh->mNormals[i].z);
      } else {
        vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
      }

      if (mesh->mTextureCoords[0]) {
        vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
                                     mesh->mTextureCoords[0][i].y);
      } else {
        vertex.TexCoords = glm::vec2(0.0f, 0.0f);
      }

      vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
      aiFace face = mesh->mFaces[i];
      for (unsigned int j = 0; j < face.mNumIndices; j++) {
        indices.push_back(face.mIndices[j]);
      }
    }

    if (mesh->mMaterialIndex >= 0) {
      aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];

      std::vector<Texture> diffuseMaps = loadMaterialTextures(
          mat, aiTextureType_DIFFUSE, "texture_diffuse", scene);
      textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

      std::vector<Texture> specularMaps = loadMaterialTextures(
          mat, aiTextureType_SPECULAR, "texture_specular", scene);
      textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

      std::vector<Texture> normalMaps = loadMaterialTextures(
          mat, aiTextureType_HEIGHT, "texture_normal", scene);
      textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    auto meshIndexed =
        std::make_shared<MeshIndexed>(vertices, indices, textures);

    auto material = std::make_shared<Material>(modelShader);
    material->setBool("material.useTex", !textures.empty());

    GameObject obj(mesh->mName.C_Str());
    obj.addComponent(meshIndexed, material);

    return obj;
  }

  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            const std::string &typeName,
                                            const aiScene *scene) {
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);

      std::string texturePath = str.C_Str();
      if (texturePath.empty())
        continue;

      std::shared_ptr<Texture2D> texture2D;
      std::string cacheKey;

      cacheKey = directory + '/' + texturePath;

      if (texturesLoaded.find(cacheKey) != texturesLoaded.end()) {
        textures.push_back(texturesLoaded[cacheKey]);
        continue;
      }

      texture2D = ModelTextureLoader::loadTexture(texturePath, directory);

      Texture texture;
      texture.id = texture2D->getID();
      texture.type = typeName;
      textures.push_back(texture);

      texturesLoaded[cacheKey] = texture;
    }

    return textures;
  }
};

#endif
