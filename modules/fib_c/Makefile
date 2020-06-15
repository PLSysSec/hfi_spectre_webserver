
CURR_DIR=$(shell realpath .)
WASM_CLANG=/opt/wasi-sdk/bin/clang

build: $(CURR_DIR)/../fib_c$(WASM_OUT_SUFFIX).wasm

$(CURR_DIR)/../fib_c$(WASM_OUT_SUFFIX).wasm: fib.cpp
	$(WASM_CLANG)++ $(WASM_CFLAGS) $< -o $@