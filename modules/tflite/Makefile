CURR_DIR=$(shell realpath .)
NATIVE_CC=gcc
NATIVE_CXX=g++
WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_AR=/opt/wasi-sdk/bin/llvm-ar
WASM_RANLIB=/opt/wasi-sdk/bin/ranlib
WASM_LD=/opt/wasi-sdk/bin/wasm-ld

TENSORFLOW_MICRO_DIR=./tensorflow/tensorflow/lite/micro
TENSORFLOW_DOWNLOADS_DIR=$(TENSORFLOW_MICRO_DIR)/tools/make/downloads

CXXFLAGS=-I./tensorflow \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/gemmlowp \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/flatbuffers/include \
				 -I$(TENSORFLOW_DOWNLOADS_DIR)/ruy \
				 -g

MODELFILE=efficientnet_lite3_fp32_1
MODELFILE_WASM=efficientnet_lite3_fp32_1_wasm

LIBDIR=$(TENSORFLOW_MICRO_DIR)/tools/make/gen/linux_x86_64/lib
NATIVE_LDFLAGS=-L$(LIBDIR) -ltensorflow-microlite
WASM_LDFLAGS=-L$(LIBDIR) -ltensorflow-microlite

.PHONY: build
build: $(CURR_DIR)/../tflite$(WASM_OUT_SUFFIX).wasm

$(CURR_DIR)/../tflite$(WASM_OUT_SUFFIX).wasm: main.cpp $(MODELFILE).h $(MODELFILE_WASM).o $(LIBDIR)/libtensorflow-microlite.a test_bytes.h
	$(WASM_CXX) $(WASM_CFLAGS) $(CXXFLAGS) -DSTUB_EXCEPTIONS $< $(MODELFILE_WASM).o -o $@ $(WASM_LDFLAGS) -D INPUT_FILENAME='"$(CURR_DIR)/banana-bitmap.bmp"'

test_bytes.h: banana-bitmap.bmp
	echo "unsigned char inputData[] = {" > $@
	hexdump -ve '1/1 "0x%.2x, "' $< >> $@
	echo "0x00 };" >> $@

# A successful native build will also require the env var NO_WASM to be set
# and you might have to clean if you've previously done a wasm build
native: main.cpp $(MODELFILE).h $(MODELFILE).o $(LIBDIR)/libtensorflow-microlite.a test_bytes.h
	$(NATIVE_CXX) $(CXXFLAGS) $< $(MODELFILE).o -o $@ $(NATIVE_LDFLAGS)

$(MODELFILE).o: $(MODELFILE).c
	$(NATIVE_CC) $< -c -o $@

$(MODELFILE_WASM).o: $(MODELFILE).c
	$(WASM_CC) $< -c -o $@

$(MODELFILE).c: $(MODELFILE).tflite
	echo "static const" > $@
	xxd -i $< >> $@
	cat model_suffix.c >> $@

testing: $(LIBDIR)/libtensorflow-microlite.a

$(LIBDIR)/libtensorflow-microlite.a: $(TENSORFLOW_DOWNLOADS_DIR)
ifdef NO_WASM
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile microlite
else
	cd ./tensorflow && \
	CC=$(WASM_CC) \
	CXX=$(WASM_CXX) \
	AR=$(WASM_AR) \
	RANLIB=$(WASM_RANLIB) \
	LD=$(WASM_LD) \
	CFLAGS="$(WASM_CFLAGS)" \
	TARGET=wasm32 \
	TARGET_ARCH=wasi \
	$(MAKE) -f tensorflow/lite/micro/tools/make/Makefile \
	CC_TOOL=$(WASM_CC) \
	CXX_TOOL=$(WASM_CXX) \
	AR_TOOL=$(WASM_AR) \
	microlite
endif

$(TENSORFLOW_DOWNLOADS_DIR):
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile third_party_downloads

.PHONY: clean
clean:
	-rm $(CURR_DIR)/../tflite.wasm
	-rm native
	-rm *.o
	-rm $(MODELFILE).c
	cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile clean
	# cd ./tensorflow && make -f tensorflow/lite/micro/tools/make/Makefile clean_downloads
