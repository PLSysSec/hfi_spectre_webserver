
CURR_DIR=$(shell realpath .)
LUCET_SRC:=$(shell realpath ../../lucet-spectre)
LUCET=$(LUCET_SRC)/target/release/lucetc
LUCET_COMMON_FLAGS=--bindings $(LUCET_SRC)/lucet-wasi/bindings.json \
--bindings $(CURR_DIR)/bindings.json \
--guard-size "4GiB" --min-reserved-size "4GiB" --max-reserved-size "4GiB"

# Note this makefile uses the CET binaries only if REALLY_USE_CET is defined
ifdef REALLY_USE_CET
	CET_CFLAGS:=-fcf-protection=full
	CET_LINKERFLAGS:=-Wl,-z,ibt -Wl,-z,shstk
else
	CET_CFLAGS:=
	CET_LINKERFLAGS:=
endif

.PHONY: modules
modules: jpeg_resize_c_all msghash_check_c_all fib_c_all html_template_all xml_to_json_all tflite_all

WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions
WASM_CFLAGS_UNROLL_LOOPS=-funroll-loops -mllvm --unroll-runtime -mllvm --unroll-runtime-epilog

.PRECIOUS: %.wasm
%.wasm:
	if [ -e $*/Makefile ]; then \
		WASM_CFLAGS="$(WASM_CFLAGS)" make -C $* build; \
	else \
		cargo build -p $* --target wasm32-wasi --release && \
		cp target/wasm32-wasi/release/$*.wasm $*.wasm; \
	fi

.PRECIOUS: %.wasm
%_unrolled.wasm:
	if [ -e $*/Makefile ]; then \
		WASM_OUT_SUFFIX="_unrolled" WASM_CFLAGS="$(WASM_CFLAGS) $(WASM_CFLAGS_UNROLL_LOOPS)" make -C $* build; \
	else \
		cargo build -p $* --target wasm32-wasi --release && \
		cp target/wasm32-wasi/release/$*.wasm $*.wasm; \
	fi

.PRECIOUS: %_stock.so
%_stock.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) $< -o $@

.PRECIOUS: %_spectre_sfi_aslr.so
%_spectre_sfi_aslr.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation sfiaslr $< -o $@

.PRECIOUS: %_spectre_cet_aslr.so
%_spectre_cet_aslr.so: %.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation cetaslr $< -o $@

.PRECIOUS: %_spectre_sfi_full.so
%_spectre_sfi_full.so: %_unrolled.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation sfi $< -o $@

.PRECIOUS: %_spectre_cet_full.so
%_spectre_cet_full.so: %_unrolled.wasm
	$(LUCET) $(LUCET_COMMON_FLAGS) --spectre-mitigation cet $< -o $@

%_all: \
%.wasm \
%_unrolled.wasm \
%_stock.so \
%_spectre_sfi_aslr.so \
%_spectre_cet_aslr.so \
%_spectre_sfi_full.so \
%_spectre_cet_full.so
	touch $*_all

clean:
	rm -f *.wasm
	rm -f *.so
	rm -f *_all
