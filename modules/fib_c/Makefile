
CURR_DIR=$(shell realpath .)
WASM_CLANG=/opt/wasi-sdk/bin/clang
WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

build: $(CURR_DIR)/../fib_c.wasm

$(CURR_DIR)/../fib_c.wasm: fib.cpp
	$(WASM_CLANG)++ $(WASM_CFLAGS) $< -o $@