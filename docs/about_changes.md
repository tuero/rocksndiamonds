
# Changes from the original client
As much as the original engine source code has been kept in tact, but several changes has been made to allow for controllers to interface with the engine.

## Compiling the project
The original `Makefile's` are kept in this project, but are prefixed by an `_`. The project won't compile using those `Makefile's`, but are still kept for completeness. This project uses `CMake`, as I found its easier to add features and check for requirements.

During compile time, there are [build scripts](../build-scripts/create_elemet_defs.pl) which set the elements' properties inside the engine source code. Shell scripts have been added to ease calling the build script inside CMake.

Finally, the original project would create element images using sprite sheets. I found that the original source compile would sometimes break on this step, and was not 100% reliable on my personal system. The needed files are already included, and that step is now skipped so that compiling is portable. 

## Main entry and game loop
Before the game loop starts, [hooks](../src/main.c) are added to interface with our project code to set the engine state depending on the command line arguments given. For example, you can start right away into a level/levelset without having to use the GUI to manually select the level/levelset and start the game. 

Inside the [game loop](../src/events.c), calls to interface with the controller are made. If a controller has been set by the command line argument `-controller <CONTROLLER>`, then the action to take by the agent are asked by the interface. 

There is also a `spriteIDs` array object with tracks each object with a unique identifier. This allows controllers to differentiate between objects of the same type, while they fall/move throughout the map (`ContinueMoving`). In the [game actions](../src/game.c) functions, we remove spriteIDs when objects are removed from the map (`RemoveField` and `Explode`), and add spriteIDs on level start, and every time an element change occurs (`CreateFieldExt`).

## Headless version
To create a non-GUI client that doesn't depend on SDL2, and subsequently allow for a faster engine access time, a headless variant is also built during compile time (binary `rocksndiamonds_headless`). Many `#ifndef's` are placed in the engine code that deals with graphics/sounds objects, which are removed during compile time when the headless binary is being built. 