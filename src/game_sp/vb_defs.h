// ----------------------------------------------------------------------------
// vb_defs.h
// ----------------------------------------------------------------------------

#ifndef VB_DEFS_H
#define VB_DEFS_H

#define Win16			1

#ifndef False
#define False			0
#define True			(!False)
#endif

#ifndef NULL
#define NULL			((void *)0)
#endif

/* these are just dummy values to prevent the compiler from complaining */
#define VALUE_START_VB		100
#define VALUE_START_VB_KEY	200
#define VALUE_START_DD		300
#define VALUE_START_DD_ERR	400
#define VALUE_START_DS		500

#define vbPicTypeBitmap		(VALUE_START_VB + 1)
#define vbSrcCopy		(VALUE_START_VB + 2)
#define vbTwips			(VALUE_START_VB + 3)
#define vbPixels		(VALUE_START_VB + 4)
#define vbPRORPortrait		(VALUE_START_VB + 5)
#define vbPRORLandscape		(VALUE_START_VB + 6)
#define vbDirectory		(VALUE_START_VB + 7)

#define vbKeySpace		(VALUE_START_VB_KEY + 1)
#define vbKeyLeft		(VALUE_START_VB_KEY + 2)
#define vbKeyRight		(VALUE_START_VB_KEY + 3)
#define vbKeyUp			(VALUE_START_VB_KEY + 4)
#define vbKeyDown		(VALUE_START_VB_KEY + 5)
#define vbKeyEscape		(VALUE_START_VB_KEY + 6)
#define vbKeyR			(VALUE_START_VB_KEY + 7)
#define vbKeyReturn		(VALUE_START_VB_KEY + 8)
#define vbKeyShift		(VALUE_START_VB_KEY + 9)

#define DD_OK			(VALUE_START_DD + 0)

#define DDSD_CAPS		(VALUE_START_DD + 1)
#define DDSD_WIDTH		(VALUE_START_DD + 2)
#define DDSD_HEIGHT		(VALUE_START_DD + 3)
#define DDSCL_NORMAL		(VALUE_START_DD + 4)
#define DDBLT_WAIT		(VALUE_START_DD + 5)
#define DDSCAPS_VIDEOMEMORY	(VALUE_START_DD + 6)
#define DDSCAPS_OFFSCREENPLAIN	(VALUE_START_DD + 7)
#define DDSCAPS_PRIMARYSURFACE	(VALUE_START_DD + 8)

#define DDERR_GENERIC		(VALUE_START_DD_ERR + 1)
#define DDERR_INVALIDCLIPLIST	(VALUE_START_DD_ERR + 2)
#define DDERR_INVALIDOBJECT	(VALUE_START_DD_ERR + 3)
#define DDERR_INVALIDPARAMS	(VALUE_START_DD_ERR + 4)
#define DDERR_INVALIDRECT	(VALUE_START_DD_ERR + 5)
#define DDERR_NOALPHAHW		(VALUE_START_DD_ERR + 6)
#define DDERR_NOBLTHW		(VALUE_START_DD_ERR + 7)
#define DDERR_NOCLIPLIST	(VALUE_START_DD_ERR + 8)
#define DDERR_NODDROPSHW	(VALUE_START_DD_ERR + 9)
#define DDERR_NOMIRRORHW	(VALUE_START_DD_ERR + 10)
#define DDERR_NORASTEROPHW	(VALUE_START_DD_ERR + 11)
#define DDERR_NOROTATIONHW	(VALUE_START_DD_ERR + 12)
#define DDERR_NOSTRETCHHW	(VALUE_START_DD_ERR + 13)
#define DDERR_NOZBUFFERHW	(VALUE_START_DD_ERR + 14)
#define DDERR_SURFACEBUSY	(VALUE_START_DD_ERR + 15)
#define DDERR_SURFACELOST	(VALUE_START_DD_ERR + 16)
#define DDERR_UNSUPPORTED	(VALUE_START_DD_ERR + 17)
#define DDERR_WASSTILLDRAWING	(VALUE_START_DD_ERR + 18)

#define DSSCL_PRIORITY		(VALUE_START_DS + 1)
#define DSBCAPS_CTRLFREQUENCY	(VALUE_START_DS + 2)
#define DSBCAPS_CTRLPAN		(VALUE_START_DS + 3)
#define DSBCAPS_CTRLVOLUME	(VALUE_START_DS + 4)
#define DSBCAPS_STATIC		(VALUE_START_DS + 5)
#define WAVE_FORMAT_PCM		(VALUE_START_DS + 6)
#define DSBSTATUS_PLAYING	(VALUE_START_DS + 7)
#define DSBPLAY_DEFAULT		(VALUE_START_DS + 8)

#endif /* VB_DEFS_H */
