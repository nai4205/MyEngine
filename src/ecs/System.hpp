#pragma once

class World;

class System {
public:
  virtual ~System() = default;

  virtual void update(float &deltaTime) {}

  virtual void render() {}
};
