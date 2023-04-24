#include "bmp.h"
#include <stdlib.h>

struct bmp_header* read_bmp_header(FILE* stream) // TODO: corrupted stream | NOT a bmp image
{
  // Stream is NULL
  if (stream == NULL)
  {
    return NULL;
  }
  
  // Check if a stream position is valid
  const long current_pos = ftell(stream);
  if (current_pos == -1L || current_pos == EOF)
  {
    return NULL;
  }
  // Set position index to zero
  if (current_pos != 0) fseek(stream, 0, SEEK_SET);
  // Allocate memory for header
  struct bmp_header *header = malloc( sizeof(struct bmp_header) );
  if (header == NULL)
  {
    return NULL;
  }
  
  // Read BMP header
  if (fread(header, sizeof(struct bmp_header), 1, stream) != 1)
  {
    free(header);
    return NULL;
  }

  return header;
}

struct pixel* read_data(FILE* stream, const struct bmp_header* header)
{
  // stream or header is NULL
  if (stream == NULL || header == NULL)
  {
    return NULL;
  }
  // Allocating mamory for pixels of the bmp file
  struct pixel* pixels = malloc( sizeof(struct pixel) * header->height * header->width );
  if (pixels == NULL)
  {
    return NULL;
  }

  for (uint32_t i = 0; i < header->height; i++)
  {
    // Read one row at each iteration
    if (fread(&pixels[i * header->width], sizeof(struct pixel), header->width, stream) != header->width)
    {
      free(pixels);
      return NULL;
    }
    
    // Determine and add padding
    const uint8_t padding = header->width % 4;
    fseek(stream, padding, SEEK_CUR);
  }

  return pixels;
}

struct bmp_image* read_bmp(FILE* stream)
{
  // Stream is NULL
  if (stream == NULL)
  {
    fprintf(stderr, "Error: stream is NULL\n");
    return NULL;
  }

  // Allocate memory for the image
  struct bmp_image *image = malloc( sizeof(struct bmp_image) );
  if (image == NULL)
  {
    fprintf(stderr, "Error: can't allocate memory\n");
    return NULL;
  }

  // Get header from a BMP file
  image->header = read_bmp_header(stream);
  if (image->header == NULL)
  {
    fprintf(stderr, "Error: This is not a BMP file.\n");
    free(image);
    return NULL;
  }

  // Get data(pixels) from a BMP file
  image->data = read_data(stream, image->header);
  if (image->data == NULL)
  {
    fprintf(stderr, "Error: Corrupted BMP file.\n");
    free(image->header);
    free(image);
    return NULL;
  }

  return image;
}

void free_bmp_image(struct bmp_image* image)
{
  if (image == NULL)
  {
    return;
  }
  // Free header segment
  if (image->header != NULL)
  {
    free(image->header);
  }
  // Free data segment
  if (image->data != NULL)
  {
    free(image->data);
  }
  // Free image
  free(image);
}

bool write_bmp(FILE* stream, const struct bmp_image* image)
{
  if (stream == NULL || image == NULL)
  {
    return false;
  }

  // Write BMP header to the stream
  if ( fwrite(image->header, sizeof(struct bmp_header), 1, stream) != 1 )
  {
    return false;
  }

  // Write pixels to the stream
  for (uint32_t i = 0; i < image->header->height; i++)
  {
    // Write one row at each iteration
    if ( fwrite(&image->data[i * image->header->width], sizeof(struct pixel), image->header->width, stream) != image->header->width )
    {
      return false;
    }
    // Determine and write padding
    const long padding = image->header->width % 4;
    if (padding != 0) fwrite(PADDING_CHAR, 1, padding, stream);
  }

  return true;
}