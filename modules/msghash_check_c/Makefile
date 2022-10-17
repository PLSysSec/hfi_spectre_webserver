WASM_CC=$(shell realpath .)/../../../wasi-sdk/bin/clang
WASM_CXX=$(shell realpath .)/../../../wasi-sdk/bin/clang++
WASM_AR=$(shell realpath .)/../../../wasi-sdk/bin/llvm-ar
WASM_RANLIB=$(shell realpath .)/../../../wasi-sdk/bin/ranlib
WASM_LD=$(shell realpath .)/../../../wasi-sdk/bin/wasm-ld
WASM_HOST=--host=wasm32-wasi
WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

build: ../msghash_check_c$(WASM_OUT_SUFFIX).wasm

./libsodium/Makefile:
	tar -xf ./libsodium.tar.xz

../msghash_check_c$(WASM_OUT_SUFFIX).wasm: msghash_check.cpp ./libsodium/Makefile
	cd libsodium && $(MAKE)
	$(WASM_CC)++ $(WASM_CFLAGS) msghash_check.cpp -I libsodium/src/libsodium/include libsodium/src/libsodium/.libs/libsodium.a -o $@

./native/app: msghash_check.cpp
	mkdir -p native
	cd libsodium && CFLAGS="-g" ./configure --disable-asm --disable-ssp && $(MAKE)
	$(CXX) -g msghash_check.cpp -I libsodium/src/libsodium/include libsodium/src/libsodium/.libs/libsodium.a -o $@

clean:
	-rm -rf ./native
	-rm -f ./libsodium/Makefile
	-rm -f ../msghash_check_c.wasm