#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  char magic[4];
  uint32_t width;
  uint32_t height;
  uint8_t channels;
  uint8_t colorspace;
} qoi_header;

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pixel;

pixel*
decode_qoi(uint8_t* image_data, size_t image_len)
{
}

uint8_t*
load_file(char* file_path, size_t* file_len)
{
  FILE* file = fopen(file_path, "rb");
  size_t size = GetFileSize(file, NULL);
  uint8_t* file_content = malloc(size + 1);
  fread(file_content, size, 1, file);
  file_content[size] = 0;
  *file_len = size;
  fclose(file);
  return file_content;
}

int
main(int argc, char** argv)
{

  if (argc < 2) {
    printf("No input files\n");
    return 1;
  } else if (argc > 2) {
    printf("Too many command line arguments");
  }

  size_t image_len;
  uint8_t* image_data = load_file(argv[1], &image_len);

  pixel* pixels = decode_qoi(image_data, image_len);

  return 0;
}