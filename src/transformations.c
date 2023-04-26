#include "bmp.h"
#include "transformations.h"
#include <stdlib.h>
#include <memory.h>
#include <math.h>

static struct bmp_header *create_bmp_header_copy(const struct bmp_header *original, int new_height, int new_width)
{
  if (original == NULL)
  {
    return NULL;
  }

  // Allocate memory for header copy
  struct bmp_header *copy = malloc( sizeof(struct bmp_header) );
  if (copy == NULL)
  {
    return NULL;
  }
  memcpy( copy, original, sizeof(struct bmp_header) );

  // Set new width, height
  if (new_height != EOF) copy->height = new_height;
  if (new_width != EOF) copy->width = new_width;
  // printf("new height: %d\n", new_height);
  // printf("new height: %d\n", copy->height);
  
  const uint32_t new_image_size = (copy->width * copy->bpp + 31) / 32 * 4 * copy->height;
  if (new_image_size != copy->image_size)
  {
    copy->image_size = new_image_size;
    copy->size = copy->offset + copy->image_size;
    printf("Image size %d Size %d\n", copy->image_size, copy->size);
  }
  
  return copy;
}

static struct bmp_image *create_image_copy(const struct bmp_image *original, uint32_t new_height, uint32_t new_width)
{
  // Image is corrupted
  if (original == NULL || original->header == NULL || original->data == NULL || new_height <= 0 || new_width <= 0)
  {
    return NULL;
  }

  // Allocate memory for the copy image
  struct bmp_image *copy = malloc( sizeof(struct bmp_image) );
  if (copy == NULL)
  {
    return NULL;
  }

  // Set header to the copy image
  copy->header = create_bmp_header_copy(original->header, new_height, new_width);
  struct bmp_header *header = copy->header;
  if (header == NULL)
  {
    free(copy);
    return NULL;
  }

  // Allocate data segment
  copy->data = malloc( sizeof(struct pixel) * header->width * header->height );
  const struct pixel *data = copy->data;
  if (data == NULL)
  {
    free_bmp_image(copy);
    return NULL;
  }
  printf("Hi: %d\n", copy->header->size);
  return copy;
}

struct bmp_image* flip_horizontally(const struct bmp_image* image)
{
  // Get copy of the initial image
  struct bmp_image *flipped = create_image_copy(image, EOF, EOF);
  if (flipped == NULL)
  {
    return NULL;
  }
  const struct bmp_header *header = flipped->header;
  const struct pixel *data = flipped->data;

  // Set pixels to the flipped image
  for (uint32_t i = 0; i < header->height; i++)
  {
    for (uint32_t j = 0; j < header->width; j++)
    {
      memcpy( (void *) &data[i * header->width + header->width - j - 1], &image->data[i * header->width + j], sizeof(struct pixel) );
    }
  }
  
  return flipped;
}

struct bmp_image* flip_vertically(const struct bmp_image* image)
{
  // Get copy of the initial image
  struct bmp_image *flipped = create_image_copy(image, EOF, EOF);
  if (flipped == NULL)
  {
    return NULL;
  }
  const struct bmp_header *header = flipped->header;
  const struct pixel *data = flipped->data;

  // Set pixels to the flipped image
  for (uint32_t i = 0; i < header->height; i++)
  {
    for (uint32_t j = 0; j < header->width; j++)
    {
      memcpy( (void *) &data[(header->height - i - 1) * header->width + j], &image->data[i * header->width + j], sizeof(struct pixel) );
    }
  }
  
  return flipped;
}

struct bmp_image* rotate_left(const struct bmp_image* image)
{
  if (image == NULL || image->header == NULL || image->data == NULL)
  {
    return NULL;
  }

  // Get copy of the initial image
  struct bmp_image *rotated = create_image_copy(image, image->header->width, image->header->height);
  if (rotated == NULL)
  {
    return NULL;
  }
  
