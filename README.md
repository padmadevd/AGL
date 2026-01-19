# AGL  
Another Game Library

## About
AGL (Another Game Library) is a cross-platform **2D game development library written in C++**, built on top of **SDL2**.

- Designed to compile on almost any operating system thanks to SDL2  
- Actively developed with **Android, Linux, and Windows** in mind  
- Uses **OpenGL ES** on Android and **OpenGL** on Linux and Windows  
- Focuses on performance, modularity, and flexibility for custom game engines  

## Features
**Another Game Library | C++, SDL2 | Apr 2025**

- Cross-platform 2D game development library powered by SDL2  
- Rendering system with OpenGL / OpenGL ES support  
- Event handling and input management  
- Framebuffer utilities  
- Particle system support  
- GLSL shader integration  
- Modular design for easy integration into custom engines  
- Emphasis on performance and clean API design  

## Dependencies
- SDL2  
- SDL-mixer  
- SDL-ttf  
- SDL-image  
- glm  
- glad  

## Notes
- **SDL2-ttf binaries** must be obtained from the official SDL website  
- Some prebuilt library files (except SDL2-ttf) are included in the `AGL/Build` directory  
- **Physics system is incomplete**  

## Future Plans
- Refinement of the public API and internal implementation  
- 2D bone animation system  
- Physics-based 2D bone system  
- Full physics system using **Chipmunk2D** as the backend  

## Showcase : Rocket Dash
Rocket Dash is a small game made using AGL to showcase the features of the library.
![rocket dash screen record](/screenshots/rec.gif)
