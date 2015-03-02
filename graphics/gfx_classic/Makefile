# =============================================================================
# Rocks'n'Diamonds - McDuffin Strikes Back!
# -----------------------------------------------------------------------------
# (c) 1995-2015 by Artsoft Entertainment
#                  Holger Schemel
#                  info@artsoft.org
#                  http://www.artsoft.org/
# -----------------------------------------------------------------------------
# graphics/gfx_classic/Makefile
# =============================================================================

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

RND = ../rocksndiamonds

RM = rm -f

EXT = pcx

FILES = RocksBusy.$(EXT)	\
	RocksCE.$(EXT)		\
	RocksDC.$(EXT)		\
	RocksDC2.$(EXT)		\
	RocksDoor.$(EXT)	\
	RocksEMC.$(EXT)		\
	RocksElements.$(EXT)	\
	RocksFontBig.$(EXT)	\
	RocksFontDC.$(EXT)	\
	RocksFontEM.$(EXT)	\
	RocksFontMedium.$(EXT)	\
	RocksFontSmall.$(EXT)	\
	RocksHeroes.$(EXT)	\
	RocksMore.$(EXT)	\
	RocksSP.$(EXT)		\
	RocksScreen.$(EXT)	\
	RocksToons.$(EXT)

# FILES_GENERATED = RocksCE.$(EXT)


# -----------------------------------------------------------------------------
# build targets
# -----------------------------------------------------------------------------

all: $(FILES)

%.pcx: %.ilbm
	ilbmtoppm $< | ppmtopcx > $@

RocksCE.ilbm: RocksCE-template.ilbm
	$(RND) -e "create CE image RocksCE.bmp"
	bmptoppm RocksCE.bmp | ppmtoilbm > RocksCE.ilbm
	$(RM) RocksCE.bmp

clean-gfx:
	@for i in $(FILES); do			\
		$(RM) $$i;			\
	done
