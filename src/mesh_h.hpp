#include <glad/gl.h>
#include <stddef.h>

class Mesh {
public:
  Mesh(const float *vertices, size_t sizeInBytes) {
    glGenVertexArrays(1, &lightSourceVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(lightSourceVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);

    // change back to 5 for textures \/
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
    //                       (void *)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
  }

  void setupObjectVAO() {
    glGenVertexArrays(1, &objectVAO);
    glBindVertexArray(objectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
  }

  void bindLightSource() const { glBindVertexArray(lightSourceVAO); }
  void bindObject() const { glBindVertexArray(objectVAO); }

  ~Mesh() {
    glDeleteVertexArrays(1, &lightSourceVAO);
    glDeleteVertexArrays(1, &objectVAO);
    glDeleteBuffers(1, &VBO);
  }

private:
  unsigned int lightSourceVAO{}, VBO{};
  unsigned int objectVAO{};
};
