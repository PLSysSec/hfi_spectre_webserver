WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_AR=/opt/wasi-sdk/bin/llvm-ar
WASM_RANLIB=/opt/wasi-sdk/bin/ranlib
WASM_LD=/opt/wasi-sdk/bin/wasm-ld

build: ../html_template$(WASM_OUT_SUFFIX).wasm

../html_template$(WASM_OUT_SUFFIX).wasm: ./main.cpp
	$(WASM_CXX) -std=c++11 $(WASM_CFLAGS) -I ./Jinja2CppLight/src/ ./Jinja2CppLight/src/Jinja2CppLight.cpp ./Jinja2CppLight/src/stringhelper.cpp ./main.cpp -o $@

clean:
	rm ../html_template.wasm
