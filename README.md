[![Build Status](https://travis-ci.com/tuero/rocksndiamonds.svg?token=zarvik1a4n45zBNhaz4z&branch=master)](https://travis-ci.com/tuero/rocksndiamonds)

# Rocks'n'Diamonds
[Rocks'n'Diamonds](https://www.artsoft.org/) is an open source (C) arcade style game based off Boulder Dash (Commodore 64), Emerald Mine (Amiga), Supaplex (Amiga/PC) and Sokoban (PC). This project uses the open source engine provided by the folks at [Artsoft Entertainment](https://www.artsoft.org/), and extends it with features such as
- Easily add your own AI controller to play the levels
- Automatic level generator
- Ability to replay the levels using the actions the AI agent made
- Comprehensive logging

## Libraries
The following header only libraries are used in this project. They are included as git submodules, so it is recommended you install them by using the `git --recurse-submodules` argument.
- Catch2 v2.11.0
- FakeIt v2.0.5
- Plog v1.1.5

Additionally, SDL2 is needed, and highly recommended, to run any the GUI version. In addition to the standard package, image, mixer, net, and ttf will also need to be installed.
```shell
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev
```


## Installing
These instructions will help you install the program. It should compile fine on most systems, but YMMV. The following systems have been tested on:
- Ubuntu 18.04, gcc/g++ 7.4.0
- Ubuntu 18.04, gcc/g++ 8.3.0
- Ubuntu 18.04, Clang/Clang++ 6.0.0
- macOS Catalina 10.15.2, Apple Clang 11.0.0

To install this project, follow the below steps:
```shell
# Clone this repository
$ git clone --recurse-submodules https://github.com/tuero/rocksndiamonds.git

# Enter the repository
$ cd rocksndiamonds

# Compile
$ mkdir build && cd build
$ cmake ..
$ make -j8
$ cd ..

# Run the GUI program with user input
$ ./rocksndiamonds_gui
```
For a detailed listing of the usage, see the [usage](docs/usage.md) section of the documents.

## Documentation
- [GUI vs Headless](docs/binaries.md)
- [Creating a basic controller](docs/controller.md)
- [Detailed controller features](docs/controller_detailed.md)
- [Accessing engine state and information](docs/engine.md)
- [Usage](docs/usage.md)
- [Logging](docs/logging.md)
- [Replay files](docs/replay.md)


## License
This project is licensed under the GNU GPL version 2 License - see the [COPYING](COPYING) file for details.