# Engine Information Helper
While it is possible to manually access the engine state from the original engine code provided by [Rocks'n'Diamonds*](https://www.artsoft.org/), a library of functions have been written for ease of use for your controller.
The following will give a brief overview of accessing information from the engine for your controller use.

## Including the necessary headers
The only header that needs to be included is [`engine_helper.h`](../src/ai/include/engine_helper.h). Including this will give access to each of the submodule engine helper functions. You should also use `using namespace enginehelper` so that you can shortname the available functions. As an example,
```cpp
// Not using shortnaming
enginetype::GridCell cell = enginehelper::actioninfo::getPlayerPosition();

// Using shortnaming
using namespace enginehelper;
enginetype::GridCell cell = actioninfo::getPlayerPosition();
```

## namespace `enginetype`
The `enginetype` namespace provides the common types which are used to interface with the engine. 
- `enum Action` defines the action/direction types which the agent takes or directions objects move in.
- `enginetype::GridCell` represents a tile cell in the engine. All functions which reference tiles in the game using `GridCell`.
- There are also many common engine item `int` definitions which correspond to their defined `int` code in the engine. For example, we can refer to a diamond element (56 in the engine) by using `enginetype::ELEMENT_DIAMOND`.

## namespace `actioninfo`
The `actioninfo` namespace provides helper functions relating directly to `Action's`, such as converting `Action's` to/from `string's`.

## namespace `elementproperty`
The `elementproperty` namespace provides helper functions relating to the properties of elements, which are usually defined as the element located in a given `GridCell`. Examples include
- item descriptions and scores (e.g. gems have a score which are earned when collected).
- status of the exit door, such as being open, closed, opening, or closing.
- whether a `GridCell` contains a temporary element or the element is moving.
- whether items have properties such as being a wall, a rock, is pushable (rocks), is collectible (gems, keys), or diggable (dirt).
- Some elements like empty space are walkable but not diggable, and dirt is diggable but not walkable (walkable being that it doesn't destroy elements as a result of the actions, like removing dirt). The `isActionMoveable` function is a catchall which checks if the agent can move into a given cell, regardless of being diggable, collectible, walkable, etc.

## namespace `gridaction`
The `gridaction` namespace provides helper functions relating to `Action's` taken and the `GridCell` involved in those actions:
- Checking if two `GridCell's` are neighbours.
- Getting the `GridCell` as a result of performing an `Action` from a given `GridCell`.
- Getting the `Action` which results in moving from one given `GridCell` to the other `GridCell`.

## namespace `gridinfo`
The `gridinfo` namespace provides helper functions relating to the information of `GridCell's` which are not focused on the actual property of the element in that cell:
- Distance between `GridCell's`, whether `GridCell's` are in bounds.
- The unique IDs of sprites.
- Counts of elements on the map.
- All empty `GridCell's`.
- The players current position.

## namespace `enginehash`
The `gridinfo` namespace provides helper functions relating to hashing states or `GridCell's`:
- Get a hash value of the current engine in the state.
- Get a hash value of a path (`vector`, `deque`, etc.) of `GridCell`.


## namespace `levelinfo`
The `gridinfo` namespace provides helper functions relating to the level/levelset. 
- Te number of gems needed to solve the level, and the number remaining (needed - current collected).
- The height/width of the level.
- The level number.

## namespace `enginestatus`
The `gridinfo` namespace provides helper functions relating to the current state of the engine.
- Whether the level is solved or failed.
- Getting the number of game engine ticks common movements. When the agent initiates a move action, it takes `enginestate::getEngineUpdateRate()` (being 8 in the standard game) game ticks to move the agent from one cell to another. 
- Setting/getting the agents action (useful if you want to use simulation in your controller)
- The current score, time left
- Simulating the engine forward a single tick, or `enginestate::getEngineUpdateRate()` (8) steps i.e. corresponding for the number of ticks required to transition the agent fully from one cell to a neighbouring cell.