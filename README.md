#### Building
Prerequisites:
* Windows 10
* Python (2 or 3)
* mingw-w64 x86_64 compiler

Steps:
* Download [GLFW 3.3.2](https://github.com/glfw/glfw/releases/tag/3.3.2) binaries for 64 bits Windows into directory `external/glfw-3.3.2.bin.WIN64`.
* Download [glad v0.1.34](https://github.com/Dav1dde/glad/releases/tag/v0.1.34) source code into directory `external/glad-0.1.34`.
* Download [GLM 0.9.9.8](https://github.com/g-truc/glm/releases/tag/0.9.9.8) source code into directory `external/glm-0.9.9.8`.
* Download [stb](https://github.com/nothings/stb) `stb_image.h` single header into directory `external/stb`.
* Download [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader/releases/tag/v1.0.6) single header into directory `external/tinyobjloader`.
* Download [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) single header into directory `external/nuklear`.
* Build using CMake.

#### Controls
*   Drag MMB to orbit around the camera's focal point.
*   Drag RMB to move the camera's focal point in the XZ-plane.
*   Scroll to change the distance of the camera to its focal point.
*   Press ESC to close the program.