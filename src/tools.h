/***********************************************************
*  Rocks'n'Diamonds -- McDuffin Strikes Back!              *
*----------------------------------------------------------*
*  (c) 1995-98 Artsoft Entertainment                       *
*              Holger Schemel                              *
*              Oststrasse 11a                              *
*              33604 Bielefeld                             *
*              phone: ++49 +521 290471                     *
*              email: aeglos@valinor.owl.de                *
*----------------------------------------------------------*
*  tools.h                                                 *
***********************************************************/

#ifndef TOOLS_H
#define TOOLS_H

#include <sys/time.h>
#include "main.h"

/* for SetDrawtoField */
#define DRAW_DIRECT		0
#define DRAW_BUFFERED		1
#define DRAW_BACKBUFFER		2

/* for DrawElementShifted */
#define NO_CUTTING		0
#define CUT_ABOVE		(1 << 0)
#define CUT_BELOW		(1 << 1)
#define CUT_LEFT		(1 << 2)
#define CUT_RIGHT		(1 << 3)

/* for masking functions */
#define NO_MASKING		0
#define USE_MASKING		1
 
/* for MoveDoor */
#define DOOR_OPEN_1		(1 << 0)
#define DOOR_OPEN_2		(1 << 1)
#define DOOR_CLOSE_1		(1 << 2)
#define DOOR_CLOSE_2		(1 << 3)
#define DOOR_OPEN_ALL		(DOOR_OPEN_1 | DOOR_OPEN_2)
#define DOOR_CLOSE_ALL		(DOOR_CLOSE_1 | DOOR_CLOSE_2)
#define DOOR_ACTION_1		(DOOR_OPEN_1 | DOOR_CLOSE_1)
#define DOOR_ACTION_2		(DOOR_OPEN_2 | DOOR_CLOSE_2)
#define DOOR_ACTION		(DOOR_ACTION_1 | DOOR_ACTION_2)
#define DOOR_COPY_BACK		(1 << 4)
#define DOOR_NO_DELAY		(1 << 5)
#define DOOR_GET_STATE		(1 << 6)

/* for Request */
#define REQ_ASK			(1 << 0)
#define REQ_OPEN		(1 << 1)
#define REQ_CLOSE		(1 << 2)
#define REQ_CONFIRM		(1 << 3)
#define REQ_STAY_CLOSED		(1 << 4)
#define REQ_STAY_OPEN		(1 << 5)
#define REQ_PLAYER		(1 << 6)

#define REQUEST_WAIT_FOR	(REQ_ASK | REQ_CONFIRM | REQ_PLAYER)

void SetDrawtoField(int);
void BackToFront();
void FadeToFront();
void ClearWindow();
int getFontWidth(int, int);
int getFontHeight(int, int);
void DrawInitText(char *, int, int);
void DrawTextF(int, int, int, char *, ...);
void DrawTextFCentered(int, int, char *, ...);
void DrawText(int, int, char *, int, int);
void DrawTextExt(Drawable, GC, int, int, char *, int, int);
void DrawAllPlayers(void);
void DrawPlayerField(int, int);
void DrawPlayer(struct PlayerInfo *);
void DrawGraphicAnimationExt(int, int, int, int, int, int, int);
void DrawGraphicAnimation(int, int, int, int, int, int);
void DrawGraphicAnimationThruMask(int, int, int, int, int, int);
void getGraphicSource(int, int *, int *, int *);
void DrawGraphic(int, int, int);
void DrawGraphicExt(Drawable, GC, int, int, int);
void DrawGraphicThruMask(int, int, int);
void DrawGraphicThruMaskExt(Drawable, int, int, int);
void DrawMiniGraphic(int, int, int);
void getMiniGraphicSource(int, Pixmap *, int *, int *);
void DrawMiniGraphicExt(Drawable, GC, int, int, int);
void DrawGraphicShifted(int, int, int, int, int, int, int);
void DrawGraphicShiftedThruMask(int, int, int, int, int, int);
void DrawScreenElementExt(int, int, int, int, int, int, int);
void DrawLevelElementExt(int, int, int, int, int, int, int);
void DrawScreenElementShifted(int, int, int, int, int, int);
void DrawLevelElementShifted(int, int, int, int, int, int);
void DrawScreenElementThruMask(int, int, int);
void DrawLevelElementThruMask(int, int, int);
void DrawLevelFieldThruMask(int, int);
void ErdreichAnbroeckeln(int, int);
void DrawScreenElement(int, int, int);
void DrawLevelElement(int, int, int);
void DrawScreenField(int, int);
void DrawLevelField(int, int);
void DrawMiniElement(int, int, int);
void DrawMiniElementOrWall(int, int, int, int);
void DrawMicroElement(int, int, int);
void DrawLevel(void);
void DrawMiniLevel(int, int, int, int);
void DrawMicroLevel(int, int, boolean);
boolean Request(char *, unsigned int);
unsigned int OpenDoor(unsigned int);
unsigned int CloseDoor(unsigned int);
unsigned int GetDoorState(void);
unsigned int MoveDoor(unsigned int);
void DrawSpecialEditorDoor();
void UndrawSpecialEditorDoor();
int ReadPixel(Drawable, int, int);

void CreateToolButtons();

int el2gfx(int);

#endif
