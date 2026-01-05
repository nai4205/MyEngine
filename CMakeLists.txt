cmake_minimum_required(VERSION 3.16)
project(LearnOpenGL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES 
  src/*.cpp
  src/*.c
  src/*.hpp
)

add_executable(app 
  ${SOURCES}
)

target_include_directories(app PRIVATE
  include
  ~/Downloads/glfw-3.4/include/
  ~/Downloads/assimp/include/assimp/
)

target_link_directories(app PRIVATE
  ~/Downloads/glfw-3.4/build/src/
)



target_link_libraries(app
  glfw
  assimp
  GL
  dl
  pthread
)
