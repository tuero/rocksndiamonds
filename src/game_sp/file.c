
#include "main_sp.h"
#include "global.h"


/* ------------------------------------------------------------------------- */
/* functions for loading Supaplex level                                      */
/* ------------------------------------------------------------------------- */

void setLevelInfoToDefaults_SP()
{
  LevelInfoType *header = &native_sp_level.header;
  char *empty_title = "-------- EMPTY --------";
  int i, x, y;

  native_sp_level.width  = SP_PLAYFIELD_WIDTH;
  native_sp_level.height = SP_PLAYFIELD_HEIGHT;

  for (x = 0; x < native_sp_level.width; x++)
    for (y = 0; y < native_sp_level.height; y++)
      native_sp_level.playfield[x][y] = fiSpace;

  /* copy string (without terminating '\0' character!) */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = empty_title[i];

  header->InitialGravity = 0;
  header->Version = 0;
  header->InitialFreezeZonks = 0;
  header->InfotronsNeeded = 0;
  header->SpecialPortCount = 0;
  header->SpeedByte = 0;
  header->CheckSumByte = 0;
  header->DemoRandomSeed = 0;

  for (i = 0; i < SP_MAX_SPECIAL_PORTS; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];

    port->PortLocation = 0;
    port->Gravity = 0;
    port->FreezeZonks = 0;
    port->FreezeEnemies = 0;
  }

  native_sp_level.demo_available = FALSE;
  native_sp_level.demo_length = 0;
}

void copyInternalEngineVars_SP()
{
  int i, x, y;

  LInfo = native_sp_level.header;

  FieldWidth  = native_sp_level.width;
  FieldHeight = native_sp_level.height;
  HeaderSize = 96;

  FieldMax = (FieldWidth * FieldHeight) + HeaderSize - 1;
  LevelMax = (FieldWidth * FieldHeight) - 1;

  FileMax = FieldMax + native_sp_level.demo_length;

  PlayField8 = REDIM_1D(sizeof(byte), 0, FileMax + 1 - 1);
  DisPlayField = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);
  PlayField16 = REDIM_1D(sizeof(int), -FieldWidth, FieldMax);

  for (i = 0, y = 0; y < native_sp_level.height; y++)
  {
    for (x = 0; x < native_sp_level.width; x++)
    {
      PlayField8[i] = native_sp_level.playfield[x][y];

      PlayField16[i] = PlayField8[i];
      DisPlayField[i] = PlayField8[i];
      PlayField8[i] = 0;

      i++;
    }
  }

  if (native_sp_level.demo_available)
  {
    DemoAvailable = True;

    for (i = 0; i < native_sp_level.demo_length; i++)
      PlayField8[FieldMax + i + 1] = native_sp_level.demo[i];
  }

  AnimationPosTable = REDIM_1D(sizeof(int), 0, LevelMax - 2 * FieldWidth);
  AnimationSubTable = REDIM_1D(sizeof(byte), 0, LevelMax - 2 * FieldWidth);
  TerminalState = REDIM_1D(sizeof(byte), 0, FieldMax + 1 - 1);

  DemoPointer = FieldMax + 1;
  DemoOffset = DemoPointer;
  DemoKeyRepeatCounter = 0;

  GravityFlag = LInfo.InitialGravity;
  FreezeZonks = LInfo.InitialFreezeZonks;

  RandomSeed = LInfo.DemoRandomSeed;

  LevelLoaded = True;
}

