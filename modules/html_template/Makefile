WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_AR=/opt/wasi-sdk/bin/llvm-ar
WASM_RANLIB=/opt/wasi-sdk/bin/ranlib
WASM_LD=/opt/wasi-sdk/bin/wasm-ld
WASM_CFLAGS=-std=c++11 -O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

build: ../html_template.wasm

../html_template.wasm: ./main.cpp
	$(WASM_CXX) $(WASM_CFLAGS) -I ./Jinja2CppLight/src/ ./Jinja2CppLight/src/Jinja2CppLight.cpp ./Jinja2CppLight/src/stringhelper.cpp ./main.cpp -o $@

clean:
	rm ../html_template.wasm
