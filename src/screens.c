/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  ©1995 Artsoft Development                               *
*        Holger Schemel                                    *
*        33659 Bielefeld-Senne                             *
*        Telefon: (0521) 493245                            *
*        eMail: aeglos@valinor.owl.de                      *
*               aeglos@uni-paderborn.de                    *
*               q99492@pbhrzx.uni-paderborn.de             *
*----------------------------------------------------------*
*  screens.c                                               *
***********************************************************/

#include "screens.h"
#include "events.h"
#include "sound.h"
#include "game.h"
#include "tools.h"
#include "editor.h"
#include "misc.h"
#include "files.h"
#include "buttons.h"

void DrawMainMenu()
{
  int i;

  FadeSounds();
  GetPlayerConfig();
  LoadLevel(level_nr);

  ClearWindow();
  DrawText(SX+16, SY+8,  "ROCKS'N'DIAMONDS",FS_BIG,FC_YELLOW);
  DrawText(SX+25+16, SY+46, "Copyright ^1995 by Holger Schemel",
	   FS_SMALL,FC_RED);
  DrawText(SX+32, SY+64, "Name:",FS_BIG,FC_GREEN);
  DrawText(SX+192,SY+64, player.alias_name,FS_BIG,FC_RED);
  DrawText(SX+32, SY+96, "Level:",FS_BIG,FC_GREEN);
  DrawText(SX+352,SY+96, int2str(level_nr,3),FS_BIG,
	   (level_nr<leveldir[leveldir_nr].num_ready ? FC_RED : FC_YELLOW));
  DrawText(SX+32, SY+128,"Hall Of Fame",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+160,"Level Creator",FS_BIG,FC_GREEN);
  DrawText(SY+32, SY+192,"Info Screen",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+224,"Start Game",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+256,"Setup",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+288,"Quit",FS_BIG,FC_GREEN);

  DrawMicroLevel(MICROLEV_XPOS,MICROLEV_YPOS);

  for(i=2;i<10;i++)
    DrawGraphic(0,i,GFX_KUGEL_BLAU);
  DrawGraphic(10,3,GFX_KUGEL_BLAU);
  DrawGraphic(14,3,GFX_KUGEL_BLAU);

  DrawText(SX+54+16,SY+326,"A Game by Artsoft Development",FS_SMALL,FC_BLUE);
  DrawText(SX+40+16,SY+344,"Graphics: Deluxe Paint IV Amiga",
	   FS_SMALL,FC_BLUE);
  DrawText(SX+60+16,SY+362,"Sounds: AudioMaster IV Amiga",
	   FS_SMALL,FC_BLUE);

  FadeToFront();
  InitAnimation();
  HandleMainMenu(0,0,0,0,MB_MENU_MARK);

  TapeStop();
  if (TAPE_IS_EMPTY(tape))
    LoadLevelTape(level_nr);
  DrawCompleteVideoDisplay();

  OpenDoor(DOOR_CLOSE_1 | DOOR_OPEN_2);

  XAutoRepeatOn(display);
}

