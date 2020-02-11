
# Binary Versions
There are two different binary versions of the Rocks'n'Diamonds game. 

## `rocksndiamonds`
This is the standard Rocks'n'Diamonds game client with a GUI, which relies on `SDL2`. All of the features such as using your own controller work here. Since graphics elements are being updated in the main game loop along with a frame delay, the controller will run in real time, which can be slow if you want to quickly test against multiple levels. There will also be a slight performance hit if your controller is simulation based such as MCTS.

## `rocksndiamonds_headless`
This is the same as the above, but with any graphics elements stripped out, and runs as a standard command line program. This doesn't depend on `SDL2`, so installing it is not required. Since graphics elements are not used, controllers can run faster than real time, which is useful for replaying a level multiple times. Simulation based controllers will also see around a 2x performance increase in terms of number of simulations per second.

Since we cannot view the AI agent and see whether the controller was successful or not, replay files can be created which saves each action the controller tells the agent to perform. These replay files can then be loaded in the `rocksndiamonds` version to view the actions the AI agent took.