#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main()
{
  // Open file
  FILE* file = fopen("./assets/square.2x3.bmp", "r");
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

  for (size_t i = 0; i < result->header->height * result->header->width; i++)
  {
    printf("Pixel[%zu]\n", i);
    printf("\tR: %d\n", result->data[i].red);
    printf("\tG: %d\n", result->data[i].green);
    printf("\tB: %d\n", result->data[i].blue);
  }

  FILE *output = fopen("output.bmp", "w");
  bool res = write_bmp(output, result);
  
  printf("%d\n", write_bmp(file, result));
  printf("Size: %zu\n", sizeof(struct bmp_header));
  
  // free all the memory
  free_bmp_image(result);
  fclose(output);
  fclose(file);
}