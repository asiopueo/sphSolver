cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
add_definitions(-std=c++11)
set(CXX_FLAGS "-Wall" "-pedantic")
set(CMAKE_CXX_FLAGS, "${CXX_FLAGS}")

project(sphSolver)
include_directories(${PROJECT_SRC_DIR}/src)
include_directories("/usr/include" "/usr/local/include" "/usr/include/freetype2")

add_executable(${PROJECT_NAME} src/main.cpp src/common.cpp src/physics.cpp src/density.cpp src/shaderLoader.cpp src/textureLoader.cpp src/marchingcubes.cpp src/renderer.cpp src/camera.cpp)
#target_include_directories(${PROJECT_NAME} PUBLIC "/usr/lib/nvidia-346" "/usr/lib/x86_64-linux-gnu/" "/usr/local/lib")
# Not sure how to explicilty link nvidia-346 and x86_64-linux-gnu:
target_link_libraries(${PROJECT_NAME} PUBLIC glfw GLEW GLU GL Xxf86vm Xrandr pthread Xi Xinerama X11 Xcursor freetype SOIL)
