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
*  init.c                                                  *
***********************************************************/

#include "init.h"
#include "misc.h"
#include "sound.h"
#include "screens.h"
#include "files.h"
#include <signal.h>

static int sound_process_id = 0;

void OpenAll(int argc, char *argv[])
{
  InitLevelAndPlayerInfo();

  InitCounter();
  InitSound();
  InitSoundProcess();
  InitJoystick();
  InitRND(NEW_RANDOMIZE);

  signal(SIGINT, CloseAll);
  signal(SIGTERM, CloseAll);

  InitDisplay(argc, argv);
  InitWindow(argc, argv);
  InitGfx();
  InitElementProperties();

  DrawMainMenu();

  XMapWindow(display, window);
  XFlush(display);
}

void InitLevelAndPlayerInfo()
{
  if (!LoadLevelInfo())			/* global level info */
    CloseAll();

  LoadPlayerInfo(PLAYER_SETUP);		/* global setup info */
  LoadPlayerInfo(PLAYER_LEVEL);		/* level specific info */
}

void InitSound()
{
  int i;

  if (sound_status==SOUND_OFF)
    return;

  if (access(sound_device_name,W_OK)<0)
  {
    fprintf(stderr,"%s: cannot access sound device - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }

  if ((sound_device=open(sound_device_name,O_WRONLY))<0)
  {
    fprintf(stderr,"%s: cannot open sound device - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }

  close(sound_device);
  sound_status=SOUND_AVAILABLE;

#ifdef VOXWARE
  sound_loops_allowed = TRUE;
  sound_loops_on = TRUE;
#endif

  for(i=0;i<NUM_SOUNDS;i++)
  {
    Sound[i].name = sound_name[i];
    if (!LoadSound(&Sound[i]))
    {
      sound_status=SOUND_OFF;
      return;
    }
  }
}

void InitSoundProcess()
{
  if (sound_status==SOUND_OFF)
    return;

  if (pipe(sound_pipe)<0)
  {
    fprintf(stderr,"%s: cannot create pipe - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }

  if ((sound_process_id=fork())<0)
  {       
    fprintf(stderr,"%s: cannot create child process - no sounds\n",progname);
    sound_status=SOUND_OFF;
    return;
  }

  if (!sound_process_id)	/* we are child */
    SoundServer();
  else				/* we are parent */
    close(sound_pipe[0]);	/* no reading from pipe needed */
}

void InitJoystick()
{
  if (global_joystick_status==JOYSTICK_OFF)
    return;

  if (access(joystick_device_name[joystick_nr],R_OK)<0)
  {
    fprintf(stderr,"%s: cannot access joystick device '%s'\n",
	    progname,joystick_device_name[joystick_nr]);
    joystick_status = JOYSTICK_OFF;
    return;
  }

  if ((joystick_device=open(joystick_device_name[joystick_nr],O_RDONLY))<0)
  {
    fprintf(stderr,"%s: cannot open joystick device '%s'\n",
	    progname,joystick_device_name[joystick_nr]);
    joystick_status = JOYSTICK_OFF;
    return;
  }

  joystick_status = JOYSTICK_AVAILABLE;
  LoadJoystickData();
}

void InitDisplay(int argc, char *argv[])
{
  char *display_name = NULL;
  int i;

  /* get X server to connect to, if given as an argument */
  for (i=1;i<argc-1;i++)
  {
    char *dispstr="-display";
    int len=MAX(strlen(dispstr),strlen(argv[i]));

    if (len<4)
      continue;
    else if (!strncmp(argv[i],dispstr,len))
    {
      display_name=argv[i+1];
      break;
    }
  }

  /* connect to X server */
  if (!(display=XOpenDisplay(display_name)))
  {
    fprintf(stderr,"%s: cannot connect to X server %s\n", 
	    progname, XDisplayName(display_name));
    exit(-1);
  }
  
  screen = DefaultScreen(display);
  cmap   = DefaultColormap(display, screen);
  pen_fg = WhitePixel(display,screen);
  pen_bg = BlackPixel(display,screen);
}

void InitWindow(int argc, char *argv[])
{
  unsigned int border_width = 4;
  Pixmap icon_pixmap, iconmask_pixmap;
  unsigned int icon_width,icon_height;
  int icon_hot_x,icon_hot_y;
  char icon_filename[256];
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  XTextProperty windowName, iconName;
  XGCValues gc_values;
  unsigned long gc_valuemask;
  char *window_name = "Rocks'n'Diamonds";
  char *icon_name = "Rocks'n'Diamonds";
  long window_event_mask;
  static struct PictureFile icon_pic =
  {
    "rocks_icon.xbm",
    "rocks_iconmask.xbm"
  };

  width = WIN_XSIZE;
  height = WIN_YSIZE;

  window = XCreateSimpleWindow(display, RootWindow(display, screen),
			    WIN_XPOS, WIN_YPOS, width, height, border_width,
			    pen_fg, pen_bg);

  sprintf(icon_filename,"%s/%s",GFX_PATH,icon_pic.picture_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &icon_pixmap,&icon_hot_x,&icon_hot_y);
  if (!icon_pixmap)
  {
    fprintf(stderr, "%s: cannot read icon bitmap file '%s'.\n",
	    progname,icon_filename);
    exit(-1);
  }

  sprintf(icon_filename,"%s/%s",GFX_PATH,icon_pic.picturemask_filename);
  XReadBitmapFile(display,window,icon_filename,
		  &icon_width,&icon_height,
		  &iconmask_pixmap,&icon_hot_x,&icon_hot_y);
  if (!iconmask_pixmap)
  {
    fprintf(stderr, "%s: cannot read icon bitmap file '%s'.\n",
	    progname,icon_filename);
    exit(-1);
  }

  size_hints.flags = PSize | PMinSize | PMaxSize;
  size_hints.width  = size_hints.min_width  = size_hints.max_width  = width;
  size_hints.height = size_hints.min_height = size_hints.max_height = height;

  if (!XStringListToTextProperty(&window_name, 1, &windowName))
  {
    fprintf(stderr, "%s: structure allocation for windowName failed.\n",
	    progname);
    exit(-1);
  }

  if (!XStringListToTextProperty(&icon_name, 1, &iconName))
  {
    fprintf(stderr, "%s: structure allocation for iconName failed.\n",
	    progname);
    exit(-1);
  }

  wm_hints.initial_state = NormalState;
  wm_hints.input = True;
  wm_hints.icon_pixmap = icon_pixmap;
  wm_hints.icon_mask = iconmask_pixmap;
  wm_hints.flags = StateHint | IconPixmapHint | IconMaskHint | InputHint;

  class_hints.res_name = progname;
  class_hints.res_class = "Rocks'n'Diamonds";

  XSetWMProperties(display, window, &windowName, &iconName, 
		   argv, argc, &size_hints, &wm_hints, 
		   &class_hints);

  XFree(windowName.value);
  XFree(iconName.value);

  /* Select event types wanted */
  window_event_mask = ExposureMask | StructureNotifyMask | FocusChangeMask |
                      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                      KeyPressMask | KeyReleaseMask;
  XSelectInput(display, window, window_event_mask);

  /* create GC for drawing with window depth */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  gc = XCreateGC(display, window, gc_valuemask, &gc_values);
}

void InitGfx()
{
  int i,j,x,y;
  int xpm_err, xbm_err;
  unsigned int width,height;
  int hot_x,hot_y;
  XGCValues gc_values;
  unsigned long gc_valuemask;
  XGCValues clip_gc_values;
  unsigned long clip_gc_valuemask;
  char filename[256];
  Pixmap shapemask;

  static struct PictureFile pic[NUM_PICTURES] =
  {
    "RocksScreen.xpm",		"RocksScreenMaske.xbm",
    "RocksDoor.xpm",		"RocksDoorMaske.xbm",
    "RocksToons.xpm",		"RocksToonsMaske.xbm",
    "RocksFont.xpm",		NULL,
    "RocksFont2.xpm",		NULL
  }; 

  for(i=0;i<NUM_PICTURES;i++)
  {
    if (pic[i].picture_filename)
    {
      sprintf(filename,"%s/%s",GFX_PATH,pic[i].picture_filename);

      xpm_att[i].valuemask = XpmCloseness;
      xpm_att[i].closeness = 20000;
      xpm_err = XpmReadFileToPixmap(display,window,filename,
				    &pix[i],&shapemask,&xpm_att[i]);
      switch(xpm_err)
      {
	case XpmOpenFailed:
          fprintf(stderr,"Xpm file open failed on '%s' !\n",filename);
	  CloseAll();
	  exit(-1);
	case XpmFileInvalid:
	  fprintf(stderr,"Invalid Xpm file '%s'!\n",filename);
	  CloseAll();
	  exit(-1);
	case XpmNoMemory:
	  fprintf(stderr,"Not enough memory !\n");	
	  CloseAll();
	  exit(1);
	case XpmColorFailed:
	  fprintf(stderr,"Can`t get any colors...\n");
	  CloseAll();
	  exit(-1);
	default:
	  break;
      }
      if (!pix[i])
      {
	fprintf(stderr, "%s: cannot read Xpm file '%s'.\n",
		progname,filename);
	CloseAll();
	exit(-1);
      }
    }

    if (pic[i].picturemask_filename)
    {
      sprintf(filename,"%s/%s",GFX_PATH,pic[i].picturemask_filename);

      xbm_err = XReadBitmapFile(display,window,filename,
				&width,&height,&clipmask[i],&hot_x,&hot_y);
      switch(xbm_err)
      {
	case BitmapSuccess:
          break;
	case BitmapOpenFailed:
	  fprintf(stderr,"Bitmap file open failed on '%s' !\n",filename);
	  CloseAll();
	  exit(-1);
	  break;
	case BitmapFileInvalid:
	  fprintf(stderr,"Bitmap file invalid: '%s' !\n",filename);
	  CloseAll();
	  exit(-1);
	  break;
	case BitmapNoMemory:
	  fprintf(stderr,"No memory for file '%s' !\n",filename);
	  CloseAll();
	  exit(-1);
	  break;
	default:
	  break;
      }
      if (!clipmask[i])
      {
	fprintf(stderr, "%s: cannot read X11 bitmap file '%s'.\n",
		progname,filename);
	CloseAll();
	exit(-1);
      }
    }
  }

  pix[PIX_DB_BACK] = XCreatePixmap(display, window,
				   WIN_XSIZE,WIN_YSIZE,
				   XDefaultDepth(display,screen));
  pix[PIX_DB_DOOR] = XCreatePixmap(display, window,
				   3*DXSIZE,DYSIZE+VYSIZE,
				   XDefaultDepth(display,screen));

  clipmask[PIX_FADEMASK] = XCreatePixmap(display, window,
					 SXSIZE+TILEX,SYSIZE+TILEY,1);

  if (!pix[PIX_DB_BACK] || !pix[PIX_DB_DOOR] || !clipmask[PIX_FADEMASK])
  {
    fprintf(stderr, "%s: cannot create additional Pixmaps!\n",progname);
    CloseAll();
    exit(-1);
  }

  /* create GC for drawing with bitplane depth */
  gc_values.graphics_exposures = False;
  gc_values.foreground = pen_bg;
  gc_values.background = pen_bg;
  gc_valuemask = GCGraphicsExposures | GCForeground | GCBackground;
  plane_gc = XCreateGC(display, clipmask[PIX_BACK], gc_valuemask, &gc_values);

  for(y=0;y<=SCR_FIELDY;y++) for(x=0;x<=SCR_FIELDX;x++)
    XCopyArea(display,clipmask[PIX_BACK],clipmask[PIX_FADEMASK],plane_gc,
	      SX+2*TILEX,SY+10*TILEY,TILEX,TILEY,x*TILEX,y*TILEY);

  for(i=0;i<NUM_PIXMAPS;i++)
  {
    if (clipmask[i])
    {
      clip_gc_values.graphics_exposures = False;
      clip_gc_values.foreground = pen_fg;
      clip_gc_values.background = pen_bg;
      clip_gc_values.clip_mask = clipmask[i];
      clip_gc_valuemask =
	GCGraphicsExposures | GCForeground | GCBackground | GCClipMask;
      clip_gc[i] = XCreateGC(display,window,clip_gc_valuemask,&clip_gc_values);
    }
  }

  drawto = drawto_field = backbuffer = pix[PIX_DB_BACK];

  XCopyArea(display,pix[PIX_BACK],backbuffer,gc,
	    0,0, WIN_XSIZE,WIN_YSIZE, 0,0);
  XFillRectangle(display,backbuffer,gc,
		 REAL_SX,REAL_SY, FULL_SXSIZE,FULL_SYSIZE);

  for(i=0;i<SCR_FIELDX;i++)
    for(j=0;j<SCR_FIELDY;j++)
      redraw[i][j]=0;
  redraw_tiles=0;
  redraw_mask=REDRAW_ALL;
}

void InitElementProperties()
{
  int i,j;

  static int ep_amoebalive[] =
  {
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL
  };
  static int ep_amoebalive_num = sizeof(ep_amoebalive)/sizeof(int);

  static int ep_amoeboid[] =
  {
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL
  };
  static int ep_amoeboid_num = sizeof(ep_amoeboid)/sizeof(int);

  static int ep_badewannoid[] =
  {
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5
  };
  static int ep_badewannoid_num = sizeof(ep_badewannoid)/sizeof(int);

  static int ep_schluessel[] =
  {
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4
  };
  static int ep_schluessel_num = sizeof(ep_schluessel)/sizeof(int);

  static int ep_pforte[] =
  {
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X
  };
  static int ep_pforte_num = sizeof(ep_pforte)/sizeof(int);

  static int ep_solid[] =
  {
    EL_BETON,
    EL_MAUERWERK,
    EL_FELSBODEN,
    EL_AUSGANG_ZU,
    EL_AUSGANG_ACT,
    EL_AUSGANG_AUF,
    EL_AMOEBE_TOT,
    EL_AMOEBE_NASS,
    EL_AMOEBE_NORM,
    EL_AMOEBE_VOLL,
    EL_MORAST_VOLL,
    EL_MORAST_LEER,
    EL_SIEB_VOLL,
    EL_SIEB_LEER,
    EL_LIFE,
    EL_LIFE_ASYNC,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5
  };
  static int ep_solid_num = sizeof(ep_solid)/sizeof(int);

  static int ep_massiv[] =
  {
    EL_BETON,
    EL_SALZSAEURE,
    EL_BADEWANNE1,
    EL_BADEWANNE2,
    EL_BADEWANNE3,
    EL_BADEWANNE4,
    EL_BADEWANNE5,
    EL_PFORTE1,
    EL_PFORTE2,
    EL_PFORTE3,
    EL_PFORTE4,
    EL_PFORTE1X,
    EL_PFORTE2X,
    EL_PFORTE3X,
    EL_PFORTE4X
  };
  static int ep_massiv_num = sizeof(ep_massiv)/sizeof(int);

  static int ep_slippery[] =
  {
    EL_FELSBODEN,
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_ABLENK_EIN,
    EL_ABLENK_AUS,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER,
    EL_BIRNE_EIN,
    EL_BIRNE_AUS,
    EL_BADEWANNE1,
    EL_BADEWANNE2
  };
  static int ep_slippery_num = sizeof(ep_slippery)/sizeof(int);

  static int ep_enemy[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_MAMPFER,
    EL_ZOMBIE,
    EL_PACMAN
  };
  static int ep_enemy_num = sizeof(ep_enemy)/sizeof(int);

  static int ep_can_fall[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_DIAMANT,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_MORAST_VOLL,
    EL_SIEB_VOLL,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER
  };
  static int ep_can_fall_num = sizeof(ep_can_fall)/sizeof(int);

  static int ep_can_smash[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_DIAMANT,
    EL_SCHLUESSEL1,
    EL_SCHLUESSEL2,
    EL_SCHLUESSEL3,
    EL_SCHLUESSEL4,
    EL_BOMBE,
    EL_KOKOSNUSS,
    EL_TROPFEN,
    EL_ZEIT_VOLL,
    EL_ZEIT_LEER
  };
  static int ep_can_smash_num = sizeof(ep_can_smash)/sizeof(int);

  static int ep_can_change[] =
  {
    EL_FELSBROCKEN,
    EL_EDELSTEIN,
    EL_DIAMANT
  };
  static int ep_can_change_num = sizeof(ep_can_change)/sizeof(int);

  static int ep_can_move[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_MAMPFER,
    EL_ZOMBIE,
    EL_PACMAN
  };
  static int ep_can_move_num = sizeof(ep_can_move)/sizeof(int);

  static int ep_could_move[] =
  {
    EL_KAEFER_R,
    EL_KAEFER_O,
    EL_KAEFER_L,
    EL_KAEFER_U,
    EL_FLIEGER_R,
    EL_FLIEGER_O,
    EL_FLIEGER_L,
    EL_FLIEGER_U,
    EL_PACMAN_R,
    EL_PACMAN_O,
    EL_PACMAN_L,
    EL_PACMAN_U
  };
  static int ep_could_move_num = sizeof(ep_could_move)/sizeof(int);

  static int ep_dont_touch[] =
  {
    EL_KAEFER,
    EL_FLIEGER
  };
  static int ep_dont_touch_num = sizeof(ep_dont_touch)/sizeof(int);

  static int ep_dont_go_to[] =
  {
    EL_KAEFER,
    EL_FLIEGER,
    EL_MAMPFER,
    EL_ZOMBIE,
    EL_PACMAN,
    EL_TROPFEN,
    EL_SALZSAEURE
  };
  static int ep_dont_go_to_num = sizeof(ep_dont_go_to)/sizeof(int);

  static long ep_bit[] =
  {
    EP_BIT_AMOEBALIVE,
    EP_BIT_AMOEBOID,
    EP_BIT_BADEWANNOID,
    EP_BIT_SCHLUESSEL,
    EP_BIT_PFORTE,
    EP_BIT_SOLID,
    EP_BIT_MASSIV,
    EP_BIT_SLIPPERY,
    EP_BIT_ENEMY,
    EP_BIT_CAN_FALL,
    EP_BIT_CAN_SMASH,
    EP_BIT_CAN_CHANGE,
    EP_BIT_CAN_MOVE,
    EP_BIT_COULD_MOVE,
    EP_BIT_DONT_TOUCH,
    EP_BIT_DONT_GO_TO
  };
  static int *ep_array[] =
  {
    ep_amoebalive,
    ep_amoeboid,
    ep_badewannoid,
    ep_schluessel,
    ep_pforte,
    ep_solid,
    ep_massiv,
    ep_slippery,
    ep_enemy,
    ep_can_fall,
    ep_can_smash,
    ep_can_change,
    ep_can_move,
    ep_could_move,
    ep_dont_touch,
    ep_dont_go_to
  };
  static int *ep_num[] =
  {
    &ep_amoebalive_num,
    &ep_amoeboid_num,
    &ep_badewannoid_num,
    &ep_schluessel_num,
    &ep_pforte_num,
    &ep_solid_num,
    &ep_massiv_num,
    &ep_slippery_num,
    &ep_enemy_num,
    &ep_can_fall_num,
    &ep_can_smash_num,
    &ep_can_change_num,
    &ep_can_move_num,
    &ep_could_move_num,
    &ep_dont_touch_num,
    &ep_dont_go_to_num
  };
  static int num_properties = sizeof(ep_num)/sizeof(int *);

  for(i=0;i<MAX_ELEMENTS;i++)
    Elementeigenschaften[i] = 0;

  for(i=0;i<num_properties;i++)
    for(j=0;j<*(ep_num[i]);j++)
      Elementeigenschaften[(ep_array[i])[j]] |= ep_bit[i];
  for(i=EL_CHAR_START;i<EL_CHAR_END;i++)
    Elementeigenschaften[i] |= EP_BIT_CHAR;
}

void CloseAll()
{
  int i;

  if (sound_process_id)
  {
    StopSounds();
    kill(sound_process_id, SIGTERM);
    FreeSounds(NUM_SOUNDS);
  }

  for(i=0;i<NUM_PIXMAPS;i++)
  {
    if (pix[i])
    {
      if (i<NUM_PICTURES)	/* XPM pictures */
      {
	XFreeColors(display,DefaultColormap(display,screen),
		    xpm_att[i].pixels,xpm_att[i].npixels,0);
	XpmFreeAttributes(&xpm_att[i]);
      }
      XFreePixmap(display,pix[i]);
    }
    if (clipmask[i])
      XFreePixmap(display,clipmask[i]);
    if (clip_gc[i])
      XFreeGC(display, clip_gc[i]);
  }

  if (gc)
    XFreeGC(display, gc);
  if (plane_gc)
    XFreeGC(display, plane_gc);

  XCloseDisplay(display);

  exit(0);
}
