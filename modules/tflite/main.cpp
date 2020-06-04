#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

extern "C" {
#include "efficientnet_lite3_fp32_1.h"
#include "ImageNetLabels.h"
}

#define KILO (1024)
#define MEGA (KILO * KILO)
#define GIGA (KILO * MEGA)

static const size_t NUM_CLASSES = 1000;

#define PANIC(...) \
  fprintf(stderr, __VA_ARGS__); \
  abort() \

float getRandFloat(std::default_random_engine engine) {
  static std::uniform_real_distribution<float> dist(0.0, 1.0);
  return dist(engine);
}

// Empirically, this endianness correction is necessary for the bmp height and width fields
int endianness_correct(int input) {
  unsigned int uinput = (unsigned int) input;
  unsigned char byte_0 = uinput & 0xFF;
  unsigned char byte_1 = (uinput >> 8) & 0xFF;
  unsigned char byte_2 = (uinput >> 16) & 0xFF;
  unsigned char byte_3 = (uinput >> 24) & 0xFF;
  unsigned int uoutput = (byte_2 << 24) | (byte_3 << 16) | (byte_0 << 8) | byte_1;
  // also we shave off the top 16 bits, as our bitmap seems to have junk data there in the
  // height field, despite the spec I found on wikipedia
  uoutput &= 0xFFFF;
  return (int) uoutput;
}

// Fills `buffer` with the contents of (just the raw pixel data from) `bitmap`
// When this function returns, `buffer` will contain three `float` values per
// pixel, corresponding to the R, G, and B values (in that order), with each
// float in the range [0.0, 1.0].
void readBitmapToBuffer(
    FILE* bitmap,
    float* buffer,
    int expected_width,
    int expected_height
) {
  // thanks to https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
  // for examples of reading bitmaps in C++

  // read header
  const int HEADER_SIZE = 54;
  unsigned char info[HEADER_SIZE];
  fread(info, sizeof(unsigned char), 54, bitmap);
  const int found_width = endianness_correct(*(int*)&info[18]);
  const int found_height = endianness_correct(*(int*)&info[22]);
  if (found_width != expected_width) {
    PANIC("Found bitmap width %i, expected %i\n", found_width, expected_width);
  }
  if (found_height != expected_height) {
    PANIC("Found bitmap height %i, expected %i\n", found_height, expected_height);
  }
  /*
  const int data_start_offset = *(int*)&info[0x0A];
  if (data_start_offset != HEADER_SIZE) {
    PANIC("Found data_start_offset %i, expected %i\n", data_start_offset, HEADER_SIZE);
  }
  */

  // read the rest of the raw data
  const int remaining_bytes = found_width * found_height * 3;  // 3 bytes per pixel
  unsigned char* bmp_data = (unsigned char*) malloc(remaining_bytes);
  if (bmp_data == nullptr) {
    PANIC("Failed to allocate buffer for bitmap data\n");
  }
  fread(bmp_data, sizeof(unsigned char), remaining_bytes, bitmap);
  fclose(bitmap);

  for (int y = 0; y < found_height; y++) {
    for (int x = 0; x < found_width; x++) {
      const unsigned idx = (y * found_width + x) * 3;  // index of this pixel in both `bmp_data` and `buffer`
      //float red = getRandFloat(generator);
      //float green = getRandFloat(generator);
      //float blue = getRandFloat(generator);
      const float red = bmp_data[idx + 2] / 255.0;  // +2 because bmps are apparently stored B,G,R
      const float green = bmp_data[idx + 1] / 255.0;
      const float blue = bmp_data[idx] / 255.0;
      buffer[idx] = red;
      buffer[idx + 1] = green;
      buffer[idx + 2] = blue;
    }
  }

  free(bmp_data);
}

#ifdef STUB_EXCEPTIONS
extern "C" {
void* __cxa_allocate_exception(size_t size) {
  return malloc(size);
}
void __cxa_throw (void *thrown_exception, void *tinfo, void* dest) {
  abort();
}
}
#endif

struct ImageNetResult {
  const char* classname;
  float probability;
};

bool sortByProbabilityDecreasing(const ImageNetResult& resA, const ImageNetResult& resB) {
  return (resA.probability > resB.probability);
}

// Returns a vector of ImageNetResults with all probabilities set to 0.
// In this vector, index 0 is class 0, index 1 is class 1, etc
std::vector<ImageNetResult> get_imagenet_classes() {

  std::vector<ImageNetResult> vec;
  for (int i = 0; i < NUM_CLASSES; i++) {
    vec.push_back(ImageNetResult { image_classes[i], 0.0 });
  }
  return vec;
}

int main(int argc, char* argv[]) {

  /* for now we simply hardcode the image filename
  if (argc <= 1) {
    PANIC("Not enough arguments: expected an image filename\n");
  }

  if (argc >= 3) {
    PANIC("Too many arguments: expected just one, an image filename\n");
  }
  */
  const char image_filename[] = "banana-bitmap.bmp";

  TfLiteStatus retcode;

  static std::default_random_engine generator;
  generator.seed(time(NULL));

  // Load the model
  printf("Loading model...\n");
  const tflite::Model* const model = ::tflite::GetModel(get_model_data());
  if (model == nullptr) {
    PANIC("Failed to load the model\n");
  }
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    PANIC("Model has the wrong schema version: expected %u, got %u\n", TFLITE_SCHEMA_VERSION, model->version());
  }
  printf("Loaded\n");

  // Build the interpreter
  tflite::ops::micro::AllOpsResolver resolver;  // this uses more mem than necessary, we could cherry-pick ops
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;
  const int tensor_arena_size = 24 * MEGA;  // experimentally this is enough for our efficientnet model
  uint8_t* tensor_arena = (uint8_t*) malloc(tensor_arena_size);
  if (tensor_arena == nullptr) {
    PANIC("Failed to allocate tensor_arena\n");
  }
  tflite::MicroInterpreter* interpreter = new tflite::MicroInterpreter( \
      model, resolver, tensor_arena, tensor_arena_size, error_reporter);
  if (interpreter == nullptr) {
    PANIC("Failed to initialize interpreter: got nullptr\n");
  }
  if (interpreter->initialization_status() != kTfLiteOk) {
    PANIC("Failed to initialize interpreter\n");
  }

  // Allocate tensors
  retcode = interpreter->AllocateTensors();
  if (retcode != kTfLiteOk) {
    PANIC("Failed to allocate tensors\n");
  }

  // This model apparently expects images which are 280x280 pixels
  const int X_MAX = 280;
  const int Y_MAX = 280;
  printf("Loading image data...\n");
  TfLiteTensor* input = interpreter->input(0);
  if (input == nullptr) {
    PANIC("Failed to get input tensor\n");
  }
  // input is expected to be a 4D array with dimensions batch_size, height, width, 3
  // (the 3 is for R, G, B)
  // we use batch_size 1
  if (input->dims->size != 4) {
    PANIC("Expected input to be 4D, but got %u dimensions\n", input->dims->size);
  }
  if (input->dims->data[1] != X_MAX) {
    PANIC("Expected first input dimension to be %u, got %u\n", X_MAX, input->dims->data[1]);
  }
  if (input->dims->data[2] != Y_MAX) {
    PANIC("Expected second input dimension to be %u, got %u\n", Y_MAX, input->dims->data[2]);
  }
  if (input->dims->data[3] != 3) {
    PANIC("Expected three color channels, got %u\n", input->dims->data[3]);
  }
  if (input->type != kTfLiteFloat32) {
    PANIC("Expected input type to be 32-bit float\n");
  }
  // load in the actual data
  FILE* f = fopen(image_filename, "rb");
  // input->dims->data.f[idx] = ...
  readBitmapToBuffer(f, interpreter->typed_input_tensor<float>(0), X_MAX, Y_MAX);
  printf("Loaded\n");

  // Actually run inference.
  printf("Running inference...\n");
  retcode = interpreter->Invoke();
  if (retcode != kTfLiteOk) {
    PANIC("Failed to run inference\n");
  }
  printf("Complete\n");

  // Collect the output
  TfLiteTensor* output = interpreter->output(0);
  if (output == nullptr) {
    PANIC("Failed to get output tensor\n");
  }
  // output is expected to be a 2D array with dimensions batch_size, NUM_CLASSES
  // we use batch_size 1
  if (output->dims->size != 2) {
    PANIC("Expected output to be 2D, but got %u dimensions\n", output->dims->size);
  }
  if (output->dims->data[0] != 1) {
    PANIC("Expected output batch size to be 1, got %u\n", output->dims->data[0]);
  }
  if (output->dims->data[1] != NUM_CLASSES) {
    PANIC("Expected output dimension to be %zu, got %u\n", NUM_CLASSES, output->dims->data[1]);
  }
  if (output->type != kTfLiteFloat32) {
    PANIC("Expected output type to be 32-bit float\n");
  }
  std::vector<ImageNetResult> results = get_imagenet_classes();
  for (int i = 0; i < NUM_CLASSES; i++) {
    results[i].probability = output->data.f[i];
  }
  std::sort(results.begin(), results.end(), &sortByProbabilityDecreasing);
  printf("Top 5 class candidates:\n");
  for (int i = 0; i < 5; i++) {
    printf("%s (%.2f%%)\n", results[i].classname, 100.0 * results[i].probability);
  }

  return 0;
}