  // printf("HEIGHT: %d\n", image->header->height);
  // printf("WIDTH: %d\n", image->header->width);
  // Set pixels to the rotated image
  for (uint32_t i = 0; i < image->header->height; i++)
  {
    for (uint32_t j = 0; j < image->header->width; j++)
    {
      const uint32_t source_index = i * image->header->width + j;
      const uint32_t dest_index = j * image->header->height + image->header->height - 1 - i;
      // printf("i           : %d\n", i);
      // printf("j           : %d\n", j);
      // printf("Source_index: %d\n", source_index);
      // printf("Desttt_index: %d\n\n", dest_index);
      rotated->data[dest_index] = image->data[source_index];
    }
  }

  return rotated;
}

struct bmp_image* rotate_right(const struct bmp_image* image)
{
  if (image == NULL || image->header == NULL || image->data == NULL)
  {
    return NULL;
  }

  // Get copy of the initial image
  struct bmp_image *rotated = create_image_copy(image, image->header->width, image->header->height);
  if (rotated == NULL)
  {
    return NULL;
  }
  
  // Set pixels to the rotated image
  for (uint32_t i = 0; i < image->header->height; i++)
  {
    for (uint32_t j = 0; j < image->header->width; j++)
    {
      const uint32_t source_index = i * image->header->width + j;
      const uint32_t dest_index = image->header->height * (image->header->width -1 - j) + i;
      // printf("i           : %d\n", i);
      // printf("j           : %d\n", j);
      // printf("Source_index: %d\n", source_index);
      // printf("Desttt_index: %d\n\n", dest_index);
      rotated->data[dest_index] = image->data[source_index];
    }
  }

  return rotated;
}

struct bmp_image* crop(const struct bmp_image* image, const uint32_t start_y, const uint32_t start_x, const uint32_t height, const uint32_t width)
{
  if (image == NULL || image->header == NULL || image->data == NULL)
  {
    return NULL;
  }
  // printf("start_y: %d\n", start_y);
  // printf("start_x: %d\n", start_x);
  // printf("heihght: %d\n", height);
  // printf("widht: %d\n", width);
  const uint32_t width_global = image->header->width;
  const uint32_t height_global = image->header->height;

  if (start_x + width - 1 >= width_global || start_y + height - 1 >= height_global || height < 1 || width < 1)
  {
    return NULL;
  }

  struct bmp_image *result = create_image_copy(image, height, width);
  if (result == NULL)
  {
    return NULL;
  }
  // printf("new_Height: %d\n", result->header->height);
  // printf("new_width: %d\n", result->header->width);
  
  // struct pixel *data = result->data;
  const uint32_t y = height_global - 1 - start_y;

  // printf("x: %d\n", start_x);
  // printf("y: %d\n", y);

  for (uint32_t i = 0, yi = y; i < height; i++, yi++)
  {
    for (uint32_t j = 0, xi = start_x; j < width; j++, xi++)
    {
      if (xi >= width_global)
      {
        xi = 0;
      }
      if (yi >= height_global)
      {
        yi = 0;
      }
      const uint32_t dest_index = i * width + j;
      const uint32_t source_index = yi * width_global + xi;
      // printf("source_index: %d\n", source_index);
      // printf("dest_index  : %d\n\n", dest_index);

      result->data[dest_index] = image->data[source_index];
    }
    
  }
  
  return result; // IF IMAGE DID ALLOCATE MEMORY BUT NOT SET, WRITE FUNC DROPS
}


struct bmp_image* scale(const struct bmp_image* image, float factor)
{
  if (image == NULL || image->header == NULL || image->data == NULL || factor <= 0)
  {
    return NULL;
  }

  const uint32_t new_width = round(image->header->width * factor);
  const uint32_t new_height = round(image->header->height * factor);

  struct bmp_image *result = create_image_copy(image, new_height, new_width);
  if (result == NULL)
  {
    return NULL;
  }

  // printf("height: %d\n", result->header->height);
  // printf("width: %d\n", result->header->width);

  const size_t result_size = result->header->width * result->header->height;
  const size_t initial_size = image->header->width * image->header->height;
  
  // Set values to the result array
  for (size_t i = 0; i < result_size; i++)
  {
    const uint32_t index_in_original = initial_size * i / result_size;
    // printf("index: %d\n", index_in_original);
    result->data[i] = image->data[index_in_original];
  }

  return result;
}