void HandleMainMenu(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;

  if (redraw)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (dx || dy)
  {
    if (dx && choice==4)
    {
      x = (dx<0 ? 11 : 15);
      y = 4;
    }
    else if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y<3)
      y = 3;
    else if (y>10)
      y = 10;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (y==4 && ((x==11 && level_nr>0) ||
	       (x==15 && level_nr<LEVELDIR_SIZE(leveldir[leveldir_nr]))) &&
      button)
  {
    static long level_delay = 0;
    int step = (button==1 ? 1 : button==2 ? 5 : 10);

    if (!DelayReached(&level_delay,20))
      goto out;

    level_nr += (x==11 ? -step : +step);
    if (level_nr<0)
      level_nr = 0;
    if (level_nr>LEVELDIR_SIZE(leveldir[leveldir_nr])-1)
      level_nr = LEVELDIR_SIZE(leveldir[leveldir_nr])-1;

    if (level_nr>player.handicap && level_nr<leveldir[leveldir_nr].num_ready)
    {
      if (x==11 || leveldir[leveldir_nr].num_free==0)
	level_nr = player.handicap;
      else
	level_nr = leveldir[leveldir_nr].num_ready;
    }

    DrawTextExt(drawto,gc,SX+352,SY+96, int2str(level_nr,3),FS_BIG,
		(level_nr<leveldir[leveldir_nr].num_ready ?FC_RED :FC_YELLOW));
    DrawTextExt(window,gc,SX+352,SY+96,	int2str(level_nr,3),FS_BIG,
		(level_nr<leveldir[leveldir_nr].num_ready ?FC_RED :FC_YELLOW));

    LoadLevel(level_nr);
    DrawMicroLevel(MICROLEV_XPOS,MICROLEV_YPOS);

    TapeErase();
    LoadLevelTape(level_nr);
    DrawCompleteVideoDisplay();
  }
  else if (x==1 && y>=3 && y<=10)
  {
    if (button)
    {
      if (y!=choice)
      {
	DrawGraphic(0,y-1,GFX_KUGEL_ROT);
	DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      if (y==3)
      {
	game_status = TYPENAME;
	HandleTypeName(strlen(player.alias_name),0);
      }
      else if (y==4)
      {
	if (num_leveldirs)
	{
	  game_status = CHOOSELEVEL;
	  DrawChooseLevel();
	  redraw = TRUE;
	}
      }
      else if (y==5)
      {
	game_status = HALLOFFAME;
	DrawHallOfFame(-1);
	redraw = TRUE;
      }
      else if (y==6)
      {
	game_status = LEVELED;
	DrawLevelEd();
	redraw = TRUE;
      }
      else if (y==7)
      {
	game_status = HELPSCREEN;
	DrawHelpScreen();
	redraw = TRUE;
      }
      else if (y==8)
      {
	if (autorecord_on && !tape.playing)
	  TapeInitRecording();

	game_status = PLAYING;
	InitGame();
	redraw = TRUE;
      }
      else if (y==9)
      {
	game_status = SETUP;
	DrawSetupScreen();
	redraw = TRUE;
      }
      else if (y==10)
      {
        if (AreYouSure("Do you really want to quit ?",AYS_ASK|AYS_STAY_CLOSED))
	  game_status = EXITGAME;
      }
    }
  }
  BackToFront();

  out:

  if (game_status==MAINMENU)
    DoAnimation();
}

#define MAX_HELPSCREEN_ELS	10
#define HA_NEXT			-999
#define HA_END			-1000

static long helpscreen_state;
static int helpscreen_step[MAX_HELPSCREEN_ELS];
static int helpscreen_frame[MAX_HELPSCREEN_ELS];
static int helpscreen_delay[MAX_HELPSCREEN_ELS];
static int helpscreen_action[] =
{
  GFX_ERDREICH,1,100,						HA_NEXT,
  GFX_LEERRAUM,1,100,						HA_NEXT,
  GFX_MORAST_LEER,1,100,					HA_NEXT,
  GFX_BETON,1,100,						HA_NEXT,
  GFX_MAUERWERK,1,100,						HA_NEXT,
  GFX_FELSBODEN,1,100,						HA_NEXT,
  GFX_EDELSTEIN,2,5,						HA_NEXT,
  GFX_DIAMANT,2,5,						HA_NEXT,
  GFX_FELSBROCKEN,4,5,						HA_NEXT,
  GFX_BOMBE,1,50, GFX_EXPLOSION,8,1, GFX_LEERRAUM,1,10,		HA_NEXT,
  GFX_KOKOSNUSS,1,50, GFX_CRACKINGNUT,3,1, GFX_EDELSTEIN,1,10,	HA_NEXT,
  GFX_ERZ_EDEL,1,50, GFX_EXPLOSION,8,1, GFX_EDELSTEIN,1,10,	HA_NEXT,
  GFX_ERZ_DIAM,1,50, GFX_EXPLOSION,8,1, GFX_DIAMANT,1,10,	HA_NEXT,
  GFX_GEBLUBBER,4,4,						HA_NEXT,
  GFX_SCHLUESSEL1,4,33,						HA_NEXT,
  GFX_PFORTE1,4,33,						HA_NEXT,
  GFX_PFORTE1X,4,33,						HA_NEXT,
  GFX_DYNAMIT_AUS,1,100,					HA_NEXT,
  GFX_DYNAMIT,7,6, GFX_EXPLOSION,8,1, GFX_LEERRAUM,1,10,	HA_NEXT,
  GFX_FLIEGER+4,1,3, GFX_FLIEGER+0,1,3, GFX_FLIEGER+4,1,3,
  GFX_FLIEGER+5,1,3, GFX_FLIEGER+1,1,3, GFX_FLIEGER+5,1,3,
  GFX_FLIEGER+6,1,3, GFX_FLIEGER+2,1,3, GFX_FLIEGER+6,1,3,
  GFX_FLIEGER+7,1,3, GFX_FLIEGER+3,1,3, GFX_FLIEGER+7,1,3,	HA_NEXT,
  GFX_KAEFER+4,1,1, GFX_KAEFER+0,1,1, GFX_KAEFER+4,1,1,
  GFX_KAEFER+5,1,1, GFX_KAEFER+1,1,1, GFX_KAEFER+5,1,1,
  GFX_KAEFER+6,1,1, GFX_KAEFER+2,1,1, GFX_KAEFER+6,1,1,
  GFX_KAEFER+7,1,1, GFX_KAEFER+3,1,1, GFX_KAEFER+7,1,1,		HA_NEXT,
  GFX_PACMAN+0,1,3, GFX_PACMAN+4,1,2, GFX_PACMAN+0,1,3,
  GFX_PACMAN+1,1,3, GFX_PACMAN+5,1,2, GFX_PACMAN+1,1,3,
  GFX_PACMAN+2,1,3, GFX_PACMAN+6,1,2, GFX_PACMAN+2,1,3,
  GFX_PACMAN+3,1,3, GFX_PACMAN+7,1,2, GFX_PACMAN+3,1,3,		HA_NEXT,
  GFX_MAMPFER+0,4,0, GFX_MAMPFER+3,1,0, GFX_MAMPFER+2,1,0,
  GFX_MAMPFER+1,1,0,						HA_NEXT,
  GFX_ZOMBIE+0,4,0, GFX_ZOMBIE+3,1,0, GFX_ZOMBIE+2,1,0,
  GFX_ZOMBIE+1,1,0,						HA_NEXT,
  GFX_ABLENK,4,1,						HA_NEXT,
  GFX_AMOEBE_LEBT,4,40,						HA_NEXT,
  GFX_AMOEBE_TOT+2,2,50, GFX_AMOEBE_TOT,2,50,			HA_NEXT,
  GFX_SIEB_LEER,4,2,						HA_NEXT,
  HA_END
};
static char *helpscreen_eltext[][2] =
{
  "Normal sand:", "You can dig through it",
  "Empty field:", "You can walk through it",
  "Quicksand: You cannot pass it,", "but rocks can fall though it",
  "Massive Wall:", "Nothing can go through it",
  "Normal Wall: You can't go through", "it, but you can bomb it away",
  "Old Wall: Like normal wall, but", "some things can fall down from it",
  "Emerald: You must collect enough of", "them to finish a level",
  "Diamond: Counts as 3 emeralds;", "Can be destroyed by rocks",
  "Rock: Smashes several things;", "Can be moved by the player",
  "Bomb: You can move it, but be", "careful when dropping it",
  "Nut: Throw a rock on it to open it;", "Each nut contains an emerald",
  "Wall with an Emerald inside:", "Bomb the wall away to get it",
  "Wall with a Diamond inside:", "Bomb the wall away to get it",
  "Acid: Destroys everything that", "falls or walks into it",
  "Key: Opens the door that has the", "same color (red/yellow/green/blue)",
  "Door: Can be opened by the key", "with the same color",
  "Door: You have to find out the", "right color of the key for it",
  "Dynamite: Collect it and use it to", "destroy walls or kill enemies",
  "Dynamite: This one explodes after", "a few seconds",
  "Spaceship: Moves at the left side", "of walls; don't touch it!",
  "Bug: Moves at the right side of", "walls; don't touch it!",
  "Pacman: Eats the amoeba and you,", "if you're not careful",
  "Cruncher: Eats diamonds and you,", "if you're not careful",
  "Robot: Tries to kill the player", "",
  "Magic Wheel: Touch it to get rid of", "the robots for some seconds",
  "Living Amoeba: Grows through empty", "fields, sand and quicksand",
  "Dead Amoeba: Does not grow, but", "can still kill bugs and spaceships",
  "Magic Wall: Changes rocks, emeralds", "and diamonds when they pass it",
};
static int num_helpscreen_els = sizeof(helpscreen_eltext)/(2*sizeof(char *));

static char *helpscreen_music[][3] =
{
  "Alchemy",			"Ian Boddy",		"Drive",
  "The Chase",			"Propaganda",		"A Secret Wish",
  "Network 23",			"Tangerine Dream",	"Exit",
  "Czardasz",			"Robert Pieculewicz",	"Czardasz",
  "21st Century Common Man",	"Tangerine Dream",	"Tyger",
  "Voyager",			"The Alan Parsons Project","Pyramid",
  "Twilight Painter",		"Tangerine Dream",	"Heartbreakers"
};
static int helpscreen_musicpos;

void DrawHelpScreenElAction(int start)
{
  int i = 0, j = 0;
  int frame, delay, graphic;
  int xstart = SX+16, ystart = SY+64+2*32, ystep = TILEY+4;

  while(helpscreen_action[j] != HA_END)
  {
    if (i>=start+MAX_HELPSCREEN_ELS || i>=num_helpscreen_els)
      break;
    else if (i<start || helpscreen_delay[i-start])
    {
      if (i>=start && helpscreen_delay[i-start])
	helpscreen_delay[i-start]--;

      while(helpscreen_action[j] != HA_NEXT)
	j++;
      j++;
      i++;
      continue;
    }

    j += 3*helpscreen_step[i-start];
    graphic = helpscreen_action[j++];

    if (helpscreen_frame[i-start])
    {
      frame = helpscreen_action[j++] - helpscreen_frame[i-start];
      helpscreen_frame[i-start]--;
    }
    else
    {
      frame = 0;
      helpscreen_frame[i-start] = helpscreen_action[j++]-1;
    }

    delay = helpscreen_action[j++];
    helpscreen_delay[i-start] = delay;

    if (helpscreen_action[j] == HA_NEXT)
    {
      if (!helpscreen_frame[i-start])
	helpscreen_step[i-start] = 0;
    }
    else
    {
      if (!helpscreen_frame[i-start])
	helpscreen_step[i-start]++;
      while(helpscreen_action[j] != HA_NEXT)
	j++;
    }
    j++;

    DrawGraphicExtHiRes(drawto,gc,xstart,ystart+(i-start)*ystep,
			graphic+frame);
    i++;
  }

  redraw_tiles += 28;
  for(i=2;i<16;i++)
    redraw[0][i] = redraw[1][i] = TRUE;
  redraw_mask |= REDRAW_TILES;
}

void DrawHelpScreenElText(int start)
{
  int i;
  int xstart = SX+56, ystart = SY+65+2*32, ystep = TILEY+4;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  ClearWindow();
  DrawText(SX+16, SY+8,  "ROCKS'N'DIAMONDS",FS_BIG,FC_YELLOW);
  DrawText(SX+25+16, SY+46, "Copyright ^1995 by Holger Schemel",
	   FS_SMALL,FC_RED);

  sprintf(text,"The game elements:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  for(i=start;i<start+MAX_HELPSCREEN_ELS && i<num_helpscreen_els;i++)
  {
    DrawText(xstart,ystart+(i-start)*ystep+(*helpscreen_eltext[i][1] ? 0 : 8),
	     helpscreen_eltext[i][0],FS_SMALL,FC_YELLOW);
    DrawText(xstart,ystart+(i-start)*ystep+16,
	     helpscreen_eltext[i][1],FS_SMALL,FC_YELLOW);
  }

  sprintf(text,"Press any key or button for next page");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);
}

void DrawHelpScreenMusicText(int num)
{
  int ystart = 150, ystep = 30;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  FadeSounds();
  ClearWindow();
  DrawText(SX+16, SY+8,  "ROCKS'N'DIAMONDS",FS_BIG,FC_YELLOW);
  DrawText(SX+25+16, SY+46, "Copyright ^1995 by Holger Schemel",
	   FS_SMALL,FC_RED);

  sprintf(text,"The game background music loops:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  sprintf(text,"Excerpt from");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+0*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%c%s%c",'\"',helpscreen_music[num][0],'\"');
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+1*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"by");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+2*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%s",helpscreen_music[num][1]);
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+3*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"from the album");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"%c%s%c",'\"',helpscreen_music[num][2],'\"');
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+5*ystep,
	   text,FS_SMALL,FC_RED);

  sprintf(text,"Press any key or button for next page");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);

  PlaySoundLoop(background_loop[num]);
}

