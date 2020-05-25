#![feature(proc_macro_hygiene, decl_macro)]

#[macro_use] extern crate rocket;

use rocket::http::RawStr;
use rocket::response::Stream;
use rocket::response::content;

use lucet_runtime::{DlModule, InstanceHandle, MmapRegion, Region, TerminationDetails, Limits};
use lucet_runtime_internals::{lucet_hostcall, vmctx::Vmctx};

use std::cell::RefCell;
use std::io;
use std::io::Stdin;
use std::os::raw::{c_char};
use std::slice;
use std::slice::from_raw_parts;

#[get("/")]
fn index() -> &'static str {
    "Hello, world!"
}

#[get("/bytes")]
fn bytes() -> content::Plain<Vec<u8>> {
    let a  = vec![ 48, 49, 50 ];
    content::Plain(a)
}

#[get("/stream")]
fn stream() -> Stream<Stdin> {
    let response = Stream::chunked(io::stdin(), 10);
    response
}

#[derive(Clone)]
pub enum ModuleResult {
    None,
    ByteArray(Vec<u8>),
}

thread_local! {
    pub static CURRENT_RESULT: RefCell<ModuleResult> = RefCell::new(ModuleResult::None);
}

#[lucet_hostcall]
#[no_mangle]
pub extern "C" fn server_module_bytearr_result(vmctx: &mut Vmctx, byte_arr: u32, bytes: u32) {
    let heap = vmctx.heap();
    let byte_arr_ptr = heap.as_ptr() as usize + byte_arr as usize;
    let response_slice = unsafe {
        slice::from_raw_parts(
            byte_arr_ptr as *const u8,
            bytes as usize
        )
    };
    let v = response_slice.to_vec();
    CURRENT_RESULT.with(|current_result|{
        *current_result.borrow_mut() = ModuleResult::ByteArray(v);
    });
}

#[derive(Responder)]
#[response(content_type = "image/jpeg")]
struct JPEGResponder {
    bytes: Vec<u8>,
}


#[get("/image")]
fn image() -> JPEGResponder {
    let bytes = std::fs::read("/home/shr/Code/SpectreSandboxing/wasm_compartments/modules/jpeg_resize_c/image.jpeg").unwrap();
    return JPEGResponder {
        bytes
    };
}

#[get("/jpeg_resize_c?<quality>")]
fn jpeg_resize_c(quality: &RawStr) -> JPEGResponder {
    let module = DlModule::load("/home/shr/Code/SpectreSandboxing/spectresfi_webserver/modules/jpeg_resize_c_stock.so").unwrap();
    let wasi_ctx = {
        let mut builder = lucet_wasi::WasiCtxBuilder::new();
        // The input to the computation is made available as arguments:
        let input = vec!["", quality.as_str()];
        builder.args(input);
        builder.build().unwrap()
    };
    let region = MmapRegion::create(
        1,
        &Limits {
            heap_memory_size: 4 * 1024 * 1024 * 1024,
            heap_address_space_size: 8 * 1024 * 1024 * 1024,
            stack_size: 16 * 1024 * 1024,
            ..Limits::default()
        }).unwrap();

    let mut instance = region
        .new_instance_builder(module)
        .with_embed_ctx(wasi_ctx)
        .build()
        .unwrap();

    CURRENT_RESULT.with(|current_result|{
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance.run(lucet_wasi::START_SYMBOL, &[]).unwrap();
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    return CURRENT_RESULT.with(|current_result|{
        let r = match &*current_result.borrow() {
            ModuleResult::ByteArray(b) => {
                JPEGResponder {
                    bytes: b.clone()
                }
            },
            _ => {
                panic!("Unexpected response from jpeg module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });

}

#[no_mangle]
pub extern "C" fn ensure_linked() {
    lucet_runtime::lucet_internal_ensure_linked();
    lucet_wasi::export_wasi_funcs();
}

fn main() {
    ensure_linked();

    let module = DlModule::load("/home/shr/Code/SpectreSandboxing/wasm_compartments/modules/jpeg_resize_c_stock.so").unwrap();

    rocket::ignite()
    .mount("/", routes![index, bytes, stream, image, jpeg_resize_c])
    .launch();
}