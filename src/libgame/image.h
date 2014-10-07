// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// image.h
// ============================================================================

#ifndef IMAGE_H
#define IMAGE_H

#include "system.h"


// these bitmap pointers either point to allocated bitmaps or are NULL
#define IMG_BITMAP_32x32	0
#define IMG_BITMAP_16x16	1
#define IMG_BITMAP_8x8		2
#define IMG_BITMAP_4x4		3
#define IMG_BITMAP_2x2		4
#define IMG_BITMAP_1x1		5
#define IMG_BITMAP_CUSTOM	6

#define NUM_IMG_BITMAPS		7

// this bitmap pointer points to one of the above bitmaps (do not free it)
#define IMG_BITMAP_GAME		7

#define NUM_IMG_BITMAP_POINTERS	8

// this bitmap pointer points to the bitmap with default image size
#define IMG_BITMAP_STANDARD	IMG_BITMAP_32x32


int getImageListSize();
struct FileInfo *getImageListEntryFromImageID(int);
Bitmap **getBitmapsFromImageID(int);
int getOriginalImageWidthFromImageID(int);
int getOriginalImageHeightFromImageID(int);
char *getTokenFromImageID(int);
int getImageIDFromToken(char *);
char *getImageConfigFilename();
int getImageListPropertyMappingSize();
struct PropertyMapping *getImageListPropertyMapping();
void InitImageList(struct ConfigInfo *, int, struct ConfigTypeInfo *,
		   char **, char **, char **, char **, char **);

void ReloadCustomImages();
void CreateImageWithSmallImages(int, int, int);
void ScaleImage(int, int);

void FreeAllImages();

#endif	/* IMAGE_H */