void DrawHelpScreenCreditsText()
{
  int ystart = 150, ystep = 30;
  char text[FULL_SXSIZE/FONT2_XSIZE+10];

  FadeSounds();
  ClearWindow();
  DrawText(SX+16, SY+8,  "ROCKS'N'DIAMONDS",FS_BIG,FC_YELLOW);
  DrawText(SX+25+16, SY+46, "Copyright ^1995 by Holger Schemel",
	   FS_SMALL,FC_RED);

  sprintf(text,"Program information:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+100,
	   text,FS_SMALL,FC_GREEN);

  sprintf(text,"This game is Freeware!");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+0*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"If you like it, send e-mail to:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+1*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"aeglos@valinor.owl.de");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+2*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"or SnailMail to:");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+3*ystep,
	   text,FS_SMALL,FC_YELLOW);
  sprintf(text,"Holger Schemel");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"Sennehof 28");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+20,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"33659 Bielefeld");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+40,
	   text,FS_SMALL,FC_RED);
  sprintf(text,"Germany");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+ystart+4*ystep+60,
	   text,FS_SMALL,FC_RED);

  sprintf(text,"Press any key or button for main menu");
  DrawText(SX+(SXSIZE-strlen(text)*FONT2_XSIZE)/2,SY+SYSIZE-20,
	   text,FS_SMALL,FC_BLUE);
}

void DrawHelpScreen()
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  for(i=0;i<MAX_HELPSCREEN_ELS;i++)
    helpscreen_step[i] = helpscreen_frame[i] = helpscreen_delay[i] = 0;
  helpscreen_musicpos = 0;
  helpscreen_state = 0;
  DrawHelpScreenElText(0);
  DrawHelpScreenElAction(0);

  FadeToFront();
  InitAnimation();
  PlaySoundLoop(SND_RHYTHMLOOP);
}

void HandleHelpScreen(int button)
{
  static long hs_delay = 0;
  int num_helpscreen_els_pages =
    (num_helpscreen_els + MAX_HELPSCREEN_ELS-1) / MAX_HELPSCREEN_ELS;
  int button_released = !button;
  int i;

  if (button_released)
  {
    if (helpscreen_state<num_helpscreen_els_pages-1)
    {
      for(i=0;i<MAX_HELPSCREEN_ELS;i++)
	helpscreen_step[i] = helpscreen_frame[i] = helpscreen_delay[i] = 0;
      helpscreen_state++;
      DrawHelpScreenElText(helpscreen_state*MAX_HELPSCREEN_ELS);
      DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    else if (helpscreen_state<num_helpscreen_els_pages+num_bg_loops-1)
    {
      helpscreen_state++;
      DrawHelpScreenMusicText(helpscreen_state-num_helpscreen_els_pages);
    }
    else if (helpscreen_state==num_helpscreen_els_pages+num_bg_loops-1)
    {
      helpscreen_state++;
      DrawHelpScreenCreditsText();
    }
    else
    {
      FadeSounds();
      DrawMainMenu();
      game_status = MAINMENU;
    }
  }
  else
  {
    if (DelayReached(&hs_delay,3))
    {
      if (helpscreen_state<num_helpscreen_els_pages)
	DrawHelpScreenElAction(helpscreen_state*MAX_HELPSCREEN_ELS);
    }
    DoAnimation();
  }

  BackToFront();
}

void CheckCheat()
{
  int old_handicap = player.handicap;

  if (!strcmp(player.alias_name,"Artsoft"))
    player.handicap = MAX(0,leveldir[leveldir_nr].num_ready-1);

  if (player.handicap != old_handicap)
  {
    SavePlayerInfo(PLAYER_LEVEL);
    level_nr = player.handicap;
  }
}

void HandleTypeName(int newxpos, KeySym key)
{
  static int xpos = 0, ypos = 2;
  unsigned char ascii;

  if (newxpos)
  {
    xpos = newxpos;
    DrawText(SX+6*32,SY+ypos*32,player.alias_name,FS_BIG,FC_YELLOW);
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
    return;
  }

  if ((key>=XK_A && key<=XK_Z) || (key>=XK_a && key<=XK_z && 
      xpos<MAX_NAMELEN-1))
  {
    if (key>=XK_A && key<=XK_Z)
      ascii = 'A'+(char)(key-XK_A);
    if (key>=XK_a && key<=XK_z)
      ascii = 'a'+(char)(key-XK_a);
    player.alias_name[xpos] = ascii;
    player.alias_name[xpos+1] = 0;
    xpos++;
    DrawTextExt(drawto,gc,SX+6*32,SY+ypos*32,
		player.alias_name,FS_BIG,FC_YELLOW);
    DrawTextExt(window,gc,SX+6*32,SY+ypos*32,
		player.alias_name,FS_BIG,FC_YELLOW);
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
  }
  else if (key==XK_Delete && xpos>0)
  {
    player.alias_name[xpos] = 0;
    xpos--;
    DrawGraphic(xpos+6,ypos,GFX_KUGEL_ROT);
    DrawGraphic(xpos+7,ypos,GFX_LEERRAUM);
  }
  else if (key==XK_Return && xpos>0)
  {
    DrawText(SX+6*32,SY+ypos*32,player.alias_name,FS_BIG,FC_RED);
    DrawGraphic(xpos+6,ypos,GFX_LEERRAUM);
    SavePlayerInfo(PLAYER_SETUP);
    CheckCheat();

    game_status = MAINMENU;
    DrawMainMenu();
  }
  BackToFront();
}

void DrawChooseLevel()
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();
  DrawText(SX,SY,"Level Directories",FS_BIG,FC_GREEN);
  for(i=0;i<num_leveldirs;i++)
  {
    DrawText(SX+32,SY+(i+2)*32,leveldir[i].name,FS_BIG,FC_YELLOW);
    DrawGraphic(0,i+2,GFX_KUGEL_BLAU);
  }

  FadeToFront();
  InitAnimation();
  HandleChooseLevel(0,0,0,0,MB_MENU_MARK);
}

