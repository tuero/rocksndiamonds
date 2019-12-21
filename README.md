[![Build Status](https://travis-ci.com/tuero/rocksndiamonds.svg?token=zarvik1a4n45zBNhaz4z&branch=master)](https://travis-ci.com/tuero/rocksndiamonds)

# Ataxx
An Ataxx game implementation that allows users to enter moves, or uses tree-search techniques to come up with moves. The two types of tree-search methods implemented are:
- AlphaBeta pruning
- NegaScout

Both search methods are implemented with transposition tables and iterative deepening. The value function for AlphaBeta is the piece differential between the players and serves as a baseline. The advanced search with NegaScout uses a value function which incorporate the piece differential along with favouring pieces which are surrounded by non-opponent pieces (harder to capture) and penalizes non-opponent pieces which have a single square gap between them (opponent can jump into the gap and capture both pieces).

## Libraries
- Catch2 v2.11.0
- FakeIt v2.0.5
- Plog v1.1.5


## Installing
These instructions will help you install the program. This has been created/tested on Ubuntu 18.04, g++ 7.3.0.
```
# Clone this repository
$ git clone https://github.com/tuero/ataxx.git

# Enter the repository
$ cd ataxx

# Compile
$ make

# Run the program
$ ./ataxx
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

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details