
# Usage
The standard Rocks'n'Diamonds client comes with various command line arguments, but the ones described here are the ones added to aid in the use of custom controllers and additional features.

## Playing a level with your controller
To play a level with your controller, you need to specify the following command line arguments:
- `-contoller CONTROLLER_NAME`: This specifies the controller to use. `CONTROLLER_NAME` is the controller string name, as defined in `./src/ai/include/controller_listing.h`.
- `-levelset LEVELSET_NAME`: This specifies the levelset to test your controller on. The `LEVELSET_NAME` is usually the folder name in `./levels/`. If you are not sure, you can start the client normally using `./rocksndiamonds_gui`, and manually click into the `LEVELSET` option to find the name.
- `-loadlevel LEVEL_NUM`: This specifies the level number to play in the requested levelset as set above.
- `-all_levels`: Instead of specifying one level to play, using this option will test the agent on all levels in the defined levelset.

## Examples
To use the controller `MCTS` on level `3` of the levelset `hard_deadlock`:
```shell
./rocksndiamonds_gui -controller MCTS -levelset hard_deadlock -loadlevel 3
```

To use the controller `MCTS` on all levels of the levelset `hard_deadlock`:
```shell
./rocksndiamonds_gui -controller MCTS -levelset hard_deadlock -all_levels
```

## Saving your controllers actions
To save the actions made by your controller to view them later during replays, use the `-save_run` argument. The replay file name is a combination of the current datetime and PID that is used when running the client. Inside the replay file, the levelset and level number are specified. If your controller is set to replay levels if it fails, an additional replay file is created with the suffix `_INDIVIDUAL' which is the actions the agent made at for the replay that solved the level.
```shell
./rocksndiamonds_gui -controller MCTS -levelset hard_deadlock -loadlevel 3 -save_run
```

## Replaying a previous run
To replay a previous run, only the `-replay REPLAY_FILE_NAME` argument should be used. Replay files are saved in `./src/ai/replays` directory. Only the file name should be given, and not the full path. 

**Note**: Replay files that were created using the headless version can be run using the GUI version. This is the recommended way of doing so if you are running many experiments.
```shell
./rocksndiamonds_gui -replay "2020-01-01 01:22:33 12345.txt"
```