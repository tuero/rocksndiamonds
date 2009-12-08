// ----------------------------------------------------------------------------
// vb_lib.h
// ----------------------------------------------------------------------------

#ifndef VB_LIB_H
#define VB_LIB_H

#define Abs(x)	ABS(x)
#define Sqr(x)	sqrt(x)


/* helper functions for constructs not supported by C */
extern void *REDIM_1D(int, int, int);
extern void *REDIM_2D(int, int, int, int, int);

extern boolean IS_NOTHING(void *, int);

extern void SET_TO_NOTHING(void *, int);

extern void MESSAGE_BOX(char *);

extern char *CAT(const char *, ...);
extern char *GET_PATH(char *, ...);
extern char *INT_TO_STR(int);

extern boolean STRING_IS_LIKE(char *, char *);

extern void FILE_GET(FILE *, int, void *, int);
extern int FILE_PUT(FILE *, int, void *, int);

/* this is just a workaround -- handle array definitions later */
extern void *Array(int, ...);

/* VB functions that do not return "int" (and would cause compiler errors) */
extern double Val(char *);
extern char *Left(char *, int);
extern char *left(char *, int);
extern char *Right(char *, int);
extern char *right(char *, int);
extern char *StrReverse(char *);
extern int InStr(int, char *, char *);
extern char *Dir(char *);
extern char *Dir_Without_Args();
extern void Kill(char *);
extern char *Chr(int);
extern char *String(int, char *);
extern void MkDir(char *);
extern char *SlashLess(char *);
extern char *Hex(int);

extern int FileLen(char *);

extern long MyGetTickCount();

extern int GetAttr(char *);

extern void DoEvents();

extern void SaveSetting(const char *, const char *, char *, int);

#if 0
extern long GetTempPath(long, char *);
#endif

#endif /* VB_LIB_H */