static void LoadNativeLevelFromFileStream_SP(FILE *file, boolean demo_available)
{
  LevelInfoType *header = &native_sp_level.header;
  int i, x, y;

  /* for details of the Supaplex level format, see Herman Perk's Supaplex
     documentation file "SPFIX63.DOC" from his Supaplex "SpeedFix" package */

  native_sp_level.width  = SP_PLAYFIELD_WIDTH;
  native_sp_level.height = SP_PLAYFIELD_HEIGHT;

  /* read level playfield (width * height == 60 * 24 tiles == 1440 bytes) */
  for (y = 0; y < native_sp_level.height; y++)
    for (x = 0; x < native_sp_level.width; x++)
      native_sp_level.playfield[x][y] = getFile8Bit(file);

  /* read level header (96 bytes) */

  ReadUnusedBytesFromFile(file, 4);	/* (not used by Supaplex engine) */

  /* initial gravity: 1 == "on", anything else (0) == "off" */
  header->InitialGravity = getFile8Bit(file);

  /* SpeedFixVersion XOR 0x20 */
  header->Version = getFile8Bit(file);

  /* level title in uppercase letters, padded with dashes ("-") (23 bytes) */
  for (i = 0; i < SP_LEVEL_NAME_LEN; i++)
    header->LevelTitle[i] = getFile8Bit(file);

  /* initial "freeze zonks": 2 == "on", anything else (0, 1) == "off" */
  header->InitialFreezeZonks = getFile8Bit(file);

  /* number of infotrons needed; 0 means that Supaplex will count the total
     amount of infotrons in the level and use the low byte of that number
     (a multiple of 256 infotrons will result in "0 infotrons needed"!) */
  header->InfotronsNeeded = getFile8Bit(file);

  /* number of special ("gravity") port entries below (maximum 10 allowed) */
  header->SpecialPortCount = getFile8Bit(file);

#if 0
  printf("::: num_special_ports == %d\n", header->SpecialPortCount);
#endif

  /* database of properties of up to 10 special ports (6 bytes per port) */
  for (i = 0; i < SP_MAX_SPECIAL_PORTS; i++)
  {
    SpecialPortType *port = &header->SpecialPort[i];

    /* high and low byte of the location of a special port; if (x, y) are the
       coordinates of a port in the field and (0, 0) is the top-left corner,
       the 16 bit value here calculates as 2 * (x + (y * 60)) (this is twice
       of what may be expected: Supaplex works with a game field in memory
       which is 2 bytes per tile) */
    port->PortLocation = getFile16BitBE(file);

#if 0
    {
      int port_x = (port->PortLocation / 2) % SP_PLAYFIELD_WIDTH;
      int port_y = (port->PortLocation / 2) / SP_PLAYFIELD_WIDTH;

      printf("::: %d: port_location == %d => (%d, %d)\n",
	     i, port->PortLocation, port_x, port_y);
    }
#endif

    /* change gravity: 1 == "turn on", anything else (0) == "turn off" */
    port->Gravity = getFile8Bit(file);

    /* "freeze zonks": 2 == "turn on", anything else (0, 1) == "turn off" */
    port->FreezeZonks = getFile8Bit(file);

    /* "freeze enemies": 1 == "turn on", anything else (0) == "turn off" */
    port->FreezeEnemies = getFile8Bit(file);

    ReadUnusedBytesFromFile(file, 1);	/* (not used by Supaplex engine) */
  }

  /* SpeedByte XOR Highbyte(RandomSeed) */
  header->SpeedByte = getFile8Bit(file);

  /* CheckSum XOR SpeedByte */
  header->CheckSumByte = getFile8Bit(file);

  /* random seed used for recorded demos */
  header->DemoRandomSeed = getFile16BitLE(file);

#if 1
  printf("::: file.c: DemoRandomSeed == %d\n", header->DemoRandomSeed);
#endif

  /* auto-determine number of infotrons if it was stored as "0" -- see above */
  if (header->InfotronsNeeded == 0)
  {
    for (x = 0; x < native_sp_level.width; x++)
      for (y = 0; y < native_sp_level.height; y++)
	if (native_sp_level.playfield[x][y] == fiInfotron)
	  header->InfotronsNeeded++;

    header->InfotronsNeeded &= 0xff;	/* only use low byte -- see above */
  }

  /* also load demo tape, if available */

  if (demo_available)
  {
    for (i = 0; i < SP_MAX_TAPE_LEN && !feof(file); i++)
    {
      native_sp_level.demo[i] = getFile8Bit(file);

      if (native_sp_level.demo[i] == 0xff)	/* "end of demo" byte */
      {
	i++;

	break;
      }
    }

    native_sp_level.demo_length = i;
    native_sp_level.demo_available = (native_sp_level.demo_length > 0);
  }
}

boolean LoadNativeLevel_SP(char *filename, int pos)
{
  FILE *file;
  int i, l, x, y;
  char name_first, name_last;
  struct LevelInfo_SP multipart_level;
  int multipart_xpos, multipart_ypos;
  boolean is_multipart_level;
  boolean is_first_part;
  boolean reading_multipart_level = FALSE;
  boolean use_empty_level = FALSE;
  LevelInfoType *header = &native_sp_level.header;
  boolean demo_available = (strSuffix(filename, ".sp") ||
			    strSuffix(filename, ".SP"));

  /* always start with reliable default values */
  setLevelInfoToDefaults_SP();
  copyInternalEngineVars_SP();

  if (!(file = fopen(filename, MODE_READ)))
  {
    Error(ERR_WARN, "cannot open level '%s' -- using empty level", filename);

    return FALSE;
  }

  /* position file stream to the requested level (in case of level package) */
  if (fseek(file, pos * SP_LEVEL_SIZE, SEEK_SET) != 0)
  {
    Error(ERR_WARN, "cannot fseek in file '%s' -- using empty level", filename);

    return FALSE;
  }

  /* there exist Supaplex level package files with multi-part levels which
     can be detected as follows: instead of leading and trailing dashes ('-')
     to pad the level name, they have leading and trailing numbers which are
     the x and y coordinations of the current part of the multi-part level;
     if there are '?' characters instead of numbers on the left or right side
     of the level name, the multi-part level consists of only horizontal or
     vertical parts */

  for (l = pos; l < SP_NUM_LEVELS_PER_PACKAGE; l++)
  {
    LoadNativeLevelFromFileStream_SP(file, demo_available);

    /* check if this level is a part of a bigger multi-part level */

    name_first = header->LevelTitle[0];
    name_last  = header->LevelTitle[SP_LEVEL_NAME_LEN - 1];

    is_multipart_level =
      ((name_first == '?' || (name_first >= '0' && name_first <= '9')) &&
       (name_last  == '?' || (name_last  >= '0' && name_last  <= '9')));

    is_first_part =
      ((name_first == '?' || name_first == '1') &&
       (name_last  == '?' || name_last  == '1'));

    /* correct leading multipart level meta information in level name */
    for (i = 0; i < SP_LEVEL_NAME_LEN && header->LevelTitle[i] == name_first; i++)
      header->LevelTitle[i] = '-';

    /* correct trailing multipart level meta information in level name */
    for (i = SP_LEVEL_NAME_LEN - 1; i >= 0 && header->LevelTitle[i] == name_last; i--)
      header->LevelTitle[i] = '-';

    /* ---------- check for normal single level ---------- */

    if (!reading_multipart_level && !is_multipart_level)
    {
      /* the current level is simply a normal single-part level, and we are
	 not reading a multi-part level yet, so return the level as it is */

      break;
    }

    /* ---------- check for empty level (unused multi-part) ---------- */

    if (!reading_multipart_level && is_multipart_level && !is_first_part)
    {
      /* this is a part of a multi-part level, but not the first part
	 (and we are not already reading parts of a multi-part level);
	 in this case, use an empty level instead of the single part */

      use_empty_level = TRUE;

      break;
    }

    /* ---------- check for finished multi-part level ---------- */

    if (reading_multipart_level &&
	(!is_multipart_level ||
	 !strEqualN(header->LevelTitle, multipart_level.header.LevelTitle,
		    SP_LEVEL_NAME_LEN)))
    {
      /* we are already reading parts of a multi-part level, but this level is
	 either not a multi-part level, or a part of a different multi-part
	 level; in both cases, the multi-part level seems to be complete */

      break;
    }

    /* ---------- here we have one part of a multi-part level ---------- */

    reading_multipart_level = TRUE;

    if (is_first_part)	/* start with first part of new multi-part level */
    {
      /* copy level info structure from first part */
      multipart_level = native_sp_level;

      /* clear playfield of new multi-part level */
      for (x = 0; x < SP_MAX_PLAYFIELD_WIDTH; x++)
	for (y = 0; y < SP_MAX_PLAYFIELD_HEIGHT; y++)
	  multipart_level.playfield[x][y] = fiSpace;
    }

    if (name_first == '?')
      name_first = '1';
    if (name_last == '?')
      name_last = '1';

    multipart_xpos = (int)(name_first - '0');
    multipart_ypos = (int)(name_last  - '0');

#if 0
    printf("----------> part (%d/%d) of multi-part level '%s'\n",
	   multipart_xpos, multipart_ypos, multipart_level.header.LevelTitle);
#endif

    if (multipart_xpos * SP_PLAYFIELD_WIDTH  > SP_MAX_PLAYFIELD_WIDTH ||
	multipart_ypos * SP_PLAYFIELD_HEIGHT > SP_MAX_PLAYFIELD_HEIGHT)
    {
      Error(ERR_WARN, "multi-part level is too big -- ignoring part of it");

      break;
    }

    multipart_level.width  = MAX(multipart_level.width,
				 multipart_xpos * SP_PLAYFIELD_WIDTH);
    multipart_level.height = MAX(multipart_level.height,
				 multipart_ypos * SP_PLAYFIELD_HEIGHT);

    /* copy level part at the right position of multi-part level */
    for (x = 0; x < SP_PLAYFIELD_WIDTH; x++)
    {
      for (y = 0; y < SP_PLAYFIELD_HEIGHT; y++)
      {
	int start_x = (multipart_xpos - 1) * SP_PLAYFIELD_WIDTH;
	int start_y = (multipart_ypos - 1) * SP_PLAYFIELD_HEIGHT;

	multipart_level.playfield[start_x + x][start_y + y] =
	  native_sp_level.playfield[x][y];
      }
    }
  }

  fclose(file);

  if (use_empty_level)
  {
    setLevelInfoToDefaults_SP();

    Error(ERR_WARN, "single part of multi-part level -- using empty level");
  }

  if (reading_multipart_level)
    native_sp_level = multipart_level;

  copyInternalEngineVars_SP();

  return TRUE;
}
