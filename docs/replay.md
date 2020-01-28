
# Saving and using replays
While the Rocks'n'Diamonds comes with its own replay feature, the implemented version which the controller uses is more practical for our use cases.

## Structure of replay files
Replay files are saved in the `./src/ai/replays` directory as simple text files. The first 3 lines are the current engine random seed, the level-set name, and the level number. This allows the client to know what level to load when replaying that run. The engine random seed is so that the engine random state can be set to what it was during the run, for reproducibility in stochastic environments.

The subsequent lines are the actions sent by the controller to the agent, in order they appear. If the controller requested a restart, then an action of `restart` is sent. The name of replay files are a combination of the datetime and PID of the client when it is ran. The replay file contains all moves made over all replays of a level. Since a controller may reattempt a single level many times, an additional replay file is created with a suffix `_INDIVIDUAL` which contains only the run on the last play through, which is the one which solves the level. This is ideal for when you only want to view what the agent has done to solve the level.

## Telling the client to save replay file
To enable the replay save feature, use the command line argument `-save_run`.
```shell
./rocksndiamonds_headless -controller MCTS -levelset hard_deadlock -loadlevel 1 -save_run
```

## Loading a replay file
When loading a replay file, the engine assumes that it exists in the `./src/ai/replays` directory. To load a replay file, use the `-replay` command line argument and specify the file name. For example, to load the replay file `./src/ai/replays/2020-01-01 01:22:33 12345.txt`, use
```shell
./rocksndiamonds_gui -replay "2020-01-01 01:22:33 12345.txt"
```