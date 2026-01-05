#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Transform component - handles position, rotation, and scale
// Provides the model matrix for rendering
class Transform {
public:
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  Transform(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 rot = glm::vec3(0.0f),
            glm::vec3 scl = glm::vec3(1.0f))
      : position(pos), rotation(rot), scale(scl) {}

  glm::mat4 getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    // Apply transformations in order: scale -> rotate -> translate
    model = glm::translate(model, position);

    // Apply rotations (order: Y, X, Z)
    model = glm::rotate(model, glm::radians(rotation.y),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.x),
                        glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z),
                        glm::vec3(0.0f, 0.0f, 1.0f));

    model = glm::scale(model, scale);

    return model;
  }
};

#endif
