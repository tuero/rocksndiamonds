// ============================================================================
// Artsoft Retro-Game Library
// ----------------------------------------------------------------------------
// (c) 1995-2014 by Artsoft Entertainment
//     		    Holger Schemel
//		    info@artsoft.org
//		    http://www.artsoft.org/
// ----------------------------------------------------------------------------
// image.c
// ============================================================================

#include "image.h"
#include "misc.h"
#include "setup.h"


struct ImageInfo
{
  char *source_filename;
  int num_references;

  Bitmap *bitmaps[NUM_IMG_BITMAP_POINTERS];

  int original_width;			/* original image file width */
  int original_height;			/* original image file height */

  boolean contains_small_images;	/* set after adding small images */
  boolean scaled_up;			/* set after scaling up */

  int game_tile_size;			/* size of in-game sized bitmap */
};
typedef struct ImageInfo ImageInfo;

static struct ArtworkListInfo *image_info = NULL;

static void *Load_Image(char *filename)
{
  ImageInfo *img_info = checked_calloc(sizeof(ImageInfo));

  if ((img_info->bitmaps[IMG_BITMAP_STANDARD] = LoadImage(filename)) == NULL)
  {
    Error(ERR_WARN, "cannot load image file '%s': LoadImage() failed: %s",
	  filename, GetError());

    free(img_info);

    return NULL;
  }

  img_info->source_filename = getStringCopy(filename);

  img_info->original_width  = img_info->bitmaps[IMG_BITMAP_STANDARD]->width;
  img_info->original_height = img_info->bitmaps[IMG_BITMAP_STANDARD]->height;

  img_info->contains_small_images = FALSE;
  img_info->scaled_up = FALSE;

  img_info->game_tile_size = 0;		// will be set later

  return img_info;
}

static void FreeImage(void *ptr)
{
  ImageInfo *image = (ImageInfo *)ptr;
  int i;

  if (image == NULL)
    return;

  for (i = 0; i < NUM_IMG_BITMAPS; i++)
    if (image->bitmaps[i])
      FreeBitmap(image->bitmaps[i]);

  if (image->source_filename)
    free(image->source_filename);

  free(image);
}

int getImageListSize()
{
  return (image_info->num_file_list_entries +
	  image_info->num_dynamic_file_list_entries);
}

struct FileInfo *getImageListEntryFromImageID(int pos)
{
  int num_list_entries = image_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);

  return (pos < num_list_entries ? &image_info->file_list[list_pos] :
	  &image_info->dynamic_file_list[list_pos]);
}

static ImageInfo *getImageInfoEntryFromImageID(int pos)
{
  int num_list_entries = image_info->num_file_list_entries;
  int list_pos = (pos < num_list_entries ? pos : pos - num_list_entries);
  ImageInfo **img_info =
    (ImageInfo **)(pos < num_list_entries ? image_info->artwork_list :
		   image_info->dynamic_artwork_list);

  return img_info[list_pos];
}

Bitmap **getBitmapsFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->bitmaps : NULL);
}

int getOriginalImageWidthFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->original_width : 0);
}

int getOriginalImageHeightFromImageID(int pos)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  return (img_info != NULL ? img_info->original_height : 0);
}

char *getTokenFromImageID(int graphic)
{
  struct FileInfo *file_list = getImageListEntryFromImageID(graphic);

  return (file_list != NULL ? file_list->token : NULL);
}

int getImageIDFromToken(char *token)
{
  struct FileInfo *file_list = image_info->file_list;
  int num_list_entries = image_info->num_file_list_entries;
  int i;

  for (i = 0; i < num_list_entries; i++)
    if (strEqual(file_list[i].token, token))
      return i;

  return -1;
}

char *getImageConfigFilename()
{
  return getCustomArtworkConfigFilename(image_info->type);
}

int getImageListPropertyMappingSize()
{
  return image_info->num_property_mapping_entries;
}

struct PropertyMapping *getImageListPropertyMapping()
{
  return image_info->property_mapping;
}

void InitImageList(struct ConfigInfo *config_list, int num_file_list_entries,
		   struct ConfigTypeInfo *config_suffix_list,
		   char **base_prefixes, char **ext1_suffixes,
		   char **ext2_suffixes, char **ext3_suffixes,
		   char **ignore_tokens)
{
  int i;

  image_info = checked_calloc(sizeof(struct ArtworkListInfo));
  image_info->type = ARTWORK_TYPE_GRAPHICS;

  /* ---------- initialize file list and suffix lists ---------- */

  image_info->num_file_list_entries = num_file_list_entries;
  image_info->num_dynamic_file_list_entries = 0;

  image_info->file_list =
    getFileListFromConfigList(config_list, config_suffix_list, ignore_tokens,
			      num_file_list_entries);
  image_info->dynamic_file_list = NULL;

  image_info->num_suffix_list_entries = 0;
  for (i = 0; config_suffix_list[i].token != NULL; i++)
    image_info->num_suffix_list_entries++;

  image_info->suffix_list = config_suffix_list;

  /* ---------- initialize base prefix and suffixes lists ---------- */

  image_info->num_base_prefixes = 0;
  for (i = 0; base_prefixes[i] != NULL; i++)
    image_info->num_base_prefixes++;

  image_info->num_ext1_suffixes = 0;
  for (i = 0; ext1_suffixes[i] != NULL; i++)
    image_info->num_ext1_suffixes++;

  image_info->num_ext2_suffixes = 0;
  for (i = 0; ext2_suffixes[i] != NULL; i++)
    image_info->num_ext2_suffixes++;

  image_info->num_ext3_suffixes = 0;
  for (i = 0; ext3_suffixes[i] != NULL; i++)
    image_info->num_ext3_suffixes++;

  image_info->num_ignore_tokens = 0;
  for (i = 0; ignore_tokens[i] != NULL; i++)
    image_info->num_ignore_tokens++;

  image_info->base_prefixes = base_prefixes;
  image_info->ext1_suffixes = ext1_suffixes;
  image_info->ext2_suffixes = ext2_suffixes;
  image_info->ext3_suffixes = ext3_suffixes;
  image_info->ignore_tokens = ignore_tokens;

  image_info->num_property_mapping_entries = 0;

  image_info->property_mapping = NULL;

  /* ---------- initialize artwork reference and content lists ---------- */

  image_info->sizeof_artwork_list_entry = sizeof(ImageInfo *);

  image_info->artwork_list =
    checked_calloc(num_file_list_entries * sizeof(ImageInfo *));
  image_info->dynamic_artwork_list = NULL;

  image_info->content_list = NULL;

  /* ---------- initialize artwork loading/freeing functions ---------- */

  image_info->load_artwork = Load_Image;
  image_info->free_artwork = FreeImage;
}

void ReloadCustomImages()
{
  print_timestamp_init("ReloadCustomImages");

  LoadArtworkConfig(image_info);
  print_timestamp_time("LoadArtworkConfig");

  ReloadCustomArtworkList(image_info);
  print_timestamp_time("ReloadCustomArtworkList");

  print_timestamp_done("ReloadCustomImages");
}

void CreateImageWithSmallImages(int pos, int zoom_factor, int tile_size)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  if (img_info == NULL)
    return;

  if (img_info->contains_small_images)
  {
    if (img_info->game_tile_size != gfx.game_tile_size)
      ReCreateGameTileSizeBitmap(img_info->bitmaps);

    img_info->game_tile_size = gfx.game_tile_size;

    return;
  }

  CreateBitmapWithSmallBitmaps(img_info->bitmaps, zoom_factor, tile_size);

  img_info->contains_small_images = TRUE;
  img_info->scaled_up = TRUE;			// scaling was also done here

  img_info->game_tile_size = gfx.game_tile_size;
}

void ScaleImage(int pos, int zoom_factor)
{
  ImageInfo *img_info = getImageInfoEntryFromImageID(pos);

  if (img_info == NULL || img_info->scaled_up)
    return;

  if (zoom_factor != 1)
    ScaleBitmap(img_info->bitmaps, zoom_factor);

  img_info->scaled_up = TRUE;
}

void FreeAllImages()
{
  FreeCustomArtworkLists(image_info);
}
