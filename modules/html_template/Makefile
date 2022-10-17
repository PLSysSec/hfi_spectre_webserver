WASM_CC=$(shell realpath .)/../../../wasi-sdk/bin/clang
WASM_CXX=$(shell realpath .)/../../../wasi-sdk/bin/clang++
WASM_AR=$(shell realpath .)/../../../wasi-sdk/bin/llvm-ar
WASM_RANLIB=$(shell realpath .)/../../../wasi-sdk/bin/ranlib
WASM_LD=$(shell realpath .)/../../../wasi-sdk/bin/wasm-ld

build: ../html_template$(WASM_OUT_SUFFIX).wasm

../html_template$(WASM_OUT_SUFFIX).wasm: ./main.cpp
	$(WASM_CXX) -std=c++11 $(WASM_CFLAGS) -I ./Jinja2CppLight/src/ ./Jinja2CppLight/src/Jinja2CppLight.cpp ./Jinja2CppLight/src/stringhelper.cpp ./main.cpp -o $@

clean:
	rm ../html_template.wasm
