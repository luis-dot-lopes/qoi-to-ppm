#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32

#include <fileapi.h>

#endif

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
decode_qoi(uint8_t* image_data,
           size_t image_len,
           size_t* image_width,
           size_t* image_height)
{
  pixel seen_pixels[64] = { 0 };
  pixel prev_pixel = { .r = 0, .g = 0, .b = 0, .a = 255 };
  *image_width = (image_data[4] << 24) + (image_data[5] << 16) +
                 (image_data[6] << 8) + (image_data[7] << 0);
  *image_height = (image_data[8] << 24) + (image_data[9] << 16) +
                  (image_data[10] << 8) + (image_data[11] << 0);
  pixel* decoded_pixels =
    malloc(sizeof(pixel) * (*image_width) * (*image_height));
  image_data += 14;
  image_len -= 14;
  if (decoded_pixels == NULL) {
    printf("Coundn't alloc memory\n");
    return NULL;
  }
  size_t decoded_count = 0;
  while (decoded_count < ((*image_height) * (*image_width))) {
    uint8_t byte = image_data[0];
    if (byte != 0b11111110 && byte != 0b11111111) {
      if (((byte & 0b11000000) >> 6) == 0b00) {
        decoded_pixels[decoded_count++] = seen_pixels[byte & ((1 << 6) - 1)];
        prev_pixel = decoded_pixels[decoded_count - 1];
      } else if (((byte & 0b11000000) >> 6) == 0b01) {
        int dr = ((byte & (0b11 << 4)) >> 4) - 2;
        int dg = ((byte & (0b11 << 2)) >> 2) - 2;
        int db = ((byte & (0b11 << 0)) >> 0) - 2;
        pixel cur_pixel = (pixel){ .r = (uint8_t)((int)prev_pixel.r + dr),
                                   .g = (uint8_t)((int)prev_pixel.g + dg),
                                   .b = (uint8_t)((int)prev_pixel.b + db),
                                   .a = prev_pixel.a };
        decoded_pixels[decoded_count++] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[(cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 +
                     cur_pixel.a * 11) %
                    64] = cur_pixel;
      } else if (((byte & 0b11000000) >> 6) == 0b10) {
        int dg = ((int)(byte & 0b111111)) - 32;
        image_data++;
        byte = image_data[0];

        int dr_dg = ((int)((byte & 0b11110000) >> 4)) - 8;
        int db_dg = ((int)(byte & 0b1111)) - 8;

        int dr = dr_dg + dg;
        int db = db_dg + dg;

        pixel cur_pixel = (pixel){ .r = prev_pixel.r + dr,
                                   .g = prev_pixel.g + dg,
                                   .b = prev_pixel.b + db,
                                   .a = prev_pixel.a };

        decoded_pixels[decoded_count++] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[(cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 +
                     cur_pixel.a * 11) %
                    64] = cur_pixel;
      } else if (((byte & 0b11000000) >> 6) == 0b11) {
        size_t run = (byte & 0b111111) + 1;
        for (size_t i = 0; i < run; ++i) {
          decoded_pixels[decoded_count++] = prev_pixel;
        }
      }
    } else {
      if (byte & 1) {

        pixel cur_pixel;

        image_data++;
        byte = image_data[0];

        cur_pixel.r = byte;

        image_data++;
        byte = image_data[0];

        cur_pixel.g = byte;

        image_data++;
        byte = image_data[0];

        cur_pixel.b = byte;

        image_data++;
        byte = image_data[0];

        cur_pixel.a = byte;
        decoded_pixels[decoded_count++] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[(cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 +
                     cur_pixel.a * 11) %
                    64] = cur_pixel;
      } else {
        pixel cur_pixel;

        image_data++;
        byte = image_data[0];

        cur_pixel.r = byte;

        image_data++;
        byte = image_data[0];

        cur_pixel.g = byte;

        image_data++;
        byte = image_data[0];

        cur_pixel.b = byte;

        cur_pixel.a = prev_pixel.a;

        decoded_pixels[decoded_count++] = cur_pixel;
        prev_pixel = cur_pixel;
        seen_pixels[(cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 +
                     cur_pixel.a * 11) %
                    64] = cur_pixel;
      }
    }
    image_data++;
  }
  printf("%d\n", decoded_count);
  return decoded_pixels;
}

int
compare_pixels(pixel p1, pixel p2)
{
  return p1.r == p2.r && p1.g == p2.g && p1.b == p2.b && p1.a == p2.a;
}

uint8_t*
encode_qoi(pixel* image_pixels,
           size_t pixels_len,
           size_t image_width,
           size_t image_height,
           size_t* data_len)
{
  pixel seen_pixels[64] = { 0 };
  pixel prev_pixel;
  uint8_t* image_data = malloc(sizeof(pixel) * pixels_len);
  size_t bytes_written = 0;
  qoi_header header = { .magic = { 'q', 'o', 'i', 'f' },
                        .channels = 3,
                        .colorspace = 0,
                        .width = image_width,
                        .height = image_height };
  memcpy(image_data, header.magic, 4);
  bytes_written += 4;

  image_data[bytes_written++] = (image_width >> 24) & 0b11111111;
  image_data[bytes_written++] = (image_width >> 16) & 0b11111111;
  image_data[bytes_written++] = (image_width >> 8) & 0b11111111;
  image_data[bytes_written++] = (image_width >> 0) & 0b11111111;

  image_data[bytes_written++] = (image_height >> 24) & 0b11111111;
  image_data[bytes_written++] = (image_height >> 16) & 0b11111111;
  image_data[bytes_written++] = (image_height >> 8) & 0b11111111;
  image_data[bytes_written++] = (image_height >> 0) & 0b11111111;

  image_data[bytes_written++] = header.channels;
  image_data[bytes_written++] = header.colorspace;

  uint8_t run;

  for (size_t i = 0; i < pixels_len; ++i) {
    pixel cur_pixel = image_pixels[i];
    uint8_t hash_index =
      (cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 + cur_pixel.a * 11) %
      64;
    pixel hash_pixel = seen_pixels[hash_index];
    int dr = cur_pixel.r - prev_pixel.r;
    int dg = cur_pixel.g - prev_pixel.g;
    int db = cur_pixel.b - prev_pixel.b;
    int dr_dg = dr - dg;
    int db_dg = db - dg;

    if (dr == 0 && dg == 0 && db == 0 && run < 63) {
      run++;
      continue;
    }
    seen_pixels[(cur_pixel.r * 3 + cur_pixel.g * 5 + cur_pixel.b * 7 +
                 cur_pixel.a * 11) %
                64] = cur_pixel;
    if (run > 0) {
      uint8_t chunk = 0b11000000 | (run - 1);
      image_data[bytes_written++] = chunk;
    }
    if (compare_pixels(cur_pixel, hash_pixel)) {
      uint8_t chunk = 0b00 | hash_index;
      image_data[bytes_written++] = chunk;
    } else if (-2 <= dr && dr <= 1 && -2 <= dg && dg <= 1 && -2 <= db &&
               db <= 1) {
      uint8_t chunk =
        0b01 | ((dr + 2) << 4) | ((dg + 2) << 2) | ((db + 2) << 0);
      image_data[bytes_written++] = chunk;
    } else if (-32 <= dg && dg <= 31 && -8 <= dr_dg && dr_dg <= 7 &&
               -8 <= db_dg && db_dg <= 7) {
      uint8_t chunk1 = 0b10 | (dg + 32);
      uint8_t chunk2 = ((dr_dg + 8) << 4) | (db_dg + 8);
      image_data[bytes_written++] = chunk1;
      image_data[bytes_written++] = chunk2;
    } else {
      image_data[bytes_written++] = 0b11111111;
      image_data[bytes_written++] = cur_pixel.r;
      image_data[bytes_written++] = cur_pixel.g;
      image_data[bytes_written++] = cur_pixel.b;
    }
  }

  *data_len = bytes_written;
  return image_data;
}

void
write_ppm_to_file(char* file_path,
                  pixel* pixels,
                  size_t image_width,
                  size_t image_height)
{
  FILE* file = fopen(file_path, "wb");
  fprintf(file, "P6\n%d %d 255\n", image_width, image_height);
  for (size_t y = 0; y < image_height; ++y) {
    for (size_t x = 0; x < image_width; ++x) {
      fwrite(&pixels[y * image_width + x].r, sizeof(uint8_t), 1, file);
      fwrite(&pixels[y * image_width + x].g, sizeof(uint8_t), 1, file);
      fwrite(&pixels[y * image_width + x].b, sizeof(uint8_t), 1, file);
    }
  }
  fclose(file);
}

pixel*
read_pixels_from_ppm(uint8_t* data, size_t* image_width, size_t* image_height)
{
  char header[41];
  memcpy(header, data, 40);
  header[40] = '\0';
  if (sscanf(header, "P6\n%d %d 255\n", image_width, image_height) < 2) {
    printf("Error while reading ppm data");
    return NULL;
  }
  for (int i = 0; i < 2; ++data)
    if (*data = '\n')
      ++i;
  ++data;
  pixel* pixels = malloc(sizeof(pixel) * (*image_height) * (*image_width));
  for (size_t i = 0; i < (*image_height) * (*image_width); i += 3) {
    pixel cur_pixel;
    cur_pixel.r = data[i];
    cur_pixel.g = data[i + 1];
    cur_pixel.b = data[i + 2];
    cur_pixel.a = 255;
  }
  return pixels;
}

uint8_t*
load_file(char* file_path, size_t* file_len)
{
  FILE* file = fopen(file_path, "rb");
#ifdef _WIN32
  size_t size = GetFileSize(file, NULL);
#endif
#ifdef __linux__
  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);
#endif
  uint8_t* file_content = malloc(size);
  fread(file_content, size, 1, file);
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
    printf("Too many command line arguments\n");
    return 1;
  }

  size_t image_len, image_width, image_height;
  uint8_t* image_data = load_file(argv[1], &image_len);

  printf("image_len: %d\n", image_len);

  if (image_len < sizeof(qoi_header)) {
    printf("Invalid image\n");
    return 1;
  }

  pixel* pixels =
    decode_qoi(image_data, image_len, &image_width, &image_height);
  if (pixels == NULL) {
    return 1;
  }

  write_ppm_to_file("out.ppm", pixels, image_width, image_height);

  free(pixels);
  free(image_data);

  return 0;
}
