import numpy as np
import copy
from tilesets.tile import Tile
import tilesets.tileset_3by3


class LevelTemplate():

    def __init__(self, numTilesWidth, numTilesHeight, backgroundTile, metaTiles, metaIndices,
    emptyTile, emptyIndices, agent_indices, gem_indices, rock_indices, key_indices=[],description=""):
        self.numTilesWidth = numTilesWidth
        self.numTilesHeight = numTilesHeight
        self.backgroundTile = backgroundTile
        self.emptyTile = emptyTile
        self.emptyIndices = emptyIndices
        self.metaTiles = metaTiles
        self.metaIndices = metaIndices
        self.description = description
        self.agent_indices = agent_indices
        self.gem_indices = gem_indices
        self.rock_indices = rock_indices
        self.key_indices = key_indices

        # Start with background tile
        self.tiles = np.array([[copy.deepcopy(backgroundTile)]*numTilesWidth]*numTilesHeight, dtype=Tile)

        # Place meta tiles
        for index, metaTile in zip(self.metaIndices, self.metaTiles):
            start_row, start_col = index
            tiles_indices = [(row, col) for row in range(start_row, start_row + metaTile.height) for col in range(start_col, start_col + metaTile.width)]
            tiles = metaTile.tiles.reshape(1, -1)[0]
            
            for index, tile in zip(tiles_indices, tiles):
                self.tiles[tuple(index)] = copy.deepcopy(tile)
                if index in empty_indices: empty_indices.remove(index)
                if index in key_indices: key_indices.remove(index)
                if index in agent_indices: agent_indices.remove(index)
                if index in gem_indices: gem_indices.remove(index)
                if index in rock_indices: rock_indices.remove(index)

        # Place empty indices
        for index in emptyIndices:
            self.tiles[tuple(index)] = copy.deepcopy(emptyTile)

    def getTiles(self):
        return copy.deepcopy(self.tiles)

    def getDescription(self):
        return self.description

    def getEmptyIndices(self):
        return copy.deepcopy(self.emptyIndices)

    def getAgentIndices(self):
        return copy.deepcopy(self.agent_indices)

    def getGemIndices(self):
        return copy.deepcopy(self.gem_indices)

    def getRockIndices(self):
        return copy.deepcopy(self.rock_indices)

    def getKeyIndices(self):
        return copy.deepcopy(self.key_indices)


empty_tile = tilesets.tileset_3by3.EMPTY_TILE
background_tile = tilesets.tileset_3by3.BLOCKING_TILE
exit_meta_tile = tilesets.tileset_3by3.META_EXIT1

exit_index = (3,3)
empty_indices = [(0,0), (0,1), (0,2), (0,3), (0,4), (1,4), (2,4), (3,4), 
                 (3,3), (3,2), (3,1), (3,0), (4, 3)]
agent_indices = empty_indices
gem_indices =   [(0,0), (0,1), (0,2), (0,3), (0,4), (3,4), 
                 (3,3), (3,2), (3,1), (3,0)]
rock_indices =  [(0,0), (0,1), (0,2), (0,3), (0,4), 
                 (3,3), (3,2), (3,1), (3,0)]



template_1 = LevelTemplate(5, 5, background_tile, [exit_meta_tile], [exit_index],  empty_tile, empty_indices, 
    agent_indices, gem_indices, rock_indices)

# --------------------------------------------------------
exit_index = (4,4)
empty_indices = [(0,0), (1,0), (1,1), (1,2), (1,3), (1,4), 
                 (0,4), (0,5), (0,6), (1,6), (2,6), (3,6),
                 (4,6), (4,5), (4,4), (4,3), (4,2), (4,1),
                 (4,0), (5,4)]
agent_indices = empty_indices

gem_indices =   [(0,0), (1,0), (1,1), (1,2), (1,3), (1,4), 
                 (0,4), (0,5), (0,6), (1,6), (2,6), (3,6),
                 (4,6), (4,5), (4,3), (4,2), (4,1), (4,0)]
rock_indices =  [(1,1), (1,2), (1,3), (0,5), (4,2), (4,1)]

m_tiles = [exit_meta_tile, tilesets.tileset_3by3.META_3]
m_indices = [exit_index, (1,3)]

template_2 = LevelTemplate(7, 6, background_tile, m_tiles, m_indices, empty_tile, empty_indices, 
agent_indices, gem_indices, rock_indices)

# --------------------------------------------------------

exit_index = (6,6)
empty_indices = [(0,0), (0,1), (0,2), (0,3), (0,4), (0,5), (0,6), (0,7), (0,8), 
    (0,9), (0,10), (0,11), (1,11), (2,11), (3,11), (4,11), (5,11), (6,11), 
    (6,0), (6,1), (6,2), (6,3), (6,4), (6,5), (6,6), (6,7), (6,8), (6,9), (6,10), 
    (2,2), (2,3), (2,4), (2,5), (2,6), (2,7), (2,8), (2,9), (3,2), (3,9),
    (4,2), (4,3), (4,4), (4,5), (4,6), (4,7), (4,8), (4,9)
]

agent_indices = [(0,0), (0,1), (0,2), (0,3), (0,4), (0,5), (0,6), (0,7), (0,8), 
    (0,9), (0,10), (0,11), (1,11), (2,11), (3,11), (4,11), (5,11), (6,11), 
    (6,0), (6,1), (6,2), (6,3), (6,4), (6,5), (6,6), (6,7), (6,8), (6,9), (6,10)
]

gem_indices = [(0,0), (0,9), 
    (6,11), 
    (6,0), (6,10), 
    (2,6), (2,7), (2,8)
]
rock_indices =  [(0,2), (0,3), (0,8), (0,9), (0,10), 
    (6,2), (6,3), (6,4), (6,8), (6,9), (6,10)
]
key_indices = [(0,0), (0,1), (0,2), (0,3), (0,4), (0,5), (0,6), (0,7), (0,8), 
    (0,9), (0,10), (0,11), (1,11), (2,11), (3,11), (4,11), (5,11), (6,11), 
    (6,0), (6,1), (6,2), (6,3), (6,4), (6,8), (6,9), (6,10)
]

m_tiles = [exit_meta_tile, tilesets.tileset_3by3.META_3, tilesets.tileset_3by3.META_GATE,
    tilesets.tileset_3by3.META_ROCK1, tilesets.tileset_3by3.META_GATE_BLOCKING1]
m_indices = [exit_index, (0,5), (4,10), (2,9), (4,9)]

template_3 = LevelTemplate(12, 8, background_tile, m_tiles, m_indices, empty_tile, empty_indices, 
    agent_indices, gem_indices, rock_indices, key_indices)