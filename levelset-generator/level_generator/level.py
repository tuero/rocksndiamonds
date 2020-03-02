
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

    def __init__(self, tileSetInfo, levelDifficulty, levelTemplate):
        """
        Level definition

        Args:
            numTilesWidth (int) : Width of the level by number of tiles
            numTilesHeight (int) : Height of the level by number of tiles
            tileSetInfo (TileSetInfo) : Information regarding tiles available to the level to use
            levelDifficulty (LevelDifficulty) : Difficulty requirements for level randomization
        """
        self.numTilesWidth = levelTemplate.getNumTilesWidth()
        self.numTilesHeight = levelTemplate.getNumTilesHeight()
        self.tileSetInfo = tileSetInfo
        self.levelDifficulty = levelDifficulty
        self.levelTemplate = levelTemplate
        self.tiles = self.levelTemplate.getTiles()

        self.set_bits_indices = []

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


    def resetLevel(self):
        # Set the level back to the default tile
        self.tiles = self.levelTemplate.getTiles()


    def getWidth(self):
        # Width of level in sprites
        return self.numTilesWidth * (self.tileSetInfo.width)


    def getHeight(self):
        # height of level in sprites
        return self.numTilesHeight * (self.tileSetInfo.height)

    
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


    def _removeIndex(self, index, lists):
        for l in lists:
            if index in l: l.remove(index)

    def remove_bits(self):
        for index in self.set_bits_indices:
            self.tiles[index].removeAllBits()

    def add_bits(self):
        #Place rock bit complexity
        bit_indices = self.levelTemplate.getBitIndices()
        num_bits = self.levelDifficulty.bitComplexity
        num_bits = random.randint(1, self.levelDifficulty.bitComplexity)
        self.set_bits_indices.clear()

        for _ in range(num_bits):
            index = random.choice([i for i in self.rock_placements if self.tiles[i].canAddRockBit()])
            self.set_bits_indices.append(index)
            self.tiles[index].addBit()

    def randomizeLevel(self):
        """
        Randomize the given level.

        Note: Throws exception if ordering of tiles placed created rules from padding which doesn't
                allow the level to be created.
        """
        self.resetLevel()
        
        agent_indices = self.levelTemplate.getAgentIndices()
        gem_indices = self.levelTemplate.getGemIndices()
        rock_indices = self.levelTemplate.getRockIndices()
        key_red_indices = self.levelTemplate.getKeyRedIndices()
        key_green_indices = self.levelTemplate.getKeyGreenIndices()
        key_blue_indices = self.levelTemplate.getKeyBlueIndices()
        key_yellow_indices = self.levelTemplate.getKeyYellowIndices()
        all_key_info = [
            (key_red_indices, self.tileSetInfo.getKeyRedTiles),
            (key_green_indices, self.tileSetInfo.getKeyGreenTiles),
            (key_blue_indices, self.tileSetInfo.getKeyBlueTiles),
            (key_yellow_indices, self.tileSetInfo.getKeyYellowTiles)
        ]
        all_indices = [
            agent_indices, gem_indices, rock_indices, key_red_indices, 
            key_green_indices, key_blue_indices, key_yellow_indices
        ]

        # Place agent
        agent_tiles = self.tileSetInfo.getAgentTiles()
        agent_index = random.choice(agent_indices)
        self._removeIndex(agent_index, all_indices)
        self.tiles[agent_index] = random.choice(agent_tiles)

        # Place key
        for key_indices, key_func in all_key_info:
            if len(key_indices) > 0:
                key_tiles = key_func()
                key_index = random.choice(key_indices)
                self._removeIndex(key_index, all_indices)
                self.tiles[key_index] = random.choice(key_tiles)

        # Place rocks
        rock_tiles = self.tileSetInfo.getRockTiles()
        num_rocks = self.levelDifficulty.numRockTiles
        self.rock_placements = []
        assert num_rocks <= len(rock_indices), "Not enough empty tiles for number of rocks."
        for _ in range(num_rocks):
            rock_index = random.choice(rock_indices)
            self._removeIndex(rock_index, all_indices)
            self.tiles[rock_index] = copy.deepcopy(random.choice(rock_tiles))
            self.rock_placements.append(rock_index)

        # Place gems
        gem_tiles = self.tileSetInfo.getGemTiles()
        num_gems = self.levelDifficulty.numGemTiles
        assert num_gems <= len(gem_indices), "Not enough empty tiles for number of gems."
        for _ in range(num_gems):
            gem_index = random.choice(gem_indices)
            self._removeIndex(gem_index, all_indices)
            self.tiles[gem_index] = copy.deepcopy(random.choice(gem_tiles))

        # self.add_bits()
        # self.remove_bits()
        

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

