#pragma once

#include "Entity.hpp"
#include <cassert>
#include <vector>

struct IComponentArray {
  virtual ~IComponentArray() = default;
  virtual void entityDestroyed(Entity e) = 0;
};

template <typename T> class ComponentArray : public IComponentArray {
private:
  std::vector<T> dense;
  std::vector<Entity> denseEntities;
  std::vector<size_t> sparse; // indexed by Entity

public:
  void insert(Entity e, T component) {
    if (e >= sparse.size()) {
      sparse.resize(e + 1, SIZE_MAX);
    }

    if (has(e)) {
      dense[sparse[e]] = component;
      return;
    }

    sparse[e] = dense.size();
    denseEntities.emplace_back(e);
    dense.emplace_back(component);
  }

  void remove(Entity e) {
    if (!has(e))
      return;

    size_t index = sparse[e];
    size_t last = dense.size() - 1;

    dense[index] = dense[last];
    denseEntities[index] = denseEntities[last];
    sparse[denseEntities[index]] = index;

    dense.pop_back();
    denseEntities.pop_back();
    sparse[e] = SIZE_MAX;
  }

  T *get(Entity e) { return has(e) ? &dense[sparse[e]] : nullptr; }

  bool has(Entity e) const {
    return e < sparse.size() && sparse[e] != SIZE_MAX;
  }

  auto begin() { return dense.begin(); }
  auto end() { return dense.end(); }

  void entityDestroyed(Entity e) override { remove(e); }
};
