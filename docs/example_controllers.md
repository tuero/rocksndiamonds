
# Example Controllers

## Default
The [default](../src/ai/controller/default/default.h) controller shows the minimum requirements to have a controller.

## simple_pathing
The [simple_pathing](../src/ai/controller/simple_pathing/simple_pathing.h) controller is a simple controller which simply paths to diamonds and the exit. It will continue collecting diamonds until it has enough points which unlocks the exit door. The agent is not capable of complicated mechanics (i.e. pushing a rock, pathing around enemies), so it can really only solve the `classic_boulderdash` level. 

You can see this controller in action by using the following:
```shell
./rocksndiamonds -controller simple_pathing -levelset classic_boulderdash -loadlevel 0
```

## MCTS
The [MCTS](../src/ai/controller/mcts/mcts.h) controller provided is a simple MCTS variant. This is a good example to see how you can interleave planning and execution. The controller operates by planning on the state which would occur **after** the current action the agent is in the middle of executing is finished. 

During the level start, the controller sends a `Action::noop` as its first action, and simulates the saved state forward. Since it takes `ENGINE_RESOLUTION` (8) steps to complete the `Action::noop` action, the controller has 8 game ticks (20ms * 8 ticks) to run MCTS and determine the next action to send. Once the current `Action::noop` is finished, the controller sends the best `Action` MCTS found, simulates its saved state forward to the state what would occur after the sending `Action` is complete. This process is repeated, with the controller than running MCTS while the best action is currently being executed.