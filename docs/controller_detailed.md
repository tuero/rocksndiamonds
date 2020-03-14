
# Detailed Controller Features
If you don't already have a basic controller setup, see the [Creating a basic controller](controller.md) section.

## Interleaving planning and execution
While not every type of controller needs to do so, it can be useful to interleave planning and execution. Since it takes `enginestate::getEngineUpdateRate()` (8) game engine ticks (with each game tick equating to 20ms in real time) to move the agent from one gridcell to another after receiving the given action, we can utilize the time during those 8 game ticks to plan the next action take after the current one is complete. 

For example, consider the MCTS controller. At game tick `n`, the controller sends the action `left`. For the next 8 game ticks, the agent will be in the middle of going from its current gridcell the cell on the left. For each of these 8 game ticks, the controller can plan the next action to send on game tick `n+8`. In the case of MCTS, we can set our planning tree to the state which would occur after the agent has completed its current move. The idea is that once we are at game tick `n+8`, our MCTS agent would have already spent 8 game ticks of planning to determine the next move to make immediately.

The `plan()` method can be overridden to allow your agent to execute code for every game engine tick. For a full example, see [`./src/ai/controller/mcts/mcts.cpp`](../src/ai/controller/mcts/mcts.cpp).

## Initializing the controller features before starting the game
The `handleLevelStart()` method is called when the level is loaded for the first time (not on subsequent resets), and here is where you can specify any actions before the engine start asking you for the moves to make.

For example, if we have a function `readValuesFromConfig()` which reads values from a configuration file which we want set before the controller starts sending actions:
```cpp
class MyController : public BaseController {
    ...
    void handleLevelStart() override {
        readValuesFromConfig();
    }
}
```

## Enabling level replays on level failure
By default, if the level is failed (agent dies, time runs out, etc.), the program terminates. However, you can request that the level is played again if your controller wants to try and solve over many runs, such as an RL method. 

To enable level retries, the `retryOnLevelFail()` method should be overwritten to return `true`.
```cpp
class MyController : public BaseController {
    ...
    bool retryOnLevelFail() const override {
        return true;
    }
}
```
When the agent fails a level, two methods are called:
- `handleLevelRestartBefore()`: This is called before the level is reloaded. Any cleanup actions that you want to happen **before** the level is reloaded should be placed here.
- `handleLevelRestartAfter()`: This is called after the level is reloaded, but before the engine asks your controller for actions. If you have initializations for your controller (similar to `handleLevelStart()`), place them in here. Note that it is suggested to keep actions which only need to be called once (such as reading configuration files) in the `handleLevelStart()` method.

An example of the control flow the engine uses is detailed below:
```cpp
if (baseController_.get()->retryOnLevelFail()) {
    baseController_.get()->handleLevelRestartBefore();
    levelinfo::restartLevel();                           // Here is where the engine reloads the level.
    baseController_.get()->handleLevelRestartAfter();
}
```

It is recommended to have a method which calls the code required for every level start (both first start and on restarts).
```cpp
class MyController : public BaseController {
    ...
    bool retryOnLevelFail() const override {
        return true;
    }

    // Private method to handle requirements for ALL restarts, including first level start
    void initializationForEveryLevelStart() {
        ...
    }

    // Only ran during the first level start, not on subsequent restarts
    void handleLevelStart() override {
        readValuesFromConfig();
        initializationForEveryLevelStart();
    }

    void handleLevelRestartBefore() override {
        cleanup();
    }

    // Called on every restart (but not first level start)
    void handleLevelRestartAfter() override {
        initializationForEveryLevelStart();
    }
}
```