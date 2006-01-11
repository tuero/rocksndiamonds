/***********************************************************
* Rocks'n'Diamonds -- McDuffin Strikes Back!               *
*----------------------------------------------------------*
* (c) 1995-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* tape.h                                                   *
***********************************************************/

#ifndef TAPE_H
#define TAPE_H

#include "main.h"


/* values for TapeTogglePause() */
#define	TAPE_TOGGLE_MANUAL	TRUE
#define	TAPE_TOGGLE_AUTOMATIC	FALSE

/* some positions in the video tape control window */
#define VIDEO_DISPLAY1_XPOS	5
#define VIDEO_DISPLAY1_YPOS	5
#define VIDEO_DISPLAY2_XPOS	5
#define VIDEO_DISPLAY2_YPOS	41
#define VIDEO_DISPLAY_XSIZE	90
#define VIDEO_DISPLAY_YSIZE	31
#define VIDEO_BUTTON_XSIZE	18
#define VIDEO_BUTTON_YSIZE	18
#define VIDEO_CONTROL_XPOS	5
#define VIDEO_CONTROL_YPOS	77
#define VIDEO_CONTROL_XSIZE	VIDEO_DISPLAY_XSIZE
#define VIDEO_CONTROL_YSIZE	VIDEO_BUTTON_YSIZE

/* values for video tape control */
#define VIDEO_STATE_PLAY_OFF	(1 << 0)
#define VIDEO_STATE_PLAY_ON	(1 << 1)
#define VIDEO_STATE_PLAY	(VIDEO_STATE_PLAY_OFF	| VIDEO_STATE_PLAY_ON)
#define VIDEO_STATE_REC_OFF	(1 << 2)
#define VIDEO_STATE_REC_ON	(1 << 3)
#define VIDEO_STATE_REC		(VIDEO_STATE_REC_OFF	| VIDEO_STATE_REC_ON)
#define VIDEO_STATE_PAUSE_OFF	(1 << 4)
#define VIDEO_STATE_PAUSE_ON	(1 << 5)
#define VIDEO_STATE_PAUSE	(VIDEO_STATE_PAUSE_OFF	| VIDEO_STATE_PAUSE_ON)
#define VIDEO_STATE_DATE_OFF	(1 << 6)
#define VIDEO_STATE_DATE_ON	(1 << 7)
#define VIDEO_STATE_DATE	(VIDEO_STATE_DATE_OFF	| VIDEO_STATE_DATE_ON)
#define VIDEO_STATE_TIME_OFF	(1 << 8)
#define VIDEO_STATE_TIME_ON	(1 << 9)
#define VIDEO_STATE_TIME	(VIDEO_STATE_TIME_OFF	| VIDEO_STATE_TIME_ON)

#define VIDEO_STATE_FFWD_OFF	(1 << 10)
#define VIDEO_STATE_FFWD_ON	(1 << 11)
#define VIDEO_STATE_FFWD	(VIDEO_STATE_FFWD_OFF	| VIDEO_STATE_FFWD_ON)
#define VIDEO_STATE_PBEND_OFF	(1 << 12)
#define VIDEO_STATE_PBEND_ON	(1 << 13)
#define VIDEO_STATE_PBEND	(VIDEO_STATE_PBEND_OFF	| VIDEO_STATE_PBEND_ON)
#define VIDEO_STATE_WARP_OFF	(1 << 14)
#define VIDEO_STATE_WARP_ON	(1 << 15)
#define VIDEO_STATE_WARP	(VIDEO_STATE_WARP_OFF	| VIDEO_STATE_WARP_ON)
#define VIDEO_STATE_WARP2_OFF	(1 << 16)
#define VIDEO_STATE_WARP2_ON	(1 << 17)
#define VIDEO_STATE_WARP2	(VIDEO_STATE_WARP_OFF	| VIDEO_STATE_WARP_ON)
#define VIDEO_STATE_1STEP_OFF	(1 << 18)
#define VIDEO_STATE_1STEP_ON	(1 << 19)
#define VIDEO_STATE_1STEP	(VIDEO_STATE_1STEP_OFF	| VIDEO_STATE_1STEP_ON)

#define VIDEO_PRESS_PLAY_ON	(1 << 20)
#define VIDEO_PRESS_PLAY_OFF	(1 << 21)
#define VIDEO_PRESS_PLAY	(VIDEO_PRESS_PLAY_OFF	| VIDEO_PRESS_PLAY_ON)
#define VIDEO_PRESS_REC_ON	(1 << 22)
#define VIDEO_PRESS_REC_OFF	(1 << 23)
#define VIDEO_PRESS_REC		(VIDEO_PRESS_REC_OFF	| VIDEO_PRESS_REC_ON)
#define VIDEO_PRESS_PAUSE_ON	(1 << 24)
#define VIDEO_PRESS_PAUSE_OFF	(1 << 25)
#define VIDEO_PRESS_PAUSE	(VIDEO_PRESS_PAUSE_OFF	| VIDEO_PRESS_PAUSE_ON)
#define VIDEO_PRESS_STOP_ON	(1 << 26)
#define VIDEO_PRESS_STOP_OFF	(1 << 27)
#define VIDEO_PRESS_STOP	(VIDEO_PRESS_STOP_OFF	| VIDEO_PRESS_STOP_ON)
#define VIDEO_PRESS_EJECT_ON	(1 << 28)
#define VIDEO_PRESS_EJECT_OFF	(1 << 29)
#define VIDEO_PRESS_EJECT	(VIDEO_PRESS_EJECT_OFF	| VIDEO_PRESS_EJECT_ON)

/* tags to draw video display labels or symbols only */
/* (negative values to prevent misinterpretation in DrawVideoDisplay(), where
   the variable "value" is also used for tape length -- better fix this) */
#define VIDEO_DISPLAY_DEFAULT		0
#define VIDEO_DISPLAY_LABEL_ONLY	-1
#define VIDEO_DISPLAY_SYMBOL_ONLY	-2


void DrawVideoDisplay(unsigned long, unsigned long);
void DrawCompleteVideoDisplay(void);

void TapeDeactivateDisplayOn();
void TapeDeactivateDisplayOff(boolean);

void TapeStartRecording(void);
void TapeHaltRecording(void);
void TapeStopRecording(void);
void TapeRecordAction(byte *);
void TapeTogglePause(boolean);
void TapeStartPlaying(void);
void TapeStopPlaying(void);
byte *TapePlayAction(void);
void TapeStop(void);
void TapeErase(void);
unsigned int GetTapeLength(void);
void TapeQuickSave(void);
void TapeQuickLoad(void);
void InsertSolutionTape(void);

void AutoPlayTape(void);

void CreateTapeButtons();
void FreeTapeButtons();
void MapTapeEjectButton();
void MapTapeWarpButton();
void MapTapeButtons();
void UnmapTapeButtons();

#endif
