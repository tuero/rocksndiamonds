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
```
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-net-dev libsdl2-ttf-dev
```


## Installing
These instructions will help you install the program. It should compile fine on most systems, but YMMV. The following systems have been tested on:
- Ubuntu 18.04, gcc/g++ 7.4.0
- Ubuntu 18.04, gcc/g++ 8.3.0
- Ubuntu 18.04, Clang/Clang++ 6.0.0
- macOS Catalina 10.15.2, Apple Clang 11.0.0

To install this project, follow the below steps:
```
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

## How It Works
### Setting up a position:
- `i n` : Initialize the game to use an (n,n) board, where 'n' is in [4,8] inclusive. This command has to be executed at least once before any of b,w,s,g,m,u commands.
- `b` : set black to move
- `w` : set white to move
- `s` : Setup a new position by specifying the contents of the board from left-to-right and top-to-bottom. The setup uses 'e' for empty, 'x' for blocked, 'b' for a black players disc, and 'w' for a white players disc. For example, using the 5x5 board above, the setup commands would look like this one:
```
s 
eeeee
eeeee
exwbe
exeee
eeeex
```

### Playing Moves
- `ms1s2` : Move from square s1 to s2, which uses [Algebraic Notation](https://en.wikipedia.org/wiki/Algebraic_notation_(chess))
- `u` : Undo the last move played. Note, that this command can be issued repeatedly to undo any number of moves.


### Search Control
- `1` : Enable subsequent searches using the AlphaBeta search, described [above](#ataxx).
- `2` : Enable subsequent searches using the NegaScout search, described [above](#ataxx).
- `d <depth>` : Set the maximum search depth used in iterative deepening to `<depth>`.
- `ft <time>` : Set the fixed search time per move to `<time>` seconds.
- `rt <time>` : Set the remaning time for the entire game to `<time>` seconds.
- `g` : Begin searching, whose settings are defined by the by the above settings.


### Execution Control
- `q` : Quit the program.


## License

This project is licensed under the GNU GPL version 2 License - see the [COPYING](COPYING) file for details