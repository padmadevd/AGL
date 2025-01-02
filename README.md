# AGL
Another Game Library.

# About
- AGL(Another Game Library) is a c++ 2D game library with SDL2 backend
- It can compile to almost any OS, Thanks to SDL2 :). But it is developed only with android, linux, and windows in mind.
- In Android, it uses OpenGLES and for linux and windows it uses openGL.

# Dependencies
- SDL2, SDL-mixer, SDL-ttf, SDL-image, glm, glad

# Building
- Refer the makefile for building.
- Install SDL2, glad, glm **(because the dependencies are not given)** and specify the include path.
- The pre-build libray files expect SDL2-ttf (**can be downloaded from the official site**) are present in the AGL\Build\.. directory.
- ***refer the header files for API reference.***
- **test.exe** present in Build/.. is a sample app to demonstrate the concave to convex polygon conversion.
    - run the test.exe
    - **DOUBLE TAP and DRAG** with left mouse button to draw a shape
    - then hit **ENTER** to calculate a convex polygon.
    - hit **SPACE** to again start drawing.
- **main.exe** is a basic app. (just run to check out).

# Notes
- ***download sdl2-ttf.dll*** from the official site and place it in the folder ***AGL\Build\..***
- **the physics is imcomplete.**

  # Future plans
  - Refining the API and implementation.
  - 2D bone system & physics based 2D bone system.
  - A complete physics system using chipmunk2D as backend.
