from PIL import Image
import enum


# Sprite enum to match binary values in engine
class Sprite(enum.IntEnum):
    null            = -1
    empty           = 0x0000
    dirt            = 0x0001
    rock            = 0x006A
    key_red         = 0x0268
    key_yellow      = 0x0269
    key_green       = 0x026A
    key_blue        = 0x026B
    gate_red        = 0x00CB
    gate_yellow     = 0x00CC
    gate_green      = 0x00CD
    gate_blue       = 0x00CE
    door_exit       = 0x0007
    wall_steel      = 0x000D
    wall_round      = 0x0003
    gem_diamond     = 0x0038
    agent           = 0x0050
    bitwall         = 0x0002


# Transposition flags
class BitField(enum.IntEnum):
    R_90    = 1 << 0        # Rotate 90
    R_180   = 1 << 1        # Rotate 180
    R_270   = 1 << 2        # Rotate 270
    FV      = 1 << 3        # Flip vertical axis
    FVR_90  = 1 << 4        # Flip vertical axis then rotate 90
    FVR_180 = 1 << 5        # Flip vertical axis then rotate 180
    FVR_270 = 1 << 6        # Flip vertical axis then rotate 270
    FH      = 1 << 7        # Flip horizontal axis
    FHR_90  = 1 << 8        # Flip horizontal axis then rotate 90
    FHR_180 = 1 << 9        # Flip horizontal axis then rotate 180
    FHR_270 = 1 << 10       # Flip horizontal axis then rotate 270


TRANSPOSITION_BIT_FLAGS = [
    BitField.R_90,
    BitField.R_180,
    BitField.R_270,
    BitField.FV,
    BitField.FVR_90,
    BitField.FVR_180,
    BitField.FVR_270,
    BitField.FH,
    BitField.FHR_90,
    BitField.FHR_180,
    BitField.FHR_270
]


# Pixel definitions for level PNG viewing
SPRITE_PIXEL_HEIGHT = 32
SPRITE_PIXEL_WIDTH  = 32
SPRITES_DIR = 'sprites/'
SPRITE_IMG = {
    Sprite.empty        :    Image.open(SPRITES_DIR + 'empty.png'),
    Sprite.dirt         :    Image.open(SPRITES_DIR + 'dirt.png'),
    Sprite.rock         :    Image.open(SPRITES_DIR + 'rock.png'),
    Sprite.gem_diamond  :    Image.open(SPRITES_DIR + 'gem_diamond.png'),
    Sprite.key_red      :    Image.open(SPRITES_DIR + 'key_red.png'),
    Sprite.key_yellow   :    Image.open(SPRITES_DIR + 'key_yellow.png'),
    Sprite.key_green    :    Image.open(SPRITES_DIR + 'key_green.png'),
    Sprite.key_blue     :    Image.open(SPRITES_DIR + 'key_blue.png'),
    Sprite.gate_red     :    Image.open(SPRITES_DIR + 'gate_red.png'),
    Sprite.gate_yellow  :    Image.open(SPRITES_DIR + 'gate_yellow.png'),
    Sprite.gate_green   :    Image.open(SPRITES_DIR + 'gate_green.png'),
    Sprite.door_exit    :    Image.open(SPRITES_DIR + 'door_exit.png'),
    Sprite.wall_steel   :    Image.open(SPRITES_DIR + 'wall_steel.png'),
    Sprite.bitwall      :    Image.open(SPRITES_DIR + 'wall_steel.png'),
    Sprite.wall_round   :    Image.open(SPRITES_DIR + 'wall_round.png'),
    Sprite.agent        :    Image.open(SPRITES_DIR + 'agent.png')
}

SpriteInfo = {'width' : SPRITE_PIXEL_WIDTH, 'height' : SPRITE_PIXEL_HEIGHT, 'img' : SPRITE_IMG}