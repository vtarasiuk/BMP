#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "transformations.h"

int main()
{
  // Open file
  FILE* file = fopen("./assets/cherry.bmp", "rb");
  if (file == NULL)
  {
    printf("Can not open file\n");
    return 1;
  }

  // Read header of BMP
  struct bmp_image *result = read_bmp(file);
  if (result == NULL)
  {
    fclose(file);
    return 2;
  }

  printf("Type: %c%c\n", result->header->type & 0xFF, result->header->type >> 8);
  printf("size: %d\n", result->header->size);
  printf("reserved1: %d\n", result->header->reserved1);
  printf("reserved2: %d\n", result->header->reserved2);
  printf("offset: %d\n", result->header->offset);
  printf("dib_size: %d\n", result->header->dib_size);
  printf("width: %d\n", result->header->width);
  printf("height: %d\n", result->header->height);
  printf("planes: %d\n", result->header->planes);
  printf("bpp: %d\n", result->header->bpp);
  printf("compression: %d\n", result->header->compression);
  printf("image_size: %d\n", result->header->image_size);
  printf("x_ppm: %d\n", result->header->x_ppm);
  printf("y_ppm: %d\n", result->header->y_ppm);
  printf("num_colors: %d\n", result->header->num_colors);
  printf("important_colors: %d\n", result->header->important_colors);



  // struct bmp_image* rotated_image = rotate_right(result);
  // FILE *out = fopen("right.bmp", "wb");
  // printf("Write: %d\n", write_bmp(out, rotated_image));
  // free_bmp_image(rotated_image);
  // fclose(out);

  // struct bmp_image* rotated_left_image = rotate_left(result);
  // FILE *out2 = fopen("left.bmp", "wb");
  // printf("Write: %d\n", write_bmp(out2, rotated_left_image));
  // free_bmp_image(rotated_left_image);
  // fclose(out2);

  struct bmp_image* crop_image = crop(result, 0, 100, 192, 156);
  FILE *out3 = fopen("crop.bmp", "wb");
  printf("Write: %d\n", write_bmp(out3, crop_image));
  free_bmp_image(crop_image);
  fclose(out3);

  struct bmp_image* resize_image = scale(result, 0.5);
  FILE *out4 = fopen("resize.bmp", "wb");
  printf("Write: %d\n", write_bmp(out4, resize_image));
  free_bmp_image(resize_image);
  fclose(out4);

  free_bmp_image(result);
  fclose(file);
}