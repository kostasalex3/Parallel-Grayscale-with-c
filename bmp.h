#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define MAGIC_VALUE         0x4D42
#define NUM_PLANE           1
#define COMPRESSION         0
#define NUM_COLORS          0
#define IMPORTANT_COLORS    0
#define BITS_PER_PIXEL      24
#define BITS_PER_BYTE       8
#define BYTES_PER_PIXEL     BITS_PER_PIXEL/BITS_PER_BYTE

#pragma pack(push)  // save the original data alignment
#pragma pack(1)     // Set data alignment to 1 byte boundary

typedef struct BMPHeader_t_ { // Total: 54 bytes
  uint16_t  type;             // Magic identifier: 0x4d42
  uint32_t  size;             // File size in bytes
  uint16_t  reserved1;        // Not used
  uint16_t  reserved2;        // Not used
  uint32_t  offset;           // Offset to image data in bytes from beginning of file (54 bytes)
  uint32_t  dib_header_size;  // DIB Header size in bytes (40 bytes)
  int32_t   width_px;         // Width of the image
  int32_t   height_px;        // Height of image
  uint16_t  num_planes;       // Number of color planes
  uint16_t  bits_per_pixel;   // Bits per pixel
  uint32_t  compression;      // Compression type
  uint32_t  image_size_bytes; // Image size in bytes
  int32_t   x_resolution_ppm; // Pixels per meter
  int32_t   y_resolution_ppm; // Pixels per meter
  uint32_t  num_colors;       // Number of colors
  uint32_t  important_colors; // Important colors
} BMPHeader_t;

#pragma pack(pop)  // restore the previous pack setting

typedef struct BMPImage_t_ {
    BMPHeader_t header;
    uint8_t* data;
} BMPImage_t;

typedef struct RGB_t_ {
    uint8_t r, g, b;
} RGB_t;

const RGB_t BLACK = {0,0,0};
const RGB_t WHITE = {255,255,255};
const RGB_t RED = {255,0,0};
const RGB_t GREEN = {0,255,0};
const RGB_t BLUE = {0,0,255};

static int getPadding(const BMPHeader_t *header) {
    return (4 - (header->width_px * BYTES_PER_PIXEL) % 4) % 4;
}

/**
 * getRowSize calcualtes the row size of the image payload.
 * It's usefull to calculate position of the pixel in the image by its coordinates.
 **/
static int getRowSize(const BMPHeader_t *header) {
  int bytes_per_row_without_padding = header->width_px * BYTES_PER_PIXEL;
  return bytes_per_row_without_padding + getPadding(header);
}

/**
 * getPosition calculates a position of the pixel in the image payload
 **/
static int getPosition(const BMPHeader_t* header, int x, int y) {
  int j = (header->height_px - y - 1) * getRowSize(header);
  int i = x * BYTES_PER_PIXEL;

  return i + j;
}

int CheckHeader(const BMPHeader_t* header) {
  return header->type == MAGIC_VALUE
    && header->num_planes == NUM_PLANE
    && header->bits_per_pixel == BITS_PER_PIXEL
    && header->compression == COMPRESSION
    && header->num_colors == NUM_COLORS;
}

/**
 * ReadBMP opens file and reads its content into BMPImage_t struct.
 * This function allocates memory in the heap for the BMPImage_t object.
 * Don't forget to destroy it using DestroyBMP function.
 **/
BMPImage_t* ReadBMP(const char* filename) {
  BMPImage_t* bmp = malloc(sizeof(BMPImage_t));

  FILE* image = fopen(filename, "rb");
    
  if (image == NULL) {
        printf("Cannot open image file: %s\n", filename);
        exit(EXIT_FAILURE);
  }
    
  int ok;
  ok = fread(&bmp->header, 54, 1, image);
  if (ok != 1) {
    printf("Cannot read image header\n");
    exit(-1);
  }

//  printf("type=%x\n", bmp->header.type);
//  printf("size=%d\n", bmp->header.size);
//  printf("reserved1=%d\n", bmp->header.reserved1);
//  printf("reserved2=%d\n", bmp->header.reserved2);
//  printf("offset=%d\n", bmp->header.offset);
//  printf("dib_header_size=%d\n", bmp->header.dib_header_size);
//  printf("width_px=%d\n", bmp->header.width_px);
//  printf("height_px=%d\n", bmp->header.height_px);
//  printf("num_planes=%d\n", bmp->header.num_planes);
//  printf("bits_per_pixel=%d\n", bmp->header.bits_per_pixel);
//  printf("compression=%d\n", bmp->header.compression);
//  printf("image_size_bytes=%d\n", bmp->header.image_size_bytes);
//  printf("x_resolution_ppm=%d\n", bmp->header.x_resolution_ppm);
//  printf("y_resolution_ppm=%d\n", bmp->header.y_resolution_ppm);
//  printf("num_colors=%d\n", bmp->header.num_colors);
//  printf("important_colors=%d\n", bmp->header.important_colors);

  if (!CheckHeader(&bmp->header)) {
    printf("Bad image\n");
    exit(-1);
  }

  bmp->data = malloc(bmp->header.image_size_bytes);
  if (!bmp->data) {
    printf("Cannot allocate %d bytes\n", bmp->header.image_size_bytes);
    exit(-1);
  }

  fseek(image, bmp->header.offset, SEEK_SET);
  ok = fread(bmp->data, bmp->header.image_size_bytes, 1, image);
  if (ok != 1) {
    printf("Cannot read image data\n");
    exit(-1);
  }

  fclose(image);

  return bmp;
}

/**
 * CreateBMP creates a bitmap from raw-colors matrix.
 * Usefull when you would like to create image from scratch.
 *
 * TODO
 **/
BMPImage_t* CreateBMP(const RGB_t** image);

void SaveBMP(const BMPImage_t* bmp, const char* filename) {
  FILE* fd = fopen(filename, "wb");
  rewind(fd);
  fwrite(&bmp->header, 54, 1, fd);
  fseek(fd, bmp->header.offset, SEEK_SET);
  fwrite(bmp->data, bmp->header.image_size_bytes, 1, fd);
  fclose(fd);
}

/**
 * DestroyBMP destroys the struct with all the inner objects.
 * Preffere using this function, instead of manual destroyng the object.
 **/
void DestroyBMP(BMPImage_t* bmp) {
  if (bmp) {
    if (bmp->data) {
      free(bmp->data);
    }
    free(bmp);
  }
}

/**
 * GetPixel returns a pixel data in RGB_t struct by [x, y] coordinates.
 * Attentin. Y is growing down!!!
 * So y=0 is on the top of the image and Y=720 on the bottom, for instance.
 **/
RGB_t GetPixel(BMPImage_t* bmp, int x, int y) {
  if (x < 0 || x >= bmp->header.width_px) {
    printf("Error. X should be [%d, %d), but %d\n", 0, bmp->header.width_px, x);
    return BLACK;
  }

  if (y < 0 || y >= bmp->header.height_px) {
    printf("Error. Y should be [%d, %d), but %d\n", 0, bmp->header.height_px, y);
    return BLACK;
  }

  int pos = getPosition(&bmp->header, x, y);
  RGB_t rgb = {
    bmp->data[pos + 2],
    bmp->data[pos + 1],
    bmp->data[pos + 0],
  };
  return rgb;
}

/**
 * SetPixel sets pixel data RGB_t by coordinates [x,y].
 * The RGB_t is unpacked with all its magic.
 **/
void SetPixel(BMPImage_t* bmp, int x, int y, RGB_t rgb) {
  if (x < 0 || x >= bmp->header.width_px) {
    printf("Error. X should be [%d, %d), but %d\n", 0, bmp->header.width_px, x);
    return;
  }

  if (y < 0 || y >= bmp->header.height_px) {
    printf("Error. Y should be [%d, %d), but %d\n", 0, bmp->header.height_px, y);
    return;
  }

  int pos = getPosition(&bmp->header, x, y);
  bmp->data[pos + 2] = rgb.r;
  bmp->data[pos + 1] = rgb.g;
  bmp->data[pos + 0] = rgb.b;
}

RGB_t** ToImageMatrix(BMPImage_t* bmp) {
  RGB_t** image = calloc(bmp->header.height_px, sizeof(RGB_t*));
    int j,i;
  for ( j = 0; j < bmp->header.height_px; j++) {
    image[j] = calloc(bmp->header.width_px, sizeof(RGB_t));
    for ( i = 0; i < bmp->header.width_px; i++) {
      RGB_t pixel = GetPixel(bmp, i, j);
      image[j][i].r = pixel.r;
      image[j][i].g = pixel.g;
      image[j][i].b = pixel.b;
    }
  }

    
  return image;
}
