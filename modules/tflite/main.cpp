#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>
#include <algorithm>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

extern "C" {
#include "efficientnet_lite3_fp32_1.h"
}

#define KILO (1024)
#define MEGA (KILO * KILO)
#define GIGA (KILO * MEGA)

#define PANIC(...) \
  fprintf(stderr, __VA_ARGS__); \
  exit(1) \

float getRandFloat(std::default_random_engine engine) {
  static std::uniform_real_distribution<float> dist(0.0, 1.0);
  return dist(engine);
}

int main(int argc, char* argv[]) {

  /* for now we don't use an input image - see more comments below
  if (argc <= 1) {
    PANIC("Not enough arguments: expected an image filename\n");
  }

  if (argc >= 3) {
    PANIC("Too many arguments: expected just one, an image filename\n");
  }
  */

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
  printf("Loading inputs...\n");
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
  for (int x = 0; x < X_MAX; x++) {
    for (int y = 0; y < Y_MAX; y++) {
      // This model expects R, G, and B values as floats in the range [0.0, 1.0].
      // For now we just generate random values
      float red = getRandFloat(generator);
      float green = getRandFloat(generator);
      float blue = getRandFloat(generator);
      // it's very possible that I have the colors switched around and/or the
      // axes transposed
      input->data.f[(x * X_MAX + y) * 3] = red;
      input->data.f[(x * X_MAX + y) * 3 + 1] = green;
      input->data.f[(x * X_MAX + y) * 3 + 2] = blue;
    }
  }
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
  const int NUM_CLASSES = 1000;
  // output is expected to be a 2D array with dimensions batch_size, NUM_CLASSES
  // we use batch_size 1
  if (output->dims->size != 2) {
    PANIC("Expected output to be 2D, but got %u dimensions\n", output->dims->size);
  }
  if (output->dims->data[0] != 1) {
    PANIC("Expected output batch size to be 1, got %u\n", output->dims->data[0]);
  }
  if (output->dims->data[1] != NUM_CLASSES) {
    PANIC("Expected output dimension to be %u, got %u\n", NUM_CLASSES, output->dims->data[1]);
  }
  if (output->type != kTfLiteFloat32) {
    PANIC("Expected output type to be 32-bit float\n");
  }
  std::vector<float> output_values;
  for (int i = 0; i < NUM_CLASSES; i++) {
    output_values.push_back(output->data.f[i]);
  }
  std::sort(output_values.begin(), output_values.end());
  printf("Top 5 output probabilities:\n");
  for (int i = 0; i < 5; i++) {
    printf("%g\n", output_values[NUM_CLASSES-1 - i]);
  }

  return 0;
}