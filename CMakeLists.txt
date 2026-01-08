cmake_minimum_required(VERSION 3.16)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use pkg-config to find libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(ASSIMP REQUIRED assimp)

file(GLOB_RECURSE SOURCES
  src/*.cpp
  src/*.c
  src/*.hpp
)

add_executable(app
  ${SOURCES}
)

target_include_directories(app PRIVATE
  src/glad/include
  include
  ${GLFW_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS}
)

target_link_directories(app PRIVATE
  ${GLFW_LIBRARY_DIRS}
  ${ASSIMP_LIBRARY_DIRS}
)

target_link_libraries(app
  ${GLFW_LIBRARIES}
  ${ASSIMP_LIBRARIES}
  dl
  pthread
)

# Platform-specific OpenGL linking
if(APPLE)
  target_link_libraries(app "-framework OpenGL")
elseif(UNIX AND NOT APPLE)
  target_link_libraries(app GL)
endif()
