#pragma once

#include "Material.hpp"
#include "MeshIndexed.hpp"
#include <memory>

class MeshRendererIndexed {
public:
  MeshRendererIndexed(std::shared_ptr<MeshIndexed> meshData,
                      std::shared_ptr<Material> mat)
      : mesh(meshData), material(mat) {}

  void render() const {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    for (unsigned int i = 0; i < mesh->textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      std::string number;
      std::string name = mesh->textures[i].type;
      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number = std::to_string(specularNr++);
      else if (name == "texture_normal")
        number = std::to_string(normalNr++);

      material->getShader()->setInt(("material." + name + number).c_str(), i);
      glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }

    material->use();
    mesh->bind();
    glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
    mesh->unbind();

    glActiveTexture(GL_TEXTURE0);
  }

  Material *getMaterial() const { return material.get(); }
  MeshIndexed *getMesh() const { return mesh.get(); }

private:
  std::shared_ptr<MeshIndexed> mesh;
  std::shared_ptr<Material> material;
};
