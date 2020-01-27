from tilesets.sprite import Sprite, BitField, TRANSPOSITION_BIT_FLAGS
from tilesets.tile import Tile, TileSetInfo, TileProperty
import numpy as np

TILE_WIDTH = 3
TILE_HEIGHT = 3

# All empty sprites
EMPTY_TILE = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.is_filler
)

AGENT_TILE = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.agent,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_agent
)

# Single dirt
DIRT_TILE1 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.dirt,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.R_90 & BitField.R_180 & BitField.R_270,
    TileProperty.is_filler
)

# Single dirt
DIRT_TILE2 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.dirt,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.dirt,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.R_90 & BitField.R_180 & BitField.R_270 & BitField.FH & BitField.FHR_90 & BitField.FHR_180 & BitField.FHR_270,
    TileProperty.is_filler
)

# Exit on bottom L/R
EXIT_TILE1 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.door_exit,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_exit
)

# Exit bottom middle
EXIT_TILE2 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.door_exit,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_exit
)

# Exit bottom left/right with partial block
EXIT_TILE3 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.wall_steel,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.door_exit,   Sprite.wall_steel,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_exit
)

# Exit bottom middle with partial block
EXIT_TILE4 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,   Sprite.empty,   Sprite.empty,   Sprite.null],
        [Sprite.null,   Sprite.wall_steel,   Sprite.empty,   Sprite.wall_steel,   Sprite.null],
        [Sprite.null,   Sprite.wall_steel,   Sprite.door_exit,   Sprite.wall_steel,   Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_exit
)


# Rocks (3) held up by dirt
ROCK_TILE1 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.rock,    Sprite.rock,    Sprite.rock,    Sprite.null],
        [Sprite.null,   Sprite.dirt,    Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.empty,    Sprite.empty,    Sprite.empty,    Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_rock
)

# Rocks (2) held up by dirt
ROCK_TILE2 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.rock,    Sprite.empty,    Sprite.rock,    Sprite.null],
        [Sprite.null,   Sprite.dirt,    Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.empty,    Sprite.empty,    Sprite.empty,    Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_rock
)

# Rocks(2 L/R) held up by dirt
ROCK_TILE3 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.empty,    Sprite.rock,    Sprite.rock,    Sprite.null],
        [Sprite.null,   Sprite.dirt,    Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.empty,    Sprite.empty,    Sprite.empty,    Sprite.null],
        [Sprite.null,   Sprite.null,    Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_rock
)


# Single gem (corner)
GEM_TILE1 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,        Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.gem_diamond, Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.null,        Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_gem
)

# Single gem (corner)
GEM_TILE2 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,        Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.gem_diamond, Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_gem
)

# Single gem (middle)
GEM_TILE3 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,        Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.gem_diamond, Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    BitField.FV,
    TileProperty.has_gem
)

# Single gem (middle)
GEM_TILE4 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,        Sprite.null,    Sprite.null,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.gem_diamond,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.dirt,    Sprite.dirt,    Sprite.null],
        [Sprite.null,   Sprite.dirt,        Sprite.null,    Sprite.null,    Sprite.null]
    ]),
    0,
    TileProperty.has_gem
)

# Gem with partial block
GEM_TILE5 = Tile(
    np.array([
        [Sprite.null,   Sprite.null,        Sprite.empty,       Sprite.null,        Sprite.null],
        [Sprite.null,   Sprite.wall_steel,  Sprite.empty,       Sprite.wall_steel,  Sprite.null],
        [Sprite.null,   Sprite.wall_steel,  Sprite.gem_diamond, Sprite.wall_steel,  Sprite.null],
        [Sprite.null,   Sprite.wall_steel,  Sprite.wall_steel,  Sprite.wall_steel,  Sprite.null],
        [Sprite.null,   Sprite.null,        Sprite.null,        Sprite.null,        Sprite.null]
    ]),
    0
)


ALL_TILES = [
    EMPTY_TILE, 
    AGENT_TILE,
    DIRT_TILE1, DIRT_TILE2,
    EXIT_TILE1, EXIT_TILE2, EXIT_TILE3, EXIT_TILE4,
    ROCK_TILE1, ROCK_TILE2, ROCK_TILE3,
    GEM_TILE1, GEM_TILE2, GEM_TILE3, GEM_TILE4, GEM_TILE5

]


# Info for level to use
tileSetInfo = TileSetInfo(TILE_WIDTH, TILE_HEIGHT, EMPTY_TILE, ALL_TILES)