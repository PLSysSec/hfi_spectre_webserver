WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_AR=/opt/wasi-sdk/bin/llvm-ar
WASM_RANLIB=/opt/wasi-sdk/bin/ranlib
WASM_LD=/opt/wasi-sdk/bin/wasm-ld
WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

build: ../xml_to_json.wasm

../xml_to_json.wasm: main.cpp
	$(WASM_CC)++ $(WASM_CFLAGS) main.cpp -I xml2json/include -o $@

native_app: main.cpp
	$(CXX) -g main.cpp -I xml2json/include -o $@

clean:
	-rm -f ./native_app
	-rm -f ../xml_to_json.wasm