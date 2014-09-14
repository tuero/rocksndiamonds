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


int getImageListSize();
struct FileInfo *getImageListEntryFromImageID(int);
Bitmap *getBitmapFromImageID(int);
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
