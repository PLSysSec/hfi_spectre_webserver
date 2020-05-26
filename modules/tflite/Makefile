CURR_DIR=$(shell realpath .)
NATIVE_CC=clang
NATIVE_CXX=$(NATIVE_CC)++
WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=$(WASM_CC)++

TENSORFLOW_MICRO_DIR=./tensorflow/tensorflow/lite/micro
TENSORFLOW_DOWNLOADS_DIR=$(TENSORFLOW_MICRO_DIR)/tools/make/downloads

CXXFLAGS=-I./tensorflow \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/gemmlowp \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/flatbuffers/include \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/ruy \
				 -g

MODELFILE=efficientnet_lite3_fp32_1

NATIVE_LIBDIR=$(TENSORFLOW_MICRO_DIR)/tools/make/gen/linux_x86_64/lib
NATIVE_LDFLAGS=-L$(NATIVE_LIBDIR) -ltensorflow-microlite
WASM_LIBDIR=$(TENSORFLOW_MICRO_DIR)/tools/make/gen/wasi/lib  # no rule to make this yet
WASM_LDFLAGS=-L$(WASM_LIBDIR) -ltensorflow-microlite

.PHONY: build
build: $(CURR_DIR)/../tflite.wasm

$(CURR_DIR)/../tflite.wasm: main.cpp $(MODELFILE).h $(MODELFILE).o $(WASM_LIBDIR)/libtensorflow-microlite.a
	$(WASM_CXX) $(CXXFLAGS) $< $(MODELFILE).o -o $@ $(WASM_LDFLAGS)

native: main.cpp $(MODELFILE).h $(MODELFILE).o $(NATIVE_LIBDIR)/libtensorflow-microlite.a
	$(NATIVE_CXX) $(CXXFLAGS) $< $(MODELFILE).o -o $@ $(NATIVE_LDFLAGS)

$(MODELFILE).o: $(MODELFILE).c
	$(NATIVE_CC) $< -c -o $@

$(MODELFILE).c: $(MODELFILE).tflite
	echo "static const" > $@
	xxd -i $< >> $@
	cat model_suffix.c >> $@

$(NATIVE_LIBDIR)/libtensorflow-microlite.a: $(TENSORFLOW_DOWNLOADS_DIR)
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile microlite

$(TENSORFLOW_DOWNLOADS_DIR):
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile third_party_downloads

.PHONY: clean
clean:
	-rm $(CURR_DIR)/../tflite.wasm
	-rm native
	-rm *.o
	-rm $(MODELFILE).c
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile clean
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile clean_downloads
