import os
import pathlib
import argparse

from level_generator.level import Level, LevelDifficulty
from tilesets.tileset_3by3 import tileSetInfo, tileSetInfo1
from tilesets.leveltemplates import template_1, template_2, template_3

# Byte offsets for level information
GEM_OFFSET              = int("0x8D", 16)
LVL_SIZE_OFFSET_HIGH    = int("0x96", 16)
LVL_SIZE_OFFSET_LOW     = int("0x97", 16)
ROW_OFFSET              = int("0x87", 16)
COL_OFFSET              = int("0x84", 16)
BODY_OFFSET             = int("0x98", 16)

# File name format
LEVEL_NAME      = '{:0>3d}.level'

# Binary level file header and footer formatting
TEMPLATE_HEDER  = 'binary-templates/level_template_header'
TEMPLATE_FOOTER = 'binary-templates/level_template_footer'
TEMPLATE_CONF   = 'binary-templates/levelinfo.conf'
CONF_NAME       = 'levelinfo.conf'


def _getTemplateData():
    # Return byte data in the header/footer template files
    with open(TEMPLATE_HEDER, 'rb+') as header_file,  open(TEMPLATE_FOOTER, 'rb+') as footer_file:
        header_data = header_file.read()
        footer_data = footer_file.read()
    return header_data, footer_data


def _setByte(file, offset, byte):
    # Set the byte at the given offset to the byte data given.
    # File pointer is then reset back to the end
    file.seek(offset, 0)
    file.write(byte)
    file.seek(0, 2)


def _setNumGems(file, numGems):
    # Set the number of gems required to solve the level (open the exit door)
    assert numGems < 256
    _setByte(file, GEM_OFFSET, bytes([numGems]))


def _setLevelSize(file, level_width, level_height):
    # Set the level size information
    # Data to be set is the level width/height and number of bytes containing level information
    assert level_width < 128
    assert level_height < 128

    level_size = 2 * level_width * level_height
    level_size_high = (level_size & 0xFF00) >> 8
    level_size_low = level_size & 0x00FF

    # Size needed to represent all tiles in the level
    _setByte(file, LVL_SIZE_OFFSET_HIGH,  bytes([level_size_high]))
    _setByte(file, LVL_SIZE_OFFSET_LOW,  bytes([level_size_low]))

    # Level size in height/width
    _setByte(file, ROW_OFFSET,  bytes([level_height]))
    _setByte(file, COL_OFFSET,  bytes([level_width]))


def _setLevelSprites(file, level):
    # Set the map data sprites for the given level
    data = level.getUnderlyingData()
    for sprite in data:
        sprite_high = (sprite & 0xFF00) >> 8
        sprite_low = sprite & 0x00FF
        file.write(bytes([sprite_high]))
        file.write(bytes([sprite_low]))


def makeFileFromLevel(filename, level):
    header_data, footer_data = _getTemplateData()

    # Write header
    output = open(filename, 'wb+')
    output.write(header_data)

    _setNumGems(output, level.levelDifficulty.numGemsRequired)
    _setLevelSize(output, level.getWidth(), level.getHeight())
    _setLevelSprites(output, level)

    # Write footer
    output.write(footer_data)
    output.close()

    return


def createLevelset(name, size, width, height, gem_tiles, rock_tiles, gems_required, bit_complexity):
    # if (gems_required > gem_tiles):
    #     print('Error: gems_required shouldn\'t be greater than gem_tiles')
    #     return

    # Create levelset directory
    levelset_dir = str(pathlib.Path(__file__).parent.parent.absolute()) + '/levels/' + name + '/'
    pathlib.Path(levelset_dir).mkdir(exist_ok=True)

    # Create levelinfo.conf
    with open(TEMPLATE_CONF, 'r') as input_conf, open(levelset_dir + CONF_NAME, 'w') as output_conf:
        conf_data = input_conf.read()
        conf_data = conf_data.replace('LEVELSET_NAME', name)
        conf_data = conf_data.replace('NUM_LEVELS', str(size))
        output_conf.write(conf_data)

    # Create template level
    levelDifficulty = LevelDifficulty(numGemTiles=gem_tiles, numRockTiles=rock_tiles, numGemsRequired=gems_required, bitComplexity=bit_complexity)
    level = Level(width, height, tileSetInfo1, levelDifficulty, template_3)

    # Create levels
    for i in range(1, size+1):
        level.randomizeLevel()
        makeFileFromLevel(levelset_dir + LEVEL_NAME.format(i), level)


MAX_LEVELSET_SIZE = 1000
def _check_levelset_size(value):
    try:
        size = int(value)
        if size < 1 or size >= MAX_LEVELSET_SIZE:
            raise argparse.ArgumentTypeError('value must be between [1, {}]'.format(MAX_LEVELSET_SIZE-1))
        return size
    except ValueError:
        raise argparse.ArgumentTypeError


DEFAULT_WIDTH = 5
DEFAULT_HEIGHT = 5
DEFAULT_GEM_TILES = 3
DEFAULT_ROCK_TILES = 2

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Levelset generator')
    parser.add_argument("-n", "--name", required=True, help="Levelset name")
    parser.add_argument("-s", "--size", required=True, type=_check_levelset_size, help="Number of levels to generate.")
    parser.add_argument("--width", required=False, type=int, default=DEFAULT_WIDTH, help="Width of levels in number of tiles.")
    parser.add_argument("--height", required=False, type=int, default=DEFAULT_HEIGHT, help="Height of levels in number of tiles.")
    parser.add_argument("--gem_tiles", required=False, type=int, default=DEFAULT_GEM_TILES, help="Number of gem tiles.")
    parser.add_argument("--rock_tiles", required=False, type=int, default=DEFAULT_ROCK_TILES, help="Number of rock tiles.")
    parser.add_argument("--gems_required", required=False, type=int, default=1, help="Number of gem required to solve the level.")
    parser.add_argument("--bit_complexity", required=False, type=int, default=0, help="Number wall bits to add.")
    args = parser.parse_args()

    createLevelset(args.name, args.size, args.width, args.height, args.gem_tiles, args.rock_tiles, args.gems_required, args.bit_complexity)
