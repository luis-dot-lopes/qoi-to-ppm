#include <stdio.h>
#include <stdlib.h>

char*
load_file(char* file_path, size_t* file_len)
{
  FILE* file = fopen(file_path, "rb");
  size_t size = GetFileSize(file, NULL);
  char* file_content = malloc(size + 1);
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
  char* image_data = load_file(argv[1], &image_len);

  return 0;
}