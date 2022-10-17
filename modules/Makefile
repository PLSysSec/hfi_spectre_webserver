
CURR_DIR=$(shell realpath .)
LUCET_SRC:=$(shell realpath ../../lucet-spectre)
LUCET=$(LUCET_SRC)/target/release/lucetc
LUCET_COMMON_FLAGS=--bindings $(LUCET_SRC)/lucet-wasi/bindings.json \
--bindings $(CURR_DIR)/bindings.json \
--guard-size "4GiB" --min-reserved-size "4GiB" --max-reserved-size "4GiB"


.PHONY: modules
modules: jpeg_resize_c_all msghash_check_c_all fib_c_all html_template_all xml_to_json_all tflite_all echo_server_all

WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions

.PRECIOUS: %.wasm
%.wasm:
	if [ -e $*/Makefile ]; then \
		WASM_CFLAGS="$(WASM_CFLAGS)" make -C $* build; \
	else \
		cargo build -p $* --target wasm32-wasi --release && \
		cp target/wasm32-wasi/release/$*.wasm $*.wasm; \
	fi

.PRECIOUS: %_stock.so
%_stock.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) $< -o $@

.PRECIOUS: %_spectre_sfi_breakout.so
%_spectre_sfi_breakout.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation sfi --spectre-stop-sbx-breakout $< -o $@

.PRECIOUS: %_spectre_hfi_breakout.so
%_spectre_hfi_breakout.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation hfi $< -o $@

%_all: \
%.wasm \
%_stock.so \
%_spectre_sfi_breakout.so \
%_spectre_hfi_breakout.so
	touch $*_all

clean:
	rm -f *.wasm
	rm -f *.so
	rm -f *_all
