# GameEngine

A simple 3D game/demo written in C++ using OpenGL. This project demonstrates a small gameplay loop with physics-based player movement, collision handling, basic AI (aliens), HUD using ImGui, a skybox, and simple sound playback on Windows.

## Features

- OpenGL rendering pipeline with shader support
- Model loading (OBJ) and texture support
- Player physics (walking, running, jumping) and collision resolution
- Centralized `CollisionManager` and `Platform` objects
- Simple enemy AI (`Alien`) that patrols and can be stomped
- HUD and in-game objectives rendered with ImGui
- Skybox, sun light source and basic materials
- Simple cutscene mechanic (rocket launch)
- Sound playback on Windows using `PlaySound` (winmm)

## Project layout (important files)

- `main.cpp` - Application entry point and game loop
- `Objects/` - Game object classes (aliens, platforms, skybox, etc.)
- `Algorithms/` - Physics and collision helper code
- `Graphics/` - Window and rendering helpers
- `Resources/` - Models, textures and sounds used by the demo

## Controls

- `W`, `A`, `S`, `D` - Move
- Mouse - Look around 
- `Space` - Jump
- `Left Shift` / `Right Shift` - Run
- `E` - Interact / Pick up / Deposit items
- `Tab` - Lock / unlock mouse cursor
- `P` - Reset player to spawn position
- `Esc` - Exit

## Tasks / Objectives (in-game)

- Collect and deliver a plant to the spaceship
- Collect and deliver fuel to the spaceship
- Feed the dog by giving a treat
- Stomp an alien to collect a dropped treat
- When all tasks are complete, board the spaceship to trigger a cutscene

## Building

This project targets Windows and was developed using Visual Studio (MSVC). The project is configured to use C++14.

Prerequisites
- Visual Studio 2017/2019/2022 (or another C++ toolchain that supports MSVC and OpenGL)
- OpenGL headers/libraries (usually provided by the system)
- GLFW (windowing & input)
- GLM (math)
- An OBJ loader (project contains `MeshLoaderObj`) and texture loader used in source
- ImGui (already integrated in the project sources)

Typical steps
1. Open the solution/project (`.sln` / `.vcxproj`) in Visual Studio.
2. Ensure include/library paths for third-party libs (GLFW, GLM, glew/glad if used) are set.
3. Make sure `Resources/` is copied to the executable working directory (the project usually does this via post-build step).
4. Build and run the project. change to x86 if it doesnt run 

## Notes

- Sound playback currently uses Windows API (`PlaySoundA`) and therefore the demo is Windows-specific for audio.
- The collision manager has a simple AABB/OBB resolution system tailored for the player-as-a-point approach used in the demo.
- This a university project
