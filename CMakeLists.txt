cmake_minimum_required(VERSION 3.17)
project(light_show)

set(CMAKE_CXX_STANDARD 14)

set(SOURCES src/main.cpp)

# build glad (before adding compile options)
add_subdirectory(${PROJECT_SOURCE_DIR}/external/glad-0.1.34)

# static link MinGW libraries
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# static link gdi32 required by GLFW
set(CMAKE_EXE_LINKER_FLAGS "-lgdi32 -static")

# add extra warnings
add_compile_options(-Wall -Wextra -pedantic)

# build executable
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})

# add include directories for tinyobjloader, nuklear, stb, and GLM
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/tinyobjloader)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/nuklear)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/stb)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/glm-0.9.9.8)

# add include directories for GLFW, and static link GLFW library
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/external/glfw-3.3.2.bin.WIN64/include)
target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCE_DIR}/external/glfw-3.3.2.bin.WIN64/lib-mingw-w64/libglfw3.a)

# link glad
target_link_libraries(${CMAKE_PROJECT_NAME} glad)