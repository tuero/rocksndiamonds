/* 2000-08-10T18:03:54Z
 *
 * open X11 display and sound
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/xpm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "../libgame/platform.h"

#include "global.h"
#include "display.h"
#include "sample.h"

Display *display;
Window xwindow;

Pixmap screenPixmap;
Pixmap scorePixmap;
Pixmap spriteBitmap;

Pixmap objPixmap;
Pixmap objmaskBitmap;
Pixmap botPixmap;
Pixmap botmaskBitmap;
Pixmap sprPixmap;
Pixmap sprmaskBitmap;
Pixmap ttlPixmap;
Pixmap ttlmaskBitmap;

GC screenGC;
GC scoreGC;
GC spriteGC;
GC antsGC;

Atom deleteAtom;

KeySym lastKeySym;

KeyCode northKeyCode[3];
KeyCode eastKeyCode[3];
KeyCode southKeyCode[3];
KeyCode westKeyCode[3];
KeyCode fireKeyCode[3];
KeyCode escKeyCode[1];

char play[SAMPLE_MAX];

static int sound_pid = -1;
int sound_pipe[2] = { -1, -1 }; /* for communication */
short *sound_data[SAMPLE_MAX]; /* pointer to sound data */
long sound_length[SAMPLE_MAX]; /* length of sound data */

static Screen *defaultScreen;
static Visual *defaultVisual;
static Colormap defaultColourmap;
static Window defaultRootWindow;
static unsigned int screenDepth;
static unsigned int screenWidth;
static unsigned int screenHeight;
static unsigned long screenBlackPixel;
static unsigned long screenWhitePixel;

static XSizeHints sizeHints;
static XSetWindowAttributes setWindowAttributes;
static XWMHints wmHints;
static XVisualInfo visualInfo;
static XGCValues gcValues;

static Colormap privateColourmap;
static Cursor cursor;
static XColor *privateColours;
static unsigned char *privateFlags;
static int privateNumColours;

static XColor redColour;
static XColor whiteColour;
static int gotRed;
static int gotWhite;

static Pixmap xpmPixmaps[4];
static Pixmap xpmBitmaps[4];
static XpmAttributes xpmAttributes[4];
static int xpmGot[4];

static int xpmAllocColourFunc(Display *, Colormap, char *, XColor *, void *);
static int xpmFreeColoursFunc(Display *, Colormap, unsigned long *, int, void *);

static KeyCode keycodes[16];

static const char *xpmNames[4] = { "object.xpm", "score.xpm", "sprite.xpm", "title.xpm" };
static const int xpmCloseness[4] = { 10000, 10000, 40000, 50000 };
static const KeySym keysyms[16] = {
	XK_Up, XK_KP_Up, XK_r, /* north */
	XK_Right, XK_KP_Right, XK_g, /* east */
	XK_Down, XK_KP_Down, XK_f, /* south */
	XK_Left, XK_KP_Left, XK_d, /* west */
	XK_Shift_L, XK_Control_R, XK_space, /* fire */
	XK_Escape /* escape */
};
static const char *sound_names[SAMPLE_MAX] = {
	"00.blank.au","01.roll.au","02.stone.au","03.nut.au","04.crack.au",
	"05.bug.au","06.tank.au","07.android.au","08.spring.au","09.slurp.au",
	"10.eater.au","11.alien.au","12.collect.au","13.diamond.au","14.squash.au",
	"15.drip.au","16.push.au","17.dirt.au","18.acid.au","19.ball.au",
	"20.grow.au","21.wonder.au","22.door.au","23.exit.au","24.dynamite.au",
	"25.tick.au","26.press.au","27.wheel.au","28.boom.au","29.time.au",
	"30.die.au"
};
static const int sound_volume[SAMPLE_MAX] = {
	20,100,100,100,100,20,20,100,100,100,
	50,100,100,100,100,100,100,100,100,100,
	100,20,100,100,100,100,100,20,100,100,
	100
};

int open_all(void)
{
	char name[MAXNAME+2];
	void *dummyptr;
	int dummyint;
	int i;

	display = XOpenDisplay(arg_display);
	if(display == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to open display", strerror(errno));
		return(1);
	}

	defaultScreen = DefaultScreenOfDisplay(display);
	defaultVisual = DefaultVisualOfScreen(defaultScreen);
	defaultColourmap = DefaultColormapOfScreen(defaultScreen);
	defaultRootWindow = RootWindowOfScreen(defaultScreen);
	screenDepth = DefaultDepthOfScreen(defaultScreen);
	screenWidth = WidthOfScreen(defaultScreen);
	screenHeight = HeightOfScreen(defaultScreen);
	screenBlackPixel = BlackPixelOfScreen(defaultScreen);
	screenWhitePixel = WhitePixelOfScreen(defaultScreen);

	if(arg_install) {
		visualInfo.visualid = XVisualIDFromVisual(defaultVisual);
		dummyptr = XGetVisualInfo(display, VisualIDMask, &visualInfo, &dummyint);
		if(dummyptr == 0) {
			fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to get visual info", strerror(errno));
			return(1);
		}
		memcpy(&visualInfo, dummyptr, sizeof(visualInfo));
		XFree(dummyptr);

		if(visualInfo.class != PseudoColor) {
			fprintf(stderr, "%s: \"%s\": %s\n", progname, XDisplayName(arg_display), "private colourmap only supported for pseudocolour display");
			return(1);
		}

		privateColourmap = XCreateColormap(display, defaultRootWindow, defaultVisual, AllocAll);
		if(privateColourmap == 0) {
			fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create colourmap", strerror(errno));
			return(1);
		}

		privateNumColours = visualInfo.colormap_size;

		privateColours = malloc(privateNumColours * sizeof(XColor));
		if(privateColours == 0) {
			fprintf(stderr, "%s: %s (%d): %s\n", progname, "malloc failed", privateNumColours * sizeof(XColor), strerror(errno));
			return(1);
		}
		for(dummyint = 0; dummyint < privateNumColours; dummyint++) privateColours[dummyint].pixel = dummyint;
		XQueryColors(display, defaultColourmap, privateColours, privateNumColours);
		XStoreColors(display, privateColourmap, privateColours, privateNumColours);

		privateFlags = malloc(privateNumColours);
		if(privateFlags == 0) {
			fprintf(stderr, "%s: %s (%d): %s\n", progname, "malloc failed", privateNumColours, strerror(errno));
			return(1);
		}
		memset(privateFlags, 0, privateNumColours);
		privateFlags[0] = 1; /* first two entries (black and white) are already allocated */
		privateFlags[1] = 1;
	}

	sizeHints.flags = PSize | PMinSize | PMaxSize;
	sizeHints.width = 20 * TILEX;
	sizeHints.height = 12 * TILEY + SCOREY;
	sizeHints.min_width = sizeHints.max_width = sizeHints.width;
	sizeHints.min_height = sizeHints.max_height = sizeHints.height;
	if(arg_geometry) {
		dummyint = XWMGeometry(display, XScreenNumberOfScreen(defaultScreen), arg_geometry, 0, 2, &sizeHints, &sizeHints.x, &sizeHints.y, &dummyint, &dummyint, &sizeHints.win_gravity);
		if(dummyint & (XValue | YValue)) sizeHints.flags |= USPosition | PWinGravity;
	}

	xwindow = XCreateWindow(display, defaultRootWindow, sizeHints.x, sizeHints.y, sizeHints.width, sizeHints.height, 2, screenDepth, InputOutput, CopyFromParent, 0, 0);
	if(xwindow == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to open window", strerror(errno));
		return(1);
	}

	setWindowAttributes.background_pixel = screenBlackPixel;
	setWindowAttributes.border_pixel = screenWhitePixel;
	setWindowAttributes.backing_store = NotUseful;
	setWindowAttributes.override_redirect = False;
	setWindowAttributes.event_mask = KeyPressMask | EnterWindowMask | LeaveWindowMask | ExposureMask;
	setWindowAttributes.colormap = privateColourmap ? privateColourmap : defaultColourmap;
	XChangeWindowAttributes(display, xwindow, CWBackPixel | CWBorderPixel | CWBackingStore | CWOverrideRedirect | CWEventMask | CWColormap, &setWindowAttributes);

	XStoreName(display, xwindow, "Emerald Mine");

	wmHints.flags = InputHint | StateHint;
	wmHints.input = True;
	wmHints.initial_state = NormalState;
	XSetWMHints(display, xwindow, &wmHints);

	XSetWMNormalHints(display, xwindow, &sizeHints);

	deleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, xwindow, &deleteAtom, 1);

	cursor = XCreateFontCursor(display, XC_trek);
	if(cursor) XDefineCursor(display, xwindow, cursor);

	XMapWindow(display, xwindow);

	for(i = 0; i < 4; i++) {
		name[MAXNAME] = 0;
		if(arg_basedir) {
			snprintf(name, MAXNAME+2, "%s/%s/%s", arg_basedir, EM_GFX_DIR, xpmNames[i]);
		} else {
			snprintf(name, MAXNAME+2, "%s/%s", EM_GFX_DIR, xpmNames[i]);
		}
		if(name[MAXNAME]) snprintf_overflow("read graphics/ files");

		xpmAttributes[i].valuemask = XpmColormap | XpmReturnAllocPixels | XpmExactColors | XpmCloseness | XpmAllocColor | XpmFreeColors;
		xpmAttributes[i].colormap = privateColourmap ? privateColourmap : defaultColourmap;
		xpmAttributes[i].exactColors = False;
		xpmAttributes[i].closeness = xpmCloseness[i];
		xpmAttributes[i].alloc_color = xpmAllocColourFunc;
		xpmAttributes[i].free_colors = xpmFreeColoursFunc;
		dummyint = XpmReadFileToPixmap(display, xwindow, name, &xpmPixmaps[i], &xpmBitmaps[i], &xpmAttributes[i]);
		if(dummyint) {
			fprintf(stderr, "%s: \"%s\": \"%s\": %s: %s: %s\n", progname, XDisplayName(arg_display), name, "failed to read xpm", XpmGetErrorString(dummyint), strerror(errno));
			return(1);
		}
		xpmGot[i] = 1;
	}

	objPixmap = xpmPixmaps[0];
	botPixmap = xpmPixmaps[1];
	sprPixmap = xpmPixmaps[2];
	ttlPixmap = xpmPixmaps[3];
	objmaskBitmap = xpmBitmaps[0];
	botmaskBitmap = xpmBitmaps[1];
	sprmaskBitmap = xpmBitmaps[2];
	ttlmaskBitmap = xpmBitmaps[3];

	screenPixmap = XCreatePixmap(display, xwindow, 22 * TILEX, 14 * TILEY, screenDepth);
	if(screenPixmap == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create pixmap", strerror(errno));
		return(1);
	}

	scorePixmap = XCreatePixmap(display, xwindow, 20 * TILEX, SCOREY, screenDepth);
	if(scorePixmap == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create pixmap", strerror(errno));
		return(1);
	}

	spriteBitmap = XCreatePixmap(display, xwindow, TILEX, TILEY, 1);
	if(spriteBitmap == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create pixmap", strerror(errno));
		return(1);
	}

	redColour.pixel = screenWhitePixel;
	whiteColour.pixel = screenBlackPixel;
	gotRed = (xpmAllocColourFunc(display, privateColourmap ? privateColourmap : defaultColourmap, "red", &redColour, 0) > 0);
	gotWhite = (xpmAllocColourFunc(display, privateColourmap ? privateColourmap : defaultColourmap, "white", &whiteColour, 0) > 0);

	gcValues.graphics_exposures = False;
	screenGC = XCreateGC(display, screenPixmap, GCGraphicsExposures, &gcValues);
	if(screenGC == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create graphics context", strerror(errno));
		return(1);
	}

	gcValues.graphics_exposures = False;
	scoreGC = XCreateGC(display, scorePixmap, GCGraphicsExposures, &gcValues);
	if(scoreGC == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create graphics context", strerror(errno));
		return(1);
	}

	gcValues.function = objmaskBitmap ? GXcopyInverted : sprmaskBitmap ? GXcopy : GXset;
	gcValues.graphics_exposures = False;
	spriteGC = XCreateGC(display, spriteBitmap, GCFunction | GCGraphicsExposures, &gcValues);
	if(spriteGC == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create graphics context", strerror(errno));
		return(1);
	}

	gcValues.foreground = redColour.pixel;
	gcValues.background = whiteColour.pixel;
	gcValues.line_style = LineDoubleDash;
	gcValues.graphics_exposures = False;
	antsGC = XCreateGC(display, screenPixmap, GCForeground | GCBackground | GCLineStyle | GCGraphicsExposures, &gcValues);
	if(antsGC == 0) {
		fprintf(stderr, "%s: \"%s\": %s: %s\n", progname, XDisplayName(arg_display), "failed to create graphics context", strerror(errno));
		return(1);
	}

	for(i = 0; i < 16; i++) {
		keycodes[i] = XKeysymToKeycode(display, keysyms[i]);
	}
	for(i = 0; i < 3; i++) northKeyCode[i] = keycodes[i + 0];
	for(i = 0; i < 3; i++) eastKeyCode[i] = keycodes[i + 3];
	for(i = 0; i < 3; i++) southKeyCode[i] = keycodes[i + 6];
	for(i = 0; i < 3; i++) westKeyCode[i] = keycodes[i + 9];
	for(i = 0; i < 3; i++) fireKeyCode[i] = keycodes[i + 12];
	for(i = 0; i < 1; i++) escKeyCode[i] = keycodes[i + 15];

#if defined(PLATFORM_LINUX) || defined(PLATFORM_BSD)
	if(arg_silence == 0) {
		for(i = 0; i < SAMPLE_MAX; i++) {
			name[MAXNAME] = 0;
			if(arg_basedir) {
				snprintf(name, MAXNAME+2, "%s/%s/%s", arg_basedir, EM_SND_DIR, sound_names[i]);
			} else {
				snprintf(name, MAXNAME+2, "%s/%s", EM_SND_DIR, sound_names[i]);
			}
			if(name[MAXNAME]) snprintf_overflow("read sounds/ directory");

			if(read_sample(name, &sound_data[i], &sound_length[i])) return(1);

			{
				short *ptr, *stop;
				int mult = sound_volume[i] * 65536 / (100 * MIXER_MAX);
				stop = sound_data[i] + sound_length[i];
				for(ptr = sound_data[i]; ptr < stop; ptr++) *ptr = (*ptr * mult) / 65536;
			}
		}

		if(pipe(sound_pipe) == -1) {
			fprintf(stderr, "%s: %s: %s\n", progname, "unable to create sound pipe", strerror(errno));
			return(1);
		}
		sound_pid = fork();
		if(sound_pid == -1) {
			fprintf(stderr, "%s: %s: %s\n", progname, "unable to fork sound thread", strerror(errno));
			return(1);
		}
		close(sound_pipe[sound_pid == 0]); sound_pipe[sound_pid == 0] = -1;
		if(sound_pid == 0) _exit(sound_thread());
		signal(SIGPIPE, SIG_IGN); /* dont crash if sound process dies */
	}
#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_BSD) */

	return(0);
}

void close_all(void)
{
	int i;

	if(sound_pid != -1) {
		kill(sound_pid, SIGTERM);
		waitpid(sound_pid, 0, 0);
	}
	if(sound_pipe[0] != -1) close(sound_pipe[0]);
	if(sound_pipe[1] != -1) close(sound_pipe[1]);
	for(i = 0; i < SAMPLE_MAX; i++) if(sound_data[i]) free(sound_data[i]);

	for(i = 0; i < 4; i++) if(xpmPixmaps[i]) XFreePixmap(display, xpmPixmaps[i]);
	for(i = 0; i < 4; i++) if(xpmBitmaps[i]) XFreePixmap(display, xpmBitmaps[i]);
	for(i = 0; i < 4; i++) if(xpmGot[i]) {
		xpmFreeColoursFunc(display, xpmAttributes[i].colormap, xpmAttributes[i].alloc_pixels, xpmAttributes[i].nalloc_pixels, 0);
		XpmFreeAttributes(&xpmAttributes[i]);
	}
	if(gotRed) xpmFreeColoursFunc(display, privateColourmap ? privateColourmap : defaultColourmap, &redColour.pixel, 1, 0);
	if(gotWhite) xpmFreeColoursFunc(display, privateColourmap ? privateColourmap : defaultColourmap, &whiteColour.pixel, 1, 0);
	if(screenGC) XFreeGC(display, screenGC);
	if(scoreGC) XFreeGC(display, scoreGC);
	if(spriteGC) XFreeGC(display, spriteGC);
	if(antsGC) XFreeGC(display, antsGC);
	if(screenPixmap) XFreePixmap(display, screenPixmap);
	if(scorePixmap) XFreePixmap(display, scorePixmap);
	if(spriteBitmap) XFreePixmap(display, spriteBitmap);
	if(xwindow) XDestroyWindow(display, xwindow);
	if(cursor) XFreeCursor(display, cursor);
	if(privateColourmap) XFreeColormap(display, privateColourmap);
	if(privateColours) free(privateColours);
	if(privateFlags) free(privateFlags);
	if(display) XCloseDisplay(display);
}

/* ---------------------------------------------------------------------- */

void sound_play(void)
{
	if(sound_pipe[1] != -1) {
		if(write(sound_pipe[1], &play, sizeof(play)) == -1) {
			fprintf(stderr, "%s: %s: %s\n", progname, "write sound", strerror(errno));
			if(sound_pipe[0] != -1) { close(sound_pipe[0]); sound_pipe[0] = -1; }
			if(sound_pipe[1] != -1) { close(sound_pipe[1]); sound_pipe[1] = -1; }
		}
	}
	memset(play, 0, sizeof(play));
}

/* ---------------------------------------------------------------------- */

static int xpmAllocColourFunc(Display *display, Colormap colourmap, char *colourname, XColor *xcolour, void *closure)
{
	int i, match;
	int r,g,b;
	long best, sum;

	if(colourname) if(XParseColor(display, colourmap, colourname, xcolour) == 0) return(-1); /* invalid name */
	if(colourmap != privateColourmap) return(XAllocColor(display, colourmap, xcolour) != 0);

/* first try to find an exact match */
	match = -1;
	for(i = 0; i < privateNumColours; i++) {
		if(privateColours[i].red == xcolour->red && privateColours[i].green == xcolour->green && privateColours[i].blue == xcolour->blue) match = i;
	}
	if(match != -1) {
		privateFlags[match] = 1;
		xcolour->pixel = privateColours[match].pixel;
		return(1);
	}

/* then find an unallocated colour that is close to what we want */
	match = -1;
	best = 1000000;
	for(i = 0; i < privateNumColours; i++) {
		if(privateFlags[i]) continue; /* skip if it is already allocated */
		r = (privateColours[i].red - xcolour->red) / 256;
		g = (privateColours[i].green - xcolour->green) / 256;
		b = (privateColours[i].blue - xcolour->blue) / 256;
		sum = r * r + g * g + b * b;
		if(sum < best) {
			best = sum;
			match = i;
		}
	}
	if(match != -1) {
		privateFlags[match] = 1;
		privateColours[match].red = xcolour->red;
		privateColours[match].green = xcolour->green;
		privateColours[match].blue = xcolour->blue;
		XStoreColor(display, colourmap, &privateColours[match]);
		xcolour->pixel = privateColours[match].pixel;
		return(1); /* found a close match */
	}

/* if all else fails, just find the closest colour and return it */
	match = -1;
	best = 1000000;
	for(i = 0; i < privateNumColours; i++) {
		r = (privateColours[i].red - xcolour->red) / 256;
		g = (privateColours[i].green - xcolour->green) / 256;
		b = (privateColours[i].blue - xcolour->blue) / 256;
		sum = r * r + g * g + b * b;
		if(sum < best) {
			best = sum;
			match = i;
		}
	}
	if(match != -1) {
		xcolour->red = privateColours[match].red;
		xcolour->green = privateColours[match].green;
		xcolour->blue = privateColours[match].blue;
		xcolour->pixel = privateColours[match].pixel;
		return(1); /* best we could do */
	}
	return(0); /* still didnt find one, give up */
}

static int xpmFreeColoursFunc(Display *display, Colormap colourmap, unsigned long *pixels, int npixels, void *closure)
{
	if(colourmap != privateColourmap) XFreeColors(display, colourmap, pixels, npixels, 0);
	return(1); /* non-zero for success */
}
