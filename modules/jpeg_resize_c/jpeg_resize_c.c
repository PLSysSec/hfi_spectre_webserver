#include <stdio.h>
#include <stdlib.h>

#include "jpeglib.h"
#include "test_bytes.h"
#include "../server_hostcalls.h"

void put_scanline_someplace(JSAMPROW rowBuffer, int row_stride,
  JSAMPLE** p_image_buffer,
  int* p_curr_image_row)
{
  int i;
  for(i = 0; i < row_stride; i++)
  {
    (*p_image_buffer)[(*p_curr_image_row) * row_stride + i] = rowBuffer[i];
  }

  (*p_curr_image_row)++;
}


int
write_JPEG_file (int quality,
  JSAMPLE** p_image_buffer,
  // unsigned char** p_outfile,
  int* image_height,
  int* image_width,
  unsigned int * total
)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
  int row_stride;               /* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  unsigned char * outbuffer = 0;
  unsigned long outsize = 0;
  jpeg_mem_dest(&cinfo, &outbuffer, &outsize);


  cinfo.image_width = *image_width;      /* image width and height, in pixels */
  cinfo.image_height = *image_height;
  cinfo.input_components = 3;           /* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */

  jpeg_set_defaults(&cinfo);

  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);


  jpeg_start_compress(&cinfo, TRUE);


  row_stride = cinfo.image_width * 3; /* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    row_pointer[0] = & (*p_image_buffer)[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  jpeg_finish_compress(&cinfo);

  // (*p_outfile) = (unsigned char*) malloc(outsize);
  // for(unsigned long i = 0; i < outsize; i++) {
  //   (*p_outfile)[i] = outbuffer[i];
  // }
  // for(unsigned long i = 0; i < outsize; i++) {
  //   printf("%02X", outbuffer[i]);
  // }
  // *total = 0;
  // for(unsigned long i = 0; i < outsize; i++) {
  //   *total += outbuffer[i];
  // }

  server_module_bytearr_result(outbuffer, outsize);


  jpeg_destroy_compress(&cinfo);

  return 1;
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */
};

typedef struct my_error_mgr * my_error_ptr;


METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  abort();
}

GLOBAL(int)
read_JPEG_file (
  JSAMPLE** p_image_buffer,
  int* image_height,
  int* image_width,
  int* p_curr_image_row)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;

  FILE * infile;                /* source file */
  JSAMPARRAY buffer;            /* Output row buffer */
  int row_stride;               /* physical row width in output buffer */

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  jpeg_create_decompress(&cinfo);

  unsigned long fsize = sizeof(inputData) - 1;
  unsigned char *fileBuff = inputData;

  jpeg_mem_src(&cinfo, fileBuff, fsize);

  (void) jpeg_read_header(&cinfo, TRUE);

  (void) jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);


  //Save data
  (*image_height) = cinfo.output_height;
  (*image_width) = cinfo.output_width;
  (*p_image_buffer) = (JSAMPLE *) malloc(cinfo.output_width * cinfo.output_height * 3 * sizeof(JSAMPLE));

  if(!(*p_image_buffer))
  {
    printf("Memory alloc failure\n");
    return 1;
  }


  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    put_scanline_someplace(buffer[0], row_stride, p_image_buffer, p_curr_image_row);
  }

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return 1;
}

//Wierd compat thing with wasi
FILE* tmpfile() {
  abort();
}

int main(int argc, char** argv)
{
  if(argc != 2) {
    printf("One argument expected [(blank), quality], got %d args\n", argc);
    abort();
  }

  char * pEnd;
  int quality = strtol(argv[1], &pEnd, 10);
  if (!quality) {
    printf("argument provided must parse into a valid non zero uint32_t\n");
    abort();
  }

  JSAMPLE * image_buffer = NULL;
  // unsigned char* outfile= NULL;

  int image_height = 0;
  int image_width = 0;
  int curr_image_row = 0;

  if(!read_JPEG_file(&image_buffer, &image_height, &image_width, &curr_image_row))
  {
    printf("Reading JPEG failed\n");
    abort();
  }

  unsigned int total = 0;
  if(!write_JPEG_file(quality, &image_buffer, &image_height, &image_width, &total))
  {
    printf("Writing JPEG failed\n");
    abort();
  }
  printf("Success: %u\n", total);

  return 0;
}