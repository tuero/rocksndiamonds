
import numpy as np
import random
import copy
from PIL import Image

from tilesets.sprite import Sprite, SpriteInfo
from tilesets.tile import Tile, TileProperty, MetaTile
from tilesets.tileset_3by3 import tileSetInfo
from tilesets.leveltemplates import LevelTemplate


IMG_EXT = '.png'


class LevelDifficulty():

    def __init__(self, numGemTiles, numRockTiles, numGemsRequired, bitComplexity=0):
        """
        Requirements which a randomized level must meet to ensure the wanted difficulty

        Args:
            numGems (int) : The number of gem tiles required
            numRocks (int) : The number of rock tiles required
        """
        self.numGemTiles = numGemTiles
        self.numRockTiles = numRockTiles
        self.numGemsRequired = numGemsRequired
        self.bitComplexity = bitComplexity

        self.isKeyDoorPresent = False



class Level():

    def __init__(self, numTilesWidth, numTilesHeight, tileSetInfo, levelDifficulty, levelTemplate):
        """
        Level definition

        Args:
            numTilesWidth (int) : Width of the level by number of tiles
            numTilesHeight (int) : Height of the level by number of tiles
            tileSetInfo (TileSetInfo) : Information regarding tiles available to the level to use
            levelDifficulty (LevelDifficulty) : Difficulty requirements for level randomization
        """
        self.numTilesWidth = levelTemplate.numTilesWidth
        self.numTilesHeight = levelTemplate.numTilesHeight
        self.tileSetInfo = tileSetInfo
        self.levelDifficulty = levelDifficulty
        self.levelTemplate = levelTemplate
        # self.tiles = np.array([[tileSetInfo.defaultTile]*numTilesWidth]*numTilesHeight, dtype=Tile)
        self.tiles = self.levelTemplate.getTiles()

        # -- reference ranges
        referenceRange_l = [(self.tileSetInfo.width + 1, y) for y in range(self.tileSetInfo.height+2)] + [(self.tileSetInfo.width, y) for y in range(self.tileSetInfo.height+2)]
        referenceRange_r = [(0, y) for y in range(self.tileSetInfo.height+2)] + [(1, y) for y in range(self.tileSetInfo.height+2)]
        referenceRange_u = [(x, self.tileSetInfo.height + 1) for x in range(self.tileSetInfo.width+2)] + [(x, self.tileSetInfo.height) for x in range(self.tileSetInfo.width+2)]
        referenceRange_d = [(x, 0) for x in range(self.tileSetInfo.width+2)] + [(x, 1) for x in range(self.tileSetInfo.width+2)]

        tileRange_l = [(1, y) for y in range(self.tileSetInfo.height+2)] + [(0, y) for y in range(self.tileSetInfo.height+2)]
        tileRange_r = [(self.tileSetInfo.width, y) for y in range(self.tileSetInfo.height+2)] + [(self.tileSetInfo.width + 1, y) for y in range(self.tileSetInfo.height+2)]
        tileRange_u = [(x, 1) for x in range(self.tileSetInfo.width+2)] + [(x, 0) for x in range(self.tileSetInfo.width+2)]
        tileRange_d = [(x, self.tileSetInfo.height) for x in range(self.tileSetInfo.width+2)] + [(x, self.tileSetInfo.height+1) for x in range(self.tileSetInfo.width+2)]

        self.refence_ranges = {'left'    : [referenceRange_l, tileRange_l],
                               'right'   : [referenceRange_r, tileRange_r],
                               'up'      : [referenceRange_u, tileRange_u],
                               'down'    : [referenceRange_d, tileRange_d]
                        }

        # Get all tile transpositions and remove duplicates
        # self.availableTiles = [tile for tile in tileSetInfo.allTiles] + [tileSetInfo.defaultTile]
        # for tile in tileSetInfo.allTiles:
        #     self.availableTiles += tile.getAllTranspositions()

        # self.availableTiles = list(set(self.availableTiles))

    def canOverwriteTile(self, tile):
        return tile == self.tileSetInfo.defaultTile or tile == self.tileSetInfo.blockingTile


    def resetLevel(self):
        # Set the level back to the default tile
        # self.tiles = np.array([[self.tileSetInfo.defaultTile]*self.numTilesWidth]*self.numTilesHeight, dtype=Tile)
        self.tiles = self.levelTemplate.getTiles()


    def getWidth(self):
        # Width of level in sprites
        return self.numTilesWidth * (self.tileSetInfo.width)


    def getHeight(self):
        # height of level in sprites
        return self.numTilesHeight * (self.tileSetInfo.height)


    def _validateTileToReference(self, tile1, tile2, tile1Range, tile2Range):
        if self.canOverwriteTile(tile1): return True
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
        tileReference = self.tiles[row, col-1] if col > 0 else self.tileSetInfo.padTile
        validLeft = self._validateTileToReference(tileReference, tile, *self.refence_ranges['left'])

        # Check if tile is valid with reference tile to the right
        tileReference = self.tiles[row, col+1] if (col < self.numTilesWidth - 1) else self.tileSetInfo.padTile
        validRight = self._validateTileToReference(tileReference, tile, *self.refence_ranges['right'])

        # Check if tile is valid with reference tile above
        tileReference = self.tiles[row-1, col] if row > 0 else self.tileSetInfo.padTile
        validAbove = self._validateTileToReference(tileReference, tile, *self.refence_ranges['up'])

        # Check if tile is valid with reference tile below
        tileReference = self.tiles[row+1, col] if (row < self.numTilesHeight - 1) else self.tileSetInfo.padTile
        validBelow = self._validateTileToReference(tileReference, tile, *self.refence_ranges['down'])

        return validLeft and validRight and validAbove and validBelow


    def placeExit(self):
        """
        Place an exit tile on the map. 
        The exit tile is always assumed to be the first tile placed, and only one tile can contain the exit.

        Note: We assume that the exit is placed on the bottom row.
        """
        empty_indices = [(self.numTilesHeight-1, col) for col in range(1, self.numTilesWidth-1) if self.canOverwriteTile(self.tiles[self.numTilesHeight-1, col])]
        exit_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_exit]
        exit_index = random.choice(empty_indices)
        self.tiles[exit_index] = random.choice(exit_tiles)


    def placeAgent(self):
        """
        Place an agent tile on the map. 
        The agent tile is always assumed to be the second tile placed, and only one tile can contain the agent.
        """
        empty_indices = [(row, col) for row in range(self.numTilesHeight) for col in range(self.numTilesWidth) if self.canOverwriteTile(self.tiles[row, col])]
        agent_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.has_agent]
        agent_index = random.choice(empty_indices)
        self.tiles[agent_index] = random.choice(agent_tiles)


    def _validateMetaTile(self, empty_tile, meta_tile):
        start_row, start_col = empty_tile

        # Walk along left
        for r in range(start_row, start_row + meta_tile.height):
            tile = self.tiles[r, start_col]
            tileReference = self.tiles[r, start_col-1] if start_col > 0 else self.tileSetInfo.padTile
            if not self._validateTileToReference(tileReference, tile, *self.refence_ranges['left']): 
                return False

        # Walk along right
        for r in range(start_row, start_row + meta_tile.height):
            tile = self.tiles[r, start_col + meta_tile.width - 1]
            tileReference = self.tiles[r, start_col + meta_tile.width] if (start_col + meta_tile.width < self.numTilesWidth - 1) else self.tileSetInfo.padTile
            if not self._validateTileToReference(tileReference, tile, *self.refence_ranges['right']): 
                return False

        # Walk along top
        for c in range(start_col, start_col + meta_tile.width):
            tile = self.tiles[start_row, c]
            tileReference = self.tiles[start_row-1, c] if start_row > 0 else self.tileSetInfo.padTile
            if not self._validateTileToReference(tileReference, tile, *self.refence_ranges['up']): 
                return False

        # Walk along bottom
        for c in range(start_col, start_col + meta_tile.width):
            tile = self.tiles[start_row + meta_tile.height - 1, c]
            print('{} {} {}'.format(start_row, meta_tile.height, self.numTilesHeight))
            tileReference = self.tiles[start_row + meta_tile.height, c] if (start_row + meta_tile.height < self.numTilesHeight - 1) else self.tileSetInfo.padTile
            if not self._validateTileToReference(tileReference, tile, *self.refence_ranges['down']): 
                return False

        return True


    def findPlacementMetaTile(self, empty_tiles, meta_tile):
        for empty_tile in empty_tiles:
            if self._validateMetaTile(empty_tile, meta_tile):
                # Place cells represented by the tile
                start_row, start_col = empty_tile
                tiles_indices = [(row, col) for row in range(start_row, start_row + meta_tile.height) for col in range(start_col, start_col + meta_tile.width)]
                tiles = meta_tile.tiles.reshape(1, -1)[0]
                for index, tile in zip(tiles_indices, tiles):
                    row, col = index
                    self.tiles[row][col] = tile
                return True
        return False

    def _getEmptyMetaTiles(self, width, height):
        empty_indices = []
        for row_t in range(self.numTilesHeight - height+1):
            for col_t in range(self.numTilesWidth - width+1):
                # (row_t, col) is start of meta tile coverage top left, need to verify all interior tiles are empty
                interior_cells = [self.canOverwriteTile(self.tiles[r, c]) for r in range(row_t, row_t+height) for c in range(col_t, col_t+width)]
                if False not in interior_cells:
                    empty_indices.append([row_t, col_t])
        return empty_indices

    def placeMetaTile(self):
        for _ in range(1000):
            flag = True
            for meta_tile in self.tileSetInfo.metaTiles:
                # Find empty locations of same size of meta tile
                empty_indices = self._getEmptyMetaTiles(meta_tile.width, meta_tile.height)
                random.shuffle(empty_indices)

                # Successfully found and placed meta tile
                if not self.findPlacementMetaTile(empty_indices, meta_tile): 
                    flag = False
                    break

            if flag: return True

        return False


    def placeExitMetaTile(self):
        random.shuffle(self.tileSetInfo.metaExitTiles)
        exit_meta_tile = self.tileSetInfo.metaExitTiles[0]
        empty_indices = self._getEmptyMetaTiles(exit_meta_tile.width, exit_meta_tile.height)
        # exit starts on lowest row, cant be on boarder column
        empty_indices = [e for e in empty_indices if e[0] == max(empty_indices, key=lambda x : x[0])[0] and e[1] > 0 and e[1] < self.numTilesWidth-1]

        # randomly choose viable option and place exit meta tile
        random.shuffle(empty_indices)
        self.findPlacementMetaTile(empty_indices, exit_meta_tile)




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
                    self.tiles[empty_index] = sprite_tiles[j]
                    empty_indices.pop(i)
                    return True
        return False


    def _placeTileCategoryType(self, numOfType, tiles_of_type):
        # Given a type of tile, randomly select tile of that type and find valid placement
        empty_indices = [(row, col) for row in range(self.numTilesHeight) for col in range(self.numTilesWidth) if self.canOverwriteTile(self.tiles[row, col])]
        
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

    def placeRoundWalls(self):
        slip_tiles = [tile for tile in self.availableTiles if tile.propertyFlags & TileProperty.is_slippery]
        return self._placeTileCategoryType(3, slip_tiles)


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


    def _removeIndex(self, index, *lists):
        for l in lists:
            if index in l: l.remove(index)
        

    def randomizeLevel(self):
        """
        Randomize the given level.

        Note: Throws exception if ordering of tiles placed created rules from padding which doesn't
                allow the level to be created.
        """
        self.resetLevel()
        
        empty_indices = self.levelTemplate.getEmptyIndices()
        agent_indices = self.levelTemplate.getAgentIndices()
        key_indices = self.levelTemplate.getKeyIndices()
        gem_indices = self.levelTemplate.getGemIndices()
        rock_indices = self.levelTemplate.getRockIndices()

        # Place agent
        agent_tiles = self.tileSetInfo.getAgentTiles()
        agent_index = random.choice(agent_indices)
        self._removeIndex(agent_index, empty_indices, gem_indices, rock_indices, key_indices)
        self.tiles[agent_index] = random.choice(agent_tiles)

        # Place key
        if len(key_indices) > 0:
            key_tiles = self.tileSetInfo.getKeyTiles()
            key_index = random.choice(key_indices)
            self._removeIndex(key_index, empty_indices, gem_indices, rock_indices, key_indices)
            self.tiles[key_index] = random.choice(key_tiles)

        # Place rocks
        rock_tiles = self.tileSetInfo.getRockTiles()
        num_rocks = self.levelDifficulty.numRockTiles
        rock_placements = []
        assert num_rocks <= len(rock_indices), "Not enough empty tiles for number of rocks."
        for _ in range(num_rocks):
            rock_index = random.choice(rock_indices)
            self._removeIndex(rock_index, empty_indices, gem_indices, rock_indices, key_indices)
            self.tiles[rock_index] = copy.deepcopy(random.choice(rock_tiles))
            rock_placements.append(rock_index)

        # Place gems
        gem_tiles = self.tileSetInfo.getGemTiles()
        num_gems = self.levelDifficulty.numGemTiles
        assert num_gems <= len(gem_indices), "Not enough empty tiles for number of gems."
        for _ in range(num_gems):
            gem_index = random.choice(gem_indices)
            self._removeIndex(gem_index, empty_indices, gem_indices, rock_indices, key_indices)
            self.tiles[gem_index] = copy.deepcopy(random.choice(gem_tiles))

        #Place rock bit complexity
        for _ in range(self.levelDifficulty.bitComplexity):
            # Get tiles with rock and has bit room
            index = random.choice([i for i in rock_placements if self.tiles[i].canAddRockBit()])
            self.tiles[index].addBit()

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

