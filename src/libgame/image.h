/***********************************************************
* Artsoft Retro-Game Library                               *
*----------------------------------------------------------*
* (c) 1994-2002 Artsoft Entertainment                      *
*               Holger Schemel                             *
*               Detmolder Strasse 189                      *
*               33604 Bielefeld                            *
*               Germany                                    *
*               e-mail: info@artsoft.org                   *
*----------------------------------------------------------*
* image.h                                                  *
***********************************************************/

#ifndef IMAGE_H
#define IMAGE_H

#include "system.h"


#if defined(TARGET_X11)

#define MAX_COLORS	256	/* maximal number of colors for each image */

typedef unsigned short Intensity;	/* RGB intensity for X11 */

typedef struct
{
  Display  *display;		/* destination display             */
  int       depth;		/* depth of destination drawable   */
  Pixel     index[MAX_COLORS];	/* array of pixel values           */
  int       no;			/* number of pixels in the array   */
  Colormap  cmap;		/* colormap used for image         */
  Pixmap   pixmap;		/* final pixmap                    */
  Pixmap   pixmap_mask;		/* final pixmap of mask            */
} XImageInfo;

struct RGBMap
{
  unsigned int used;			/* number of colors used in RGB map */
  Intensity    red[MAX_COLORS];		/* color values in X style          */
  Intensity    green[MAX_COLORS];
  Intensity    blue[MAX_COLORS];
  boolean      color_used[MAX_COLORS];	/* flag if color cell is used       */
};

typedef struct
{
  unsigned int  type;		/* type of image (True-Color etc.)     */
  struct RGBMap rgb;		/* RGB map of image if IRGB type       */
  unsigned int  width;		/* width of image in pixels            */
  unsigned int  height;		/* height of image in pixels           */
  unsigned int  depth;		/* depth of image in bits if IRGB type */
  unsigned int  bytes_per_pixel;/* (depth + 7) / 8                     */
  unsigned int  bytes_per_row;	/* width * bytes_per_pixel             */
  byte         *data;		/* image data                          */
} Image;

#define IMAGETYPE_BITMAP	0	/* monochrome bitmap       */
#define IMAGETYPE_RGB		1	/* RGB image with colormap */
#define IMAGETYPE_TRUECOLOR	2	/* true-color image        */

#define TRUECOLOR_RED(pixel)	(((unsigned long)((pixel) & 0xff0000)) >> 16)
#define TRUECOLOR_GREEN(pixel)	(((unsigned long)((pixel) & 0xff00)) >> 8)
#define TRUECOLOR_BLUE(pixel)	( (unsigned long)((pixel) & 0xff))
#define RGB_TO_TRUECOLOR(r,g,b)	((((unsigned long)((r) & 0xff00)) << 8) | ((g) & 0xff00) | (((unsigned short)(b)) >> 8))

Image *newImage(unsigned int, unsigned int, unsigned int);
void freeImage(Image *);
void freeXImage(Image *, XImageInfo *);

Pixmap Pixmap_to_Mask(Pixmap, int, int);

void ZoomPixmap(Display *, GC, Pixmap, Pixmap, int, int, int, int);

int Read_PCX_to_Pixmap(Display *, Window, GC, char *, Pixmap *, Pixmap *);

#endif /* TARGET_X11 */

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
void CreateImageWithSmallImages(int, int);
void ScaleImage(int, int);

void FreeAllImages();

#endif	/* IMAGE_H */
