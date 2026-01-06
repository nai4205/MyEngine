#ifndef SYSTEM_HPP
#define SYSTEM_HPP

class World;

class System {
public:
  virtual ~System() = default;

  virtual void update(float &deltaTime) {}

  virtual void render() {}
};

#endif // SYSTEM_HPP
