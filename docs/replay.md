
# Saving and using replays
While the Rocks'n'Diamonds comes with its own replay feature, the implemented version which the controller uses is more practical for our use cases.

## Structure of replay files
Replay files are saved in the `./src/ai/replays` directory as simple text files. A folder is created for each program run, with the folder name being a combination of the datetime and PID of the client when it is ran. The first 3 lines are the current engine random seed, the level-set name, and the level number. This allows the client to know what level to load when replaying that run. The engine random seed is so that the engine random state can be set to what it was during the run, for reproducibility in stochastic environments.

The subsequent lines are the actions sent by the controller to the agent, in order they appear. If the controller requested a restart, then an action of `restart` is sent. The name of replay files are the level number that is being ran by the controller. 



The replay file contains all moves made over all replays of a level. This file is called `complete.txt`. Since this can be quite long, the complete replay file needs to be enabled using the `-save_run` command as explained below. For each level completed by the controller, an additional replay file is created titled `i.txt`, where `i` is the level number. These replay files **only** contain the run on the last play through, which is the one which solves the level. This is ideal for when you only want to view what the agent has done to solve the level.

## Telling the client to save replay file
To enable the replay save feature for **all** runs, use the command line argument `-save_run`.
```shell
./rocksndiamonds_headless -controller MCTS -levelset hard_deadlock -loadlevel 1 -save_run
```

## Loading a replay file
When loading a replay file, the engine assumes that it exists in the `./src/ai/replays` directory. To load a replay file, use the `-replay` command line argument and specify the file name. For example, to load the replay file of the complete runs `./src/ai/replays/2020-01-01_01:22:33_12345/complete.txt`, use
```shell
./rocksndiamonds -replay "2020-01-01_01:22:33_12345/complete.txt"
```

Similarly, to load the replay file of the single run for level 1 `./src/ai/replays/2020-01-01_01:22:33_12345/1.txt`, use
```shell
./rocksndiamonds -replay "2020-01-01_01:22:33_12345/1.txt"
```