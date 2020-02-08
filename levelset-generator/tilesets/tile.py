import numpy as np
import enum
import random
from tilesets.sprite import BitField, Sprite, TRANSPOSITION_BIT_FLAGS


# Tile information
class TileSetInfo():
    def __init__(self, width, height, defaultTile, blockingTile, padTile, allTiles, metaTiles=[], metaExitTiles=[]):
        """
        Holds information regarding the tileset

        Args:
            width (int) : The number of sprites the tile holds in width
            height (int) : The number of sprites the tile holds in height
            defaultTile (Tile) : Default tile the level will use
            padTile (Tile) : Tile to check for boarder elements
            allTiles (array of Tiles) : All tiles contained in the tileset
        """

        self.width = width
        self.height = height
        self.defaultTile = defaultTile
        self.blockingTile = blockingTile
        self.padTile = padTile
        self.allTiles = allTiles
        self.metaTiles = metaTiles
        self.metaExitTiles = metaExitTiles


class TileSetInfo1():
    def __init__(self, width, height, tiles, metaTiles=[]):
        """
        Holds information regarding the tileset

        Args:
            width (int) : The number of sprites the tile holds in width
            height (int) : The number of sprites the tile holds in height
            defaultTile (Tile) : Default tile the level will use
            padTile (Tile) : Tile to check for boarder elements
            allTiles (array of Tiles) : All tiles contained in the tileset
        """

        self.width = width
        self.height = height
        self.tiles = []
        self.metaTiles = metaTiles

        for tile in tiles:
            self.tiles.append(tile)
            self.tiles += tile.getAllTranspositions()

        self.tiles = list(set(self.tiles))


    def _getTilesProperty(self, prop):
        return [t for t in self.tiles if t.hasProperty(prop)]

    def getAgentTiles(self):
        return self._getTilesProperty(TileProperty.has_agent)

    def getKeyTiles(self):
        return self._getTilesProperty(TileProperty.has_key)

    def getGemTiles(self):
        return self._getTilesProperty(TileProperty.has_gem)

    def getRockTiles(self):
        return list(filter(lambda x : x not in self._getTilesProperty(TileProperty.has_gem), self._getTilesProperty(TileProperty.has_rock)))

    def getGemMetaTiles(self):
        return [mt for mt in self.metaTiles if mt.hasProperty(TileProperty.has_gem)]

    def getRockMetaTiles(self):
        return [mt for mt in self.metaTiles if mt.hasProperty(TileProperty.has_rock) and not mt.hasProperty(TileProperty.has_gem)]


# Tile feature identification
class TileProperty(enum.IntEnum):
    has_gem             = 1 << 0
    has_rock            = 1 << 1
    has_exit            = 1 << 2
    has_key             = 1 << 3
    has_gate            = 1 << 4
    has_agent           = 1 << 5
    is_filler           = 1 << 6
    is_slippery         = 1 << 7



class Tile():

    def __init__(self, data, transpositionFlags=0, propertyFlags=0, gemCount=0, rockCount=0):
        """
        Tiles represent a grid of sprites. Levels are constructed of multiple tiles.

        Args:
            data (numpy.array of Sprites) : Numpy array of sprite objects
            transpositionFlags (int) : Bitfield of transpositions the tile is allowed to take
            propertyFlags (int) : Bitfield of properties the tile holds
        """
        self.data = data                                    # Array of sprite cells
        self.transpositionFlags = transpositionFlags
        self.propertyFlags = propertyFlags
        self.gemCount = gemCount
        self.rockCount = rockCount


    def __hash__(self):
        return hash(str(self.data))


    def __eq__(self, other):
        return self.__hash__() == other.__hash__()


    def copy(self, data):
        return Tile(data, self.transpositionFlags, self.propertyFlags)


    def getData(self, row, col):
        return self.data[row, col]

    def hasProperty(self, propertyFlag):
        return self.propertyFlags & propertyFlag


    def addBit(self):
        insert_row = self.data.shape[0] - 2
        insert_col_range = range(1, self.data.shape[1] - 1)
        empty_indices = [(insert_row, i) for i in insert_col_range if self.data[(insert_row, i)] == Sprite.empty]
        assert len(empty_indices) > 0
        self.data[random.choice(empty_indices)] = Sprite.wall_steel

    def getNumberRockBits(self):
        bottom_row = self.data.shape[0] - 2
        return (self.data[bottom_row].tolist()[1:-1]).count(Sprite.wall_steel)

    def canAddRockBit(self):
        return self.getNumberRockBits() < self.data.shape[1] - 2



    def transpose(self, flag):
        """
        Make a copy of the Tile with the given transposition applied.

        Args:
            flag (int) : Bitfield with bit set to corresponding transpposition to apply

        Returns:
            Tile : A copy of the tile with the applied transposition
        """
        # Rotations
        if flag == BitField.R_90 or flag == BitField.R_180 or flag == BitField.R_270:
            rotation_number = {BitField.R_90 : 1, BitField.R_180 : 2, BitField.R_270 : 3}[flag]
            return self.copy(np.rot90(self.data, rotation_number))

        # Flip along vertical axis (left to right) and subsequent rotations
        if flag == BitField.FV or flag == BitField.FVR_90 or flag == BitField.FVR_180 or flag == BitField.FVR_270:
            rotation_number = {BitField.FV: 0, BitField.FVR_90 : 1, BitField.FVR_180 : 2, BitField.FVR_270 : 3}[flag]
            return self.copy(np.rot90(np.fliplr(self.data), rotation_number))

        # Flip along horizontal axis (up to down) and subsequent rotations
        if flag == BitField.FH or flag == BitField.FHR_90 or flag == BitField.FHR_180 or flag == BitField.FHR_270:
            rotation_number = {BitField.FH : 0, BitField.FHR_90 : 1, BitField.FHR_180 : 2, BitField.FHR_270 : 3}[flag]
            return self.copy(np.rot90(np.flipud(self.data), rotation_number))


    def getAllTranspositions(self):
        """
        Get all legal transpositions for the Tile.

        Returns:
            array of Tiles : All tile objects which correspond to a valid transposition
        """
        return [self.transpose(flag) for flag in TRANSPOSITION_BIT_FLAGS if flag & self.transpositionFlags]


class MetaTile():

    def __init__(self, width, height, tiles, maxBitComplexity=0):
        self.width = width
        self.height = height
        self.tiles = tiles
        self.maxBitComplexity = maxBitComplexity

    def getGemCount(self):
        return sum(tile.gemCount for tile in self.tiles.flatten().tolist())

    def getRockCount(self):
        return sum(tile.rockCount for tile in self.tiles.flatten().tolist())

    def hasProperty(self, tileProperty):
        return True in [tile.tileProperty & tileProperty for tile in self.tiles.flatten().tolist()]
