WASM_CC=$(shell realpath .)/../../../wasi-sdk/bin/clang
WASM_CXX=$(shell realpath .)/../../../wasi-sdk/bin/clang++
WASM_AR=$(shell realpath .)/../../../wasi-sdk/bin/llvm-ar
WASM_RANLIB=$(shell realpath .)/../../../wasi-sdk/bin/ranlib
WASM_LD=$(shell realpath .)/../../../wasi-sdk/bin/wasm-ld

build: ../xml_to_json$(WASM_OUT_SUFFIX).wasm

../xml_to_json$(WASM_OUT_SUFFIX).wasm: main.cpp
	$(WASM_CC)++ $(WASM_CFLAGS) main.cpp -I xml2json/include -o $@

native_app: main.cpp
	$(CXX) -g main.cpp -I xml2json/include -o $@

clean:
	-rm -f ./native_app
	-rm -f ../xml_to_json.wasm