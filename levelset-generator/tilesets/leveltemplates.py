import numpy as np
import copy
from tilesets.tile import Tile
import tilesets.tileset_3by3


background_code = ['XX']
agent_codes = ['PP', 'AR', 'AG', 'AB', 'AY', 'CR', 'CG', 'CB', 'CY', 'NK']
gem_codes = ['GG', 'AR', 'AG', 'AB', 'AY', 'CR', 'CG', 'CB', 'CY', 'NK']
rock_codes = ['RR', 'AR', 'AG', 'AB', 'AY', 'NK']
door_red_horz       = 'R1'
door_green_horz     = 'G1'
door_blue_horz      = 'B1'
door_yellow_horz    = 'Y1'
key_red = ['KR', 'AR', 'CR']
key_green = ['KG', 'AG', 'CG']
key_blue = ['KB', 'AB', 'CB']
key_yellow = ['KY', 'AY', 'CY']
bits = ['AR', 'AG', 'AB', 'AY', 'CR', 'CG', 'CB', 'CY', 'NK', 'RR']

codes_list = [
    agent_codes, gem_codes, rock_codes, 
    key_red, key_green, key_blue, key_yellow,
    bits
]

class LevelTemplate():

    def __init__(self, template_file_name):
        self.backgroundTile = tilesets.tileset_3by3.BLOCKING_TILE1
        self.emptyTile = tilesets.tileset_3by3.EMPTY_TILE

        door_map = {
            door_red_horz : tilesets.tileset_3by3.DOOR_RED_HORZ_TILE,
            door_green_horz : tilesets.tileset_3by3.DOOR_GREEN_HORZ_TILE,
            door_blue_horz : tilesets.tileset_3by3.DOOR_BLUE_HORZ_TILE,
            door_yellow_horz : tilesets.tileset_3by3.DOOR_YELLOW_HORZ_TILE
        }

        meta_tile_map = {
            "00" : tilesets.tileset_3by3.META_00,           # Exit with 2 rocks above
            "10" : tilesets.tileset_3by3.META_10,           # Diamond with trap
            "20" : tilesets.tileset_3by3.META_20,           # 3 rocks
            "21" : tilesets.tileset_3by3.META_21,           # Gate blocking (door right)
            "22" : tilesets.tileset_3by3.META_22,           # Gate blocking (door left)
            "23" : tilesets.tileset_3by3.META_23,           # single diamond
            "30" : tilesets.tileset_3by3.META_30,           # bottom right corner for forced middle
            "31" : tilesets.tileset_3by3.META_31,           # bottom left corner for forced middle
            "32" : tilesets.tileset_3by3.META_32,           # upper right corner for forced middle
            "33" : tilesets.tileset_3by3.META_33,           # upper left corner for forced middle
            "34" : tilesets.tileset_3by3.META_34,           # bottom left and right corner for forced middle
            "35" : tilesets.tileset_3by3.META_35,           # upper left and right corner for forced middle
            "40" : tilesets.tileset_3by3.META_40,           # Just the exit
            "41" : tilesets.tileset_3by3.META_41,           # 3 rocks in the top row
            "42" : tilesets.tileset_3by3.META_42,           # Only middle row is open
            "43" : tilesets.tileset_3by3.META_43,           # Only bottom row is open
        }

        # Valid indices for random placement
        self.emptyIndices = []
        self.agent_indices = []
        self.gem_indices = []
        self.rock_indices = []
        self.key_red_indices = []
        self.key_green_indices = []
        self.key_blue_indices = []
        self.key_yellow_indices = []
        self.bit_indices = []
        indices_list = [
            self.agent_indices, self.gem_indices, self.rock_indices, 
            self.key_red_indices, self.key_green_indices,
            self.key_blue_indices, self.key_yellow_indices,
            self.bit_indices
        ]

        with open(template_file_name, 'r') as f:
            # Start with background tile
            self.numTilesHeight, self.numTilesWidth = f.readline().split()
            self.numTilesHeight = int(self.numTilesHeight)
            self.numTilesWidth = int(self.numTilesWidth)
            self.tiles = np.array([[copy.deepcopy(self.backgroundTile)]*self.numTilesWidth]*self.numTilesHeight, dtype=Tile)

            data = np.array([['  ']*self.numTilesWidth]*self.numTilesHeight, dtype=str)
            for row, line in enumerate(f):
                for col, item in enumerate(line.strip().split(' ')):
                    data[(row, col)] = item
                    for codes, indices in zip(codes_list, indices_list):
                        if item in codes: indices.append((row, col))

                    # Doors get auto placed, non-background starts as empty
                    if item not in background_code: self.tiles[(row, col)] = copy.deepcopy(self.emptyTile)
                    if item in door_map: self.tiles[(row, col)] = copy.deepcopy(door_map[item])
            
            for row, col, in [(r, c) for r in range(self.numTilesHeight) for c in range(self.numTilesWidth)]:
                item = data[(row, col)]
                if item in meta_tile_map: 
                    start_row, start_col = row, col
                    metaTile = meta_tile_map[item]
                    tiles_indices = [(r, c) for r in range(start_row, start_row + metaTile.height) for c in range(start_col, start_col + metaTile.width)]
                    tiles = metaTile.tiles.reshape(1, -1)[0]

                    # Set each tile in meta_tile
                    for index, tile in zip(tiles_indices, tiles):
                        self.tiles[tuple(index)] = copy.deepcopy(tile)
                        for codes, indices in zip(codes_list, indices_list):
                            if item in codes: indices.remove((row, col))   


    def getNumTilesWidth(self):
        return self.numTilesWidth

    def getNumTilesHeight(self):
        return self.numTilesHeight

    def getTiles(self):
        return copy.deepcopy(self.tiles)

    def getEmptyIndices(self):
        return copy.deepcopy(self.emptyIndices)

    def getAgentIndices(self):
        return copy.deepcopy(self.agent_indices)

    def getGemIndices(self):
        return copy.deepcopy(self.gem_indices)

    def getRockIndices(self):
        return copy.deepcopy(self.rock_indices)

    def getKeyRedIndices(self):
        return copy.deepcopy(self.key_red_indices)
    
    def getKeyGreenIndices(self):
        return copy.deepcopy(self.key_green_indices)

    def getKeyBlueIndices(self):
        return copy.deepcopy(self.key_blue_indices)

    def getKeyYellowIndices(self):
        return copy.deepcopy(self.key_yellow_indices)

    def getBitIndices(self):
        return copy.deepcopy(self.bit_indices)

