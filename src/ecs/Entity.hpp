#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <cstdint>

// Entity is just a unique ID
using Entity = uint32_t;

// Reserved value for null/invalid entity
constexpr Entity NULL_ENTITY = 0;

#endif // ENTITY_HPP
