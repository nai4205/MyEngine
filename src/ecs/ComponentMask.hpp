#ifndef COMPONENT_MASK_HPP
#define COMPONENT_MASK_HPP

#include <bitset>
#include <cstdint>

constexpr size_t MAX_COMPONENTS = 64;

// Component type ID (0-63)
using ComponentTypeId = uint8_t;

// Bitmask representing which components an entity has
// Each bit corresponds to a component type
using ComponentMask = std::bitset<MAX_COMPONENTS>;

#endif // COMPONENT_MASK_HPP
