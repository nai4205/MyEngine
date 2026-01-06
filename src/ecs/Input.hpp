#ifndef INPUT_HPP
#define INPUT_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_set>

// Input state manager - stores current frame's input state
// Can be accessed from any system without needing the GLFWwindow
class Input {
private:
  std::unordered_set<int> keysPressed;
  std::unordered_set<int> keysJustPressed;
  std::unordered_set<int> keysJustReleased;

  glm::vec2 mousePosition = glm::vec2(0.0f);
  glm::vec2 mouseDelta = glm::vec2(0.0f);
  glm::vec2 lastMousePosition = glm::vec2(0.0f);
  float scrollDelta = 0.0f;

  bool firstMouse = true;

public:
  // Called at the start of each frame to reset per-frame state
  void newFrame() {
    keysJustPressed.clear();
    keysJustReleased.clear();
    mouseDelta = glm::vec2(0.0f);
    scrollDelta = 0.0f;
  }

  // ========== KEYBOARD ==========

  void setKeyPressed(int key, bool pressed) {
    if (pressed) {
      if (keysPressed.find(key) == keysPressed.end()) {
        keysJustPressed.insert(key);
      }
      keysPressed.insert(key);
    } else {
      keysPressed.erase(key);
      keysJustReleased.insert(key);
    }
  }

  bool isKeyPressed(int key) const {
    return keysPressed.find(key) != keysPressed.end();
  }

  bool isKeyJustPressed(int key) const {
    return keysJustPressed.find(key) != keysJustPressed.end();
  }

  bool isKeyJustReleased(int key) const {
    return keysJustReleased.find(key) != keysJustReleased.end();
  }

  // ========== MOUSE ==========

  void setMousePosition(float x, float y) {
    glm::vec2 newPos(x, y);

    if (firstMouse) {
      lastMousePosition = newPos;
      firstMouse = false;
    }

    mouseDelta = newPos - lastMousePosition;
    lastMousePosition = newPos;
    mousePosition = newPos;
  }

  glm::vec2 getMousePosition() const { return mousePosition; }

  glm::vec2 getMouseDelta() const { return mouseDelta; }

  void setScrollDelta(float delta) { scrollDelta = delta; }

  float getScrollDelta() const { return scrollDelta; }

  void resetMouseDelta() {
    mouseDelta = glm::vec2(0.0f);
    firstMouse = true;
  }
};

#endif // INPUT_HPP