void HandleChooseLevel(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;

  if (redraw)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y<3)
      y = 3;
    else if (y>num_leveldirs+2)
      y = num_leveldirs+2;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (x==1 && y>=3 && y<=num_leveldirs+2)
  {
    if (button)
    {
      if (y!=choice)
      {
        DrawGraphic(0,y-1,GFX_KUGEL_ROT);
        DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      player.leveldir_nr = leveldir_nr = y-3;
      LoadPlayerInfo(PLAYER_LEVEL);
      SavePlayerInfo(PLAYER_SETUP);
      CheckCheat();

      game_status = MAINMENU;
      DrawMainMenu();
      redraw = TRUE;
    }
  }
  BackToFront();

  if (game_status==CHOOSELEVEL)
    DoAnimation();
}

void DrawHallOfFame(int pos)
{
  int y;
  char txt[40];

  CloseDoor(DOOR_CLOSE_2);

  if (pos<0) 
    LoadScore(level_nr);
  ClearWindow();
  DrawText(SX+64,SY+10,"Hall Of Fame",FS_BIG,FC_YELLOW);
  sprintf(txt,"HighScores of Level %d",level_nr);
  DrawText(SX+256-strlen(txt)*7,SY+48,txt,FS_SMALL,FC_RED);
  for(y=0;y<MAX_SCORE_ENTRIES;y++)
  {
    DrawText(SX,SY+64+y*32,".................",FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
    DrawText(SX,SY+64+y*32,highscore[y].Name,FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
    DrawText(SX+12*32,SY+64+y*32,
	     int2str(highscore[y].Score,5),FS_BIG,
	     (y==pos ? FC_RED : FC_GREEN));
  }

  FadeToFront();
  InitAnimation();
  PlaySound(SND_HALLOFFAME);
}

void HandleHallOfFame(int button)
{
  int button_released = !button;

  if (button_released)
  {
    FadeSound(SND_HALLOFFAME);
    game_status = MAINMENU;
    DrawMainMenu();
    BackToFront();
  }
  else
    DoAnimation();
}

void DrawSetupScreen()
{
  int i;

  CloseDoor(DOOR_CLOSE_2);

  ClearWindow();
  DrawText(SX+16, SY+16,  "SETUP",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+2*32,"Sound:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+3*32,"Sound loops:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+4*32,"Game music:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+5*32,"Toons:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+6*32,"Buffered gfx:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+7*32,"Fading:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+8*32,"Quick Doors:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+9*32,"Auto-Record:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+10*32,"Joystick:",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+11*32,"Cal. Joystick",FS_BIG,FC_GREEN);

  DrawText(SX+32, SY+13*32,"Exit",FS_BIG,FC_GREEN);
  DrawText(SX+32, SY+14*32,"Save and exit",FS_BIG,FC_GREEN);

  if (SETUP_SOUND_ON(player.setup))
    DrawText(SX+14*32, SY+2*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+2*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_SOUND_LOOPS_ON(player.setup))
    DrawText(SX+14*32, SY+3*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+3*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_SOUND_MUSIC_ON(player.setup))
    DrawText(SX+14*32, SY+4*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+4*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_TOONS_ON(player.setup))
    DrawText(SX+14*32, SY+5*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+5*32,"off",FS_BIG,FC_BLUE);

  if (!SETUP_DIRECT_DRAW_ON(player.setup))
    DrawText(SX+14*32, SY+6*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+6*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_FADING_ON(player.setup))
    DrawText(SX+14*32, SY+7*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+7*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_QUICK_DOORS_ON(player.setup))
    DrawText(SX+14*32, SY+8*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+8*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_RECORD_EACH_GAME_ON(player.setup))
    DrawText(SX+14*32, SY+9*32,"on",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+9*32,"off",FS_BIG,FC_BLUE);

  if (SETUP_2ND_JOYSTICK_ON(player.setup))
    DrawText(SX+14*32, SY+10*32,"2nd",FS_BIG,FC_YELLOW);
  else
    DrawText(SX+14*32, SY+10*32,"1st",FS_BIG,FC_YELLOW);

  for(i=2;i<15;i++)
    if (i!=12)
      DrawGraphic(0,i,GFX_KUGEL_BLAU);

  FadeToFront();
  InitAnimation();
  HandleSetupScreen(0,0,0,0,MB_MENU_MARK);
}

void HandleSetupScreen(int mx, int my, int dx, int dy, int button)
{
  static int choice = 3;
  static int redraw = TRUE;
  int x = (mx+32-SX)/32, y = (my+32-SY)/32;

  if (redraw)
  {
    DrawGraphic(0,choice-1,GFX_KUGEL_ROT);
    redraw = FALSE;
  }

  if (dx || dy)
  {
    if (dy)
    {
      x = 1;
      y = choice+dy;
    }
    else
      x = y = 0;

    if (y==13)
      y = (dy>0 ? 14 : 12);

    if (y<3)
      y = 3;
    else if (y>15)
      y = 15;
  }

  if (!mx && !my && !dx && !dy)
  {
    x = 1;
    y = choice;
  }

  if (x==1 && y>=3 && y<=15 && y!=13)
  {
    if (button)
    {
      if (y!=choice)
      {
	DrawGraphic(0,y-1,GFX_KUGEL_ROT);
	DrawGraphic(0,choice-1,GFX_KUGEL_BLAU);
      }
      choice = y;
    }
    else
    {
      int yy = y-1;

      if (y==3 && sound_status==SOUND_AVAILABLE)
      {
	if (SETUP_SOUND_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SOUND;
      }
      else if (y==4 && sound_loops_allowed)
      {
	if (SETUP_SOUND_LOOPS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SOUND_LOOPS;
      }
      else if (y==5 && sound_loops_allowed)
      {
	if (SETUP_SOUND_MUSIC_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_SOUND_MUSIC;
      }
      else if (y==6)
      {
	if (SETUP_TOONS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_TOONS;
      }
      else if (y==7)
      {
	if (!SETUP_DIRECT_DRAW_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_DIRECT_DRAW;
      }
      else if (y==8)
      {
	if (SETUP_FADING_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_FADING;
      }
      else if (y==9)
      {
	if (SETUP_QUICK_DOORS_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_QUICK_DOORS;
      }
      else if (y==10)
      {
	if (SETUP_RECORD_EACH_GAME_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"off",FS_BIG,FC_BLUE);
	else
	  DrawText(SX+14*32, SY+yy*32,"on ",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_RECORD_EACH_GAME;
      }
      else if (y==11)
      {
	if (SETUP_2ND_JOYSTICK_ON(player.setup))
	  DrawText(SX+14*32, SY+yy*32,"1st",FS_BIG,FC_YELLOW);
	else
	  DrawText(SX+14*32, SY+yy*32,"2nd",FS_BIG,FC_YELLOW);
	player.setup ^= SETUP_2ND_JOYSTICK;
      }
      else if (y==12)
      {
	CalibrateJoystick();
	redraw = TRUE;
      }
      else if (y==14 || y==15)
      {
        if (y==15)
	{
	  SavePlayerInfo(PLAYER_SETUP);
	  SaveJoystickData();
	}

	game_status = MAINMENU;
	DrawMainMenu();
	redraw = TRUE;
      }
    }
  }
  BackToFront();

  if (game_status==SETUP)
    DoAnimation();
}

void CalibrateJoystick()
{
#ifdef __FreeBSD__
  struct joystick joy_ctrl;
#else
  struct joystick_control
  {
    int buttons;
    int x;
    int y;
  } joy_ctrl;
#endif

  int new_joystick_xleft, new_joystick_xright, new_joystick_xmiddle;
  int new_joystick_yupper, new_joystick_ylower, new_joystick_ymiddle;

  if (joystick_status==JOYSTICK_OFF)
    goto error_out;

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+8*32, " THE UPPER LEFT ",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+9*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xleft = joy_ctrl.x;
  new_joystick_yupper = joy_ctrl.y;

  ClearWindow();
  DrawText(SX+16, SY+7*32, "MOVE JOYSTICK TO",FS_BIG,FC_YELLOW);
  DrawText(SX+32, SY+8*32, "THE LOWER RIGHT",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+9*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xright = joy_ctrl.x;
  new_joystick_ylower = joy_ctrl.y;

  ClearWindow();
  DrawText(SX+32, SY+16+7*32, "CENTER JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+16+8*32, "AND PRESS BUTTON",FS_BIG,FC_YELLOW);
  BackToFront();

#ifdef __FreeBSD__
  joy_ctrl.b1 = joy_ctrl.b2 = 0;
#else
  joy_ctrl.buttons = 0;
#endif
  while(Joystick() & JOY_BUTTON);
#ifdef __FreeBSD__
  while(!(joy_ctrl.b1||joy_ctrl.b2))
#else
  while(!joy_ctrl.buttons)
#endif
  {
    if (read(joystick_device, &joy_ctrl, sizeof(joy_ctrl)) != sizeof(joy_ctrl))
    {
      joystick_status=JOYSTICK_OFF;
      goto error_out;
    }
    Delay(10000);
  }

  new_joystick_xmiddle = joy_ctrl.x;
  new_joystick_ymiddle = joy_ctrl.y;

  joystick[joystick_nr].xleft = new_joystick_xleft;
  joystick[joystick_nr].yupper = new_joystick_yupper;
  joystick[joystick_nr].xright = new_joystick_xright;
  joystick[joystick_nr].ylower = new_joystick_ylower;
  joystick[joystick_nr].xmiddle = new_joystick_xmiddle;
  joystick[joystick_nr].ymiddle = new_joystick_ymiddle;

  CheckJoystickData();

  DrawSetupScreen();
  while(Joystick() & JOY_BUTTON);
  return;

  error_out:

  ClearWindow();
  DrawText(SX+16, SY+16, "NO JOYSTICK",FS_BIG,FC_YELLOW);
  DrawText(SX+16, SY+48, " AVAILABLE ",FS_BIG,FC_YELLOW);
  Delay(3000000);
  DrawSetupScreen();
}

void HandleVideoButtons(int mx, int my, int button)
{
  if (game_status!=MAINMENU && game_status!=PLAYING)
    return;

  switch(CheckVideoButtons(mx,my,button))
  {
    case BUTTON_VIDEO_EJECT:
      TapeStop();
      if (!TAPE_IS_EMPTY(tape))
	SaveLevelTape(tape.level_nr);
      else
	AreYouSure("Tape is empty !",AYS_CONFIRM);
      DrawCompleteVideoDisplay();
      break;
    case BUTTON_VIDEO_STOP:
      TapeStop();
      break;
    case BUTTON_VIDEO_PAUSE:
      TapeTogglePause();
      break;
    case BUTTON_VIDEO_REC:
      if (tape.pausing)
	TapeTogglePause();
      else if (game_status==MAINMENU)
	TapeInitRecording();
      break;
    case BUTTON_VIDEO_PLAY:
      if (tape.pausing)
	TapeTogglePause();
      else if (game_status==MAINMENU)
	TapeInitPlaying();
      break;
    default:
      break;
  }
}

void HandleSoundButtons(int mx, int my, int button)
{
  if (game_status!=PLAYING)
    return;

  switch(CheckSoundButtons(mx,my,button))
  {
    case BUTTON_SOUND_MUSIC:
      if (sound_music_on)
      { 
	sound_music_on = FALSE;
	player.setup &= ~SETUP_SOUND_MUSIC;
	FadeSound(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      }
      else if (sound_loops_allowed)
      { 
	sound_music_on = TRUE;
	player.setup |= SETUP_SOUND_MUSIC;
	PlaySoundLoop(background_loop[level_nr % num_bg_loops]);
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_MUSIC_OFF);
      break;
    case BUTTON_SOUND_LOOPS:
      if (sound_loops_on)
      { 
	sound_loops_on = FALSE;
	player.setup &= ~SETUP_SOUND_LOOPS;
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      }
      else if (sound_loops_allowed)
      { 
	sound_loops_on = TRUE;
	player.setup |= SETUP_SOUND_LOOPS;
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_LOOPS_OFF);
      break;
    case BUTTON_SOUND_SOUND:
      if (sound_on)
      { 
	sound_on = FALSE;
	player.setup &= ~SETUP_SOUND;
	DrawSoundDisplay(BUTTON_SOUND_SOUND_OFF);
      }
      else if (sound_status==SOUND_AVAILABLE)
      { 
	sound_on = TRUE;
	player.setup |= SETUP_SOUND;
	DrawSoundDisplay(BUTTON_SOUND_SOUND_ON);
      }
      else
	DrawSoundDisplay(BUTTON_SOUND_SOUND_OFF);
      break;
    default:
      break;
  }
}

void HandleGameButtons(int mx, int my, int button)
{
  if (game_status!=PLAYING)
    return;

  switch(CheckGameButtons(mx,my,button))
  {
    case BUTTON_GAME_STOP:
      if (AreYouSure("Do you really want to quit the game ?",
		      AYS_ASK | AYS_STAY_CLOSED))
      { 
	game_status = MAINMENU;
	DrawMainMenu();
      }
      else
	OpenDoor(DOOR_OPEN_1 | DOOR_COPY_BACK);
      break;
    case BUTTON_GAME_PAUSE:
      if (tape.pausing)
      {
	tape.pausing = FALSE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
      }
      else
      {
	tape.pausing = TRUE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_ON,0);
      }
      break;
    case BUTTON_GAME_PLAY:
      if (tape.pausing)
      {
	tape.pausing = FALSE;
	DrawVideoDisplay(VIDEO_STATE_PAUSE_OFF,0);
      }
      break;
    default:
      break;
  }
}
