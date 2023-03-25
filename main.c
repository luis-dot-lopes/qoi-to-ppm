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
  pixel seen_pixels[64] = { 0 };
  pixel prev_pixel = { .r = 0, .g = 0, .b = 0, .a = 255 };
  qoi_header header = *((qoi_header*)image_data);
  image_data += sizeof(qoi_header);
  image_len -= sizeof(qoi_header);
  uint32_t image_width = header.width, image_height = header.height;
  pixel* decoded_pixels = malloc(sizeof(pixel) * image_width * image_height);
  size_t decoded_count = 0;
  while (image_len > 8) {
    uint8_t byte = image_data[0];
    if (!(byte & ((1 << 7) - 1) << 1)) {
      if (byte & (0b11 << 6) == 0b00) {
        decoded_pixels[decoded_count] = seen_pixels[byte & ((1 << 6) - 1)];
        prev_pixel = decoded_pixels[decoded_count]; // May cause problems
      } else if (byte & (0b11 << 6) == 0b01) {
        int dr = (byte & (0b11 << 4)) - 2;
        int dg = (byte & (0b11 << 2)) - 2;
        int db = (byte & (0b11 << 0)) - 2;
        pixel cur_pixel = (pixel){ .r = prev_pixel.r + dr,
                                                 .g = prev_pixel.g + dg,
                                                 .b = prev_pixel.b + db,
                                                 .a = prev_pixel.a };
        decoded_pixels[decoded_count] = cur_pixel;
        prev_pixel = cur_pixel; // May cause problems
        seen_pixels[cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 + cur_pixel.a * 11] = cur_pixel;
      } else if (byte & (0b11 << 6) == 0b10) {
        int dg = byte & ((1 << 6) - 1);
        image_data++;
        image_len--;
        byte = image_data[0] - 32;

        int dr_dg = (byte & ((1 << 4) - 1) << 4) - 8;
        int db_dg = (byte & ((1 << 4) - 1)) - 8;

        int dr = dr_dg + dg;
        int db = db_dg + dg;

        pixel cur_pixel = (pixel){ .r = prev_pixel.r + dr,
                                                 .g = prev_pixel.g + dg,
                                                 .b = prev_pixel.b + db,
                                                 .a = prev_pixel.a };

        decoded_pixels[decoded_count] = cur_pixel
        prev_pixel = cur_pixel; // May cause problems
        seen_pixels[cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 + cur_pixel.a * 11] = cur_pixel;
      } else {
        size_t run = byte & ((1 << 6) - 1);
        for(size_t i = 0; i < run; ++i) {
            decoded_pixels[decoded_count] = prev_pixel;
        }
        decoded_count += run - 1;
      }
      decoded_count++;
      image_data++;
      image_len--;
    } else {
      if(byte & 1) {

        pixel cur_pixel;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.r = byte;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.g = byte;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.b = byte;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.a = byte;
        decoded_pixels[decoded_count] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 + cur_pixel.a * 11] = cur_pixel;
        decoded_count++;
      } else {
        pixel cur_pixel;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.r = byte;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.g = byte;

        image_data++;
        image_len--;
        byte = image_data[0];

        cur_pixel.b = byte;
        
        cur_pixel.a = prev_pixel.a;

        decoded_pixels[decoded_count] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 + cur_pixel.a * 11] = cur_pixel;
        decoded_count++;
      }
    }
  }
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
    return 1;
  }

  size_t image_len;
  uint8_t* image_data = load_file(argv[1], &image_len);

  if (image_len < sizeof(qoi_header)) {
    printf("Invalid image");
    return 1;
  }

  pixel* pixels = decode_qoi(image_data, image_len);
  if (pixels == NULL) {
    return 1;
  }

  free(pixels);

  return 0;
}
