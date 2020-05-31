WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_AR=/opt/wasi-sdk/bin/llvm-ar
WASM_RANLIB=/opt/wasi-sdk/bin/ranlib
WASM_LD=/opt/wasi-sdk/bin/wasm-ld
WASM_HOST=--host=wasm32-wasi
WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

build: ../msghash_check_c.wasm

./libsodium/Makefile:
	tax -xf ./libsodium.tar.xz
	cd libsodium && CC=$(WASM_CC) AR=$(WASM_AR) RANLIB=$(WASM_RANLIB) LD=$(WASM_LD) CFLAGS="$(WASM_CFLAGS)" ./configure --disable-asm --disable-ssp $(WASM_HOST)

../msghash_check_c.wasm: msghash_check.cpp ./libsodium/Makefile
	cd libsodium && $(MAKE)
	$(WASM_CC)++ $(WASM_CFLAGS) msghash_check.cpp -I libsodium/src/libsodium/include libsodium/src/libsodium/.libs/libsodium.a -o $@

./native/app: msghash_check.cpp
	mkdir -p native
	cd libsodium && CFLAGS="-g" ./configure --disable-asm --disable-ssp && $(MAKE)
	$(CXX) -g msghash_check.cpp -I libsodium/src/libsodium/include libsodium/src/libsodium/.libs/libsodium.a -o $@

clean:
	cd ./libsodium && $(MAKE) clean
	-rm -rf ./native
	-rm -f ./libsodium/Makefile
	-rm -f ../msghash_check_c.wasm