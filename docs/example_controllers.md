
# Example Controllers

## Default
The [default](../src/ai/controller/default/default.h) controller shows the minimum requirements to have a controller.

## MCTS
The [MCTS](../src/ai/controller/mcts/mcts.h) controller provided is a simple MCTS variant. This is a good example to see how you can interleave planning and execution. The controller operates by planning on the state which would occur **after** the current action the agent is in the middle of executing is finished. 

During the level start, the controller sends a `Action::noop` as its first action, and simulates the saved state forward. Since it takes `ENGINE_RESOLUTION` (8) steps to complete the `Action::noop` action, the controller has 8 game ticks (20ms * 8 ticks) to run MCTS and determine the next action to send. Once the current `Action::noop` is finished, the controller sends the best `Action` MCTS found, simulates its saved state forward to the state what would occur after the sending `Action` is complete. This process is repeated, with the controller than running MCTS while the best action is currently being executed.