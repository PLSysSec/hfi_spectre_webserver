#include <stdio.h>
#include <stdlib.h>

#define PANIC(msg) \
  fprintf(stderr, msg "\n"); \
  exit(1)

int pthread_create(void* thread, const void* attr, void* fn, void* arg) {
  PANIC("Tried to call pthread_create");
}

int pthread_join(void* thread, void** val_ptr) {
  PANIC("Tried to call pthread_join");
}
