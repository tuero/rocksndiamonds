
import numpy as np
import random
from PIL import Image

from tilesets.sprite import Sprite, SpriteInfo
from tilesets.tile import Tile, TileProperty
from tilesets.tileset_3by3 import tileSetInfo


IMG_EXT = '.png'


class LevelDifficulty():

    def __init__(self, numGemTiles, numRockTiles, numGemsRequired):
        """
        Requirements which a randomized level must meet to ensure the wanted difficulty

        Args:
            numGems (int) : The number of gem tiles required
            numRocks (int) : The number of rock tiles required
        """
        self.numGemTiles = numGemTiles
        self.numRockTiles = numRockTiles
        self.numGemsRequired = numGemsRequired


class Level():

    def __init__(self, numTilesWidth, numTilesHeight, tileSetInfo, levelDifficulty):
        """
        Level definition

        Args:
            numTilesWidth (int) : Width of the level by number of tiles
            numTilesHeight (int) : Height of the level by number of tiles
            tileSetInfo (TileSetInfo) : Information regarding tiles available to the level to use
            levelDifficulty (LevelDifficulty) : Difficulty requirements for level randomization
        """
        self.numTilesWidth = numTilesWidth
        self.numTilesHeight = numTilesHeight
        self.tileSetInfo = tileSetInfo
        self.levelDifficulty = levelDifficulty
        self.tiles = np.array([[tileSetInfo.defaultTile]*numTilesWidth]*numTilesHeight, dtype=Tile)

        # Get all tile transpositions and remove duplicates
        self.availableTiles = [tile for tile in tileSetInfo.allTiles] + [tileSetInfo.defaultTile]
        for tile in tileSetInfo.allTiles:
            self.availableTiles += tile.getAllTranspositions()

        self.availableTiles = list(set(self.availableTiles))


    def resetLevel(self):
        # Set the level back to the default tile
        self.tiles = np.array([[self.tileSetInfo.defaultTile]*self.numTilesWidth]*self.numTilesHeight, dtype=Tile)


    def getWidth(self):
        # Width of level in sprites
        return self.numTilesWidth * (self.tileSetInfo.width)


    def getHeight(self):
        # height of level in sprites
        return self.numTilesHeight * (self.tileSetInfo.height)


    def _validateTileToReference(self, tile1, tile2, tile1Range, tile2Range):
        # For given tile pair, check tile padding is null or both are matching
        for tile1Coord, tile2Coord in zip(tile1Range, tile2Range):
            col1, row1 = tile1Coord
            col2, row2 = tile2Coord
            if not (tile1.getData(row1, col1) == tile2.getData(row2, col2) or (tile1.getData(row1, col1) == Sprite.null or tile2.getData(row2, col2) == Sprite.null)):
                return False
        return True


    def _validateTileToAllNeighbours(self, tile, row, col):
        # Check each neighbour for the given tile, and check padding rules
        validLeft, validRight, validAbove, validBelow = True, True, True, True

        # Check if tile is valid with reference tile to the left
        if col > 0:
            tileReference = self.tiles[row, col-1]
            referenceRange = [(self.tileSetInfo.width + 1, y) for y in range(self.tileSetInfo.height+2)] + [(self.tileSetInfo.width, y) for y in range(self.tileSetInfo.height+2)]
            tileRange = [(1, y) for y in range(self.tileSetInfo.height+2)] + [(0, y) for y in range(self.tileSetInfo.height+2)]
            validLeft = self._validateTileToReference(tileReference, tile, referenceRange, tileRange)

        # Check if tile is valid with reference tile to the right
        if col < self.numTilesWidth - 1:
            tileReference = self.tiles[row, col+1]
            referenceRange = [(0, y) for y in range(self.tileSetInfo.height+2)] + [(1, y) for y in range(self.tileSetInfo.height+2)]
            tileRange = [(self.tileSetInfo.width, y) for y in range(self.tileSetInfo.height+2)] + [(self.tileSetInfo.width + 1, y) for y in range(self.tileSetInfo.height+2)]
            validRight = self._validateTileToReference(tileReference, tile, referenceRange, tileRange)

        # Check if tile is valid with reference tile above
        if row > 0:
            tileReference = self.tiles[row-1, col]
            referenceRange = [(x, self.tileSetInfo.height + 1) for x in range(self.tileSetInfo.width+2)] + [(x, self.tileSetInfo.height) for x in range(self.tileSetInfo.width+2)]
            tileRange = [(x, 1) for x in range(self.tileSetInfo.width+2)] + [(x, 0) for x in range(self.tileSetInfo.width+2)]
            validAbove = self._validateTileToReference(tileReference, tile, referenceRange, tileRange)

        # Check if tile is valid with reference tile below
        if row < self.numTilesHeight - 1:
            tileReference = self.tiles[row+1, col]
            referenceRange = [(x, 0) for x in range(self.tileSetInfo.width+2)] + [(x, 1) for x in range(self.tileSetInfo.width+2)]
            tileRange = [(x, self.tileSetInfo.height) for x in range(self.tileSetInfo.width+2)] + [(x, self.tileSetInfo.height+1) for x in range(self.tileSetInfo.width+2)]
            validBelow = self._validateTileToReference(tileReference, tile, referenceRange, tileRange)

        return validLeft and validRight and validAbove and validBelow


    def placeExit(self):
        """
        Place an exit tile on the map. 
        The exit tile is always assumed to be the first tile placed, and only one tile can contain the exit.

        Note: We assume that the exit is placed on the bottom row.
        """
        empty_indices = [(self.numTilesHeight-1, col) for col in range(self.numTilesWidth) if self.tiles[self.numTilesHeight-1, col] == self.tileSetInfo.defaultTile]
        exit_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_exit]
        empty_index = random.choice(empty_indices)
        self.tiles[empty_index[0], empty_index[1]] = random.choice(exit_tiles)


    def placeAgent(self):
        """
        Place an agent tile on the map. 
        The agent tile is always assumed to be the second tile placed, and only one tile can contain the agent.
        """
        empty_indices = [(row, col) for row in range(self.numTilesHeight) for col in range(self.numTilesWidth) if self.tiles[row, col] == self.tileSetInfo.defaultTile]
        agent_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_agent]
        empty_index = random.choice(empty_indices)
        self.tiles[empty_index[0], empty_index[1]] = random.choice(agent_tiles)


    def findValidPlacement(self, empty_indices, sprite_tiles):
        """
        Find a valid placement for the given tile/

        Note:   Methods as defined in (PROCEDURAL GENERATION OF SOKOBAN LEVELS, Taylor & Parberry 2011)
                are used to determine valid placement of tiles. Each nxn tile has a padding boarder, which
                determines rules for boarding tiles to match.

        Args:
            empty_indices (array of index pairs) : List of indicies to try and insert a tile in
            sprite_tiles (array of Tile) : List of Tile objects

        Returns: True if the tile is inserted, False otherwise (i.e. no valid placement found).
        """
        for i in range(len(empty_indices)):
            empty_index = empty_indices[i]
            for j in range(len(sprite_tiles)):
                if self._validateTileToAllNeighbours(sprite_tiles[j], empty_index[0], empty_index[1]):
                    self.tiles[empty_index[0], empty_index[1]] = sprite_tiles[j]
                    empty_indices.pop(i)
                    return True
        return False


    def _placeTileCategoryType(self, numOfType, tiles_of_type):
        # Given a type of tile, randomly select tile of that type and find valid placement
        empty_indices = [(row, col) for row in range(self.numTilesHeight) for col in range(self.numTilesWidth) if self.tiles[row, col] == self.tileSetInfo.defaultTile]

        # For each gem, find random gem tile and random location
        for _ in range(numOfType):
            random.shuffle(tiles_of_type)
            random.shuffle(empty_indices)

            # Try all possibilities until we have valid placement
            if not self.findValidPlacement(empty_indices, tiles_of_type):
                return False
        return True


    def placeGems(self):
        """
        Place a gem tile on the map. 
        
        Note: Depending on the padding rules, it may not be possible to place a gem tile.

        Returns:
            True if a gem tile can successfully inserted, false otherwise.
        """
        gem_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_gem]
        return self._placeTileCategoryType(self.levelDifficulty.numGemTiles, gem_tiles)


    def placeRocks(self):
        """
        Place a rock tile on the map. 
        
        Note: Depending on the padding rules, it may not be possible to place a rock tile.

        Returns:
            True if a rock tile can successfully inserted, false otherwise.
        """
        rock_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_rock]
        return self._placeTileCategoryType(self.levelDifficulty.numRockTiles, rock_tiles)

    
    def getUnderlyingData(self):
        """
        Get the underlying sprite data for the whole level.

        Return:
            Data (array of Sprites) : A flattened array of Sprites representing the level.
        """
        total_width = self.numTilesWidth * (self.tileSetInfo.width)
        total_height = self.numTilesHeight * (self.tileSetInfo.height)

        data = []
        for y in range(total_height):
            for x in range(total_width):
                tile_y = y // self.tileSetInfo.height
                tile_x = x // self.tileSetInfo.width
                offset_y = (y % self.tileSetInfo.height) + 1
                offset_x = (x % self.tileSetInfo.width) + 1
                data.append(self.tiles[tile_y, tile_x].getData(offset_y, offset_x))

        return data


    def randomizeLevel(self):
        """
        Randomize the given level.

        Note: Throws exception if ordering of tiles placed created rules from padding which doesn't
                allow the level to be created.
        """
        timeout_counter = 0

        while (True):
            timeout_counter += 1

            if timeout_counter > 1: 
                raise Exception('Can\'t find a legal level')

            # Try and 
            self.resetLevel()
            self.placeExit()
            self.placeAgent()

            # These placements may fail, continue until valid
            if not self.placeGems(): 
                continue
            if not self.placeRocks(): 
                continue

            # For each empty tile, try and insert 
            empty_indices = [(row, col) for row in range(self.numTilesHeight) for col in range(self.numTilesWidth) if self.tiles[row, col] == self.tileSetInfo.defaultTile]
            for empty_index in empty_indices:
                row, col = empty_index
                valid_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.is_filler and self._validateTileToAllNeighbours(tile, row, col)]
                
                # No valid tile
                if len(valid_tiles) == 0: continue

                # Place tile 
                self.tiles[row, col] = random.choice(valid_tiles)

            return
        

    def _setTileImage(self, image, tile_row, tile_col):
        # Set the partial images for a given sprite
        for row in range(self.tileSetInfo.height):
            for col in range(self.tileSetInfo.width):
                x_offset = tile_col * ((self.tileSetInfo.width) * SpriteInfo['width']) + (col * SpriteInfo['width'])
                y_offset = tile_row * ((self.tileSetInfo.height) * SpriteInfo['height']) + (row * SpriteInfo['height'])
                sprite = self.tiles[tile_row, tile_col].getData(row+1, col+1)
                image.paste(SpriteInfo['img'][sprite], (x_offset, y_offset))


    def saveLevelImage(self, image_name):
        """
        Save the level information as a PNG file.

        Args:
            image_name (str) : String name of output file.
        """
        total_width = self.numTilesWidth * (self.tileSetInfo.width) * SpriteInfo['width']
        total_height = self.numTilesHeight * (self.tileSetInfo.height) * SpriteInfo['height']

        # Create starting image
        image = Image.new('RGB', (total_width, total_height))

        # Save each tile
        for tile_row in range(self.numTilesHeight):
            for tile_col in range(self.numTilesWidth):
                self._setTileImage(image, tile_row, tile_col)

        if image_name[-len(IMG_EXT):] != IMG_EXT: 
            image_name += IMG_EXT
        image.save(image_name)
        return

