WASM_CC=/opt/wasi-sdk/bin/clang
WASM_CXX=/opt/wasi-sdk/bin/clang++
WASM_CFLAGS=-O3 -Wl,--allow-undefined -Wl,--export-all -fno-exceptions
build: ../jpeg_resize_c.wasm

JPEG_FILES=./libjpeg/jaricom.c ./libjpeg/jcapimin.c ./libjpeg/jcapistd.c ./libjpeg/jcarith.c ./libjpeg/jccoefct.c ./libjpeg/jccolor.c ./libjpeg/jcdctmgr.c ./libjpeg/jchuff.c ./libjpeg/jcinit.c ./libjpeg/jcmainct.c ./libjpeg/jcmarker.c ./libjpeg/jcmaster.c ./libjpeg/jcomapi.c \
./libjpeg/jcparam.c ./libjpeg/jcprepct.c ./libjpeg/jcsample.c ./libjpeg/jctrans.c ./libjpeg/jdapimin.c ./libjpeg/jdapistd.c ./libjpeg/jdarith.c ./libjpeg/jdatadst.c ./libjpeg/jdatasrc.c ./libjpeg/jdcoefct.c ./libjpeg/jdcolor.c ./libjpeg/jddctmgr.c ./libjpeg/jdhuff.c \
./libjpeg/jdinput.c ./libjpeg/jdmainct.c ./libjpeg/jdmarker.c ./libjpeg/jdmaster.c ./libjpeg/jdmerge.c ./libjpeg/jdpostct.c ./libjpeg/jdsample.c ./libjpeg/jdtrans.c ./libjpeg/jerror.c ./libjpeg/jfdctflt.c ./libjpeg/jfdctfst.c ./libjpeg/jfdctint.c \
./libjpeg/jidctflt.c ./libjpeg/jidctfst.c ./libjpeg/jidctint.c ./libjpeg/jmemansi.c ./libjpeg/jmemmgr.c ./libjpeg/jquant1.c ./libjpeg/jquant2.c ./libjpeg/jutils.c \
./libjpeg/rdbmp.c ./libjpeg/rdcolmap.c ./libjpeg/rdgif.c ./libjpeg/rdppm.c ./libjpeg/rdrle.c ./libjpeg/rdswitch.c ./libjpeg/rdtarga.c ./libjpeg/transupp.c ./libjpeg/wrbmp.c ./libjpeg/wrgif.c \
./libjpeg/wrppm.c ./libjpeg/wrrle.c ./libjpeg/wrtarga.c

test_bytes.h: image.jpeg
	echo "unsigned char inputData[] = {" > $@
	hexdump -ve '1/1 "0x%.2x, "' $< >> $@
	echo "0x00 };" >> $@

../jpeg_resize_c.wasm: jpeg_resize_c.c $(JPEG_FILES) test_bytes.h
	$(WASM_CC) $(WASM_CFLAGS) $(JPEG_FILES) jpeg_resize_c.c -Wno-deprecated-declarations -I . -I libjpeg -o $@
