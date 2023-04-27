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
  
  const uint32_t new_image_size = (copy->width * copy->height * 3) + copy->height * (copy->width % 4);

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

/**
 * Remove unwanted outer area from image.
 *
 * Creates copy of image containing only selected rectangular area.
 *
 * @arg image the image
 * @arg start_y top-left corner position on y-axis of selected area in the range <0, image->height>
 * @arg start_x top-left corner position on x-axis of selected area in the range <0, image->width>
 * @arg height the height of selected area in pixels in the range <1, image->height>
 * @arg width the width of selected area in pixels in the range <1, image->width>
 * @return the copy of image containing only selected area or null, if there is no image (NULL given) or area position is out of range
 */
struct bmp_image* crop(const struct bmp_image* image, const uint32_t start_y, const uint32_t start_x, const uint32_t height, const uint32_t width)
{
  if (image == NULL || image->header == NULL || image->data == NULL)
  {
    return NULL;
  }
  printf("start_y: %d\n", start_y);
  printf("start_x: %d\n", start_x);
  printf("heihght: %d\n", height);
  printf("widht: %d\n", width);
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

   for (uint32_t i = 0; i < height; i++)
  {
    const uint32_t source_row_index = y - i;
    const uint32_t dest_row_index = height - i - 1;
    const struct pixel *source_row_start = image->data + (source_row_index * width_global) + start_x;
    struct pixel *dest_row_start = result->data + (dest_row_index * width);
    memcpy(dest_row_start, source_row_start, sizeof(struct pixel) * width);
  }

  /*
  for (uint32_t i = 0, yi = y; i < height; i++, yi--)
  {
    for (uint32_t j = 0, xi = start_x; j < width; j++, xi++)
    {
      if (xi >= width_global)
      {
        xi = 0;
      }
      const uint32_t dest_index = i * width + j;
      const uint32_t source_index = yi * width_global + xi;
      // printf("source_index: %d\n", source_index);
      // printf("dest_index  : %d\n\n", dest_index);

      result->data[dest_index] = image->data[source_index];
    }
  }
  */
  
  return result; // IF IMAGE DID ALLOCATE MEMORY BUT NOT SET, WRITE FUNC DROPS
}


struct bmp_image* scale(const struct bmp_image* image, float factor)
{
  if (image == NULL || image->header == NULL || image->data == NULL || factor < 0)
  {
    return NULL;
  }

  const uint32_t new_width = round(image->header->width * factor);
  const uint32_t new_height = round(image->header->height * factor);

  if (new_width == 0 || new_height == 0)
  {
    return NULL;
  }

  struct bmp_image *result = create_image_copy(image, new_height, new_width);
  if (result == NULL)
  {
    return NULL;
  }

  printf("height: %d\n", result->header->height);
  printf("width: %d\n", result->header->width);

  const size_t result_size = result->header->width * result->header->height;
  const size_t initial_size = image->header->width * image->header->height;
  
  printf("Result_size: %zu\n", result_size);
  printf("Initial size: %zu\n", initial_size);

  // if (result_size > initial_size)
  // {
  //   for (uint32_t i = 0; i < result->header->height; i++)
  //   {
  //     for (uint32_t j = 0; j < result->header->width; j++)
  //     {
  //       const uint32_t x_initial = image->header->width * j / result->header->width;
  //       const uint32_t y_initial = image->header->height * i / result->header->height;

  //       printf("x_init: %d\n", x_initial);
  //       printf("y_init: %d\n", y_initial);
  //       printf("source_pos: %d\n\n", y_initial * image->header->width + x_initial);
  //     }
      
  //   }
    
  // }

  for (int j = 0, old_j = 0; j < result->header->width; j++, old_j = j * image->header->width / result->header->width)
  {
    for (int i = 0, old_i = 0; i < result->header->height; i++, old_i = i * image->header->height / result->header->height)
    {
      printf("source index: %d\n", image->header->width * old_i + old_j);
      printf("dest index: %d\n\n", result->header->width * i + j);
      result->data[result->header->width * i + j] = image->data[image->header->width * old_i + old_j];
    }
  }
  // free_bmp_image(result);
  return result;
}


struct bmp_image* extract(const struct bmp_image* image, const char* colors_to_keep)
{
  if (image == NULL || image->header == NULL || image->data == NULL || colors_to_keep == NULL)
  {
    return NULL;
  }

  bool red_keep = false, green_keep = false, blue_keep = false;

  uint8_t index = 0;
  while (colors_to_keep[index] != '\0')
  {
    switch (colors_to_keep[index])
    {
      case 'r':
        if (red_keep)
        {
          return NULL;
        }
        red_keep = true;
        break;
      
      case 'g':
        if (green_keep) return NULL;
        green_keep = true;
        break;
      
      case 'b':
        if (blue_keep) return NULL;
        blue_keep = true;
        break;
      
      default:
        return NULL;
    }
    index++;
  }

  // Get copy of the initial image
  struct bmp_image *result_image = create_image_copy(image, image->header->height, image->header->width);
  if (result_image == NULL)
  {
    return NULL;
  }

  for (uint32_t i = 0; i < image->header->height; i++)
  {
    for (uint32_t j = 0; j < image->header->width; j++)
    {
      const uint32_t index = i * image->header->width + j;
      result_image->data[index] = image->data[index];
      if (!red_keep)
      {
        result_image->data[index].red = 0;
      }
      if (!green_keep)
      {
        result_image->data[index].green = 0;
      }
      if (!blue_keep)
      {
        result_image->data[index].blue = 0;
      }
    }
  }
  
  return result_image;
}