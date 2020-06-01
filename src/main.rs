#![feature(proc_macro_hygiene, decl_macro)]

#[macro_use]
extern crate rocket;

#[macro_use]
extern crate clap;

use anyhow::Error;
use clap::Arg;
use lazy_static::lazy_static;
use libc::c_char;
use lucet_runtime::DlModule;
use lucet_runtime_internals::{lucet_hostcall, vmctx::Vmctx};
use rocket::data::{Data, FromDataSimple, Outcome};
use rocket::http::{RawStr, Status};
use rocket::response::{content, Stream};
use rocket::Outcome::{Failure, Success};
use rocket::Request;
use std::cell::RefCell;
use std::ffi::CStr;
use std::io;
use std::io::{Read, Stdin};
use std::path::PathBuf;
use std::slice;

mod service_directory;
mod wasm_module;

#[get("/")]
fn index() -> &'static str {
    "Hello, world!"
}

#[get("/bytes")]
fn bytes() -> content::Plain<Vec<u8>> {
    let a = vec![48, 49, 50];
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
    String(String),
}

thread_local! {
    pub static CURRENT_RESULT: RefCell<ModuleResult> = RefCell::new(ModuleResult::None);
}

#[lucet_hostcall]
#[no_mangle]
pub extern "C" fn server_module_bytearr_result(vmctx: &mut Vmctx, byte_arr: u32, bytes: u32) {
    let heap = vmctx.heap();
    let byte_arr_ptr = heap.as_ptr() as usize + byte_arr as usize;
    let response_slice =
        unsafe { slice::from_raw_parts(byte_arr_ptr as *const u8, bytes as usize) };
    let v = response_slice.to_vec();
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::ByteArray(v);
    });
}

#[lucet_hostcall]
#[no_mangle]
pub extern "C" fn server_module_string_result(vmctx: &mut Vmctx, string_resp: u32) {
    let heap = vmctx.heap();
    let byte_arr_ptr = heap.as_ptr() as usize + string_resp as usize;
    let char_ptr = byte_arr_ptr as *const c_char;
    let c_str: &CStr = unsafe { CStr::from_ptr(char_ptr) };
    let str_slice: &str = c_str.to_str().unwrap();
    let str_buf: String = str_slice.to_owned(); // if necessary
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::String(str_buf);
    });
}

#[derive(Responder)]
#[response(content_type = "image/jpeg")]
struct JPEGResponder {
    bytes: Vec<u8>,
}

#[get("/jpeg_resize_c?<quality>&<protection>")]
fn jpeg_resize_c(quality: &RawStr, protection: &RawStr) -> Result<JPEGResponder, Error> {
    let input: Vec<String> = vec!["".to_string(), quality.to_string()];
    let instance = wasm_module::create_wasm_instance(
        "jpeg_resize_c".to_string(),
        protection.to_string(),
        input,
        unsafe { ASLR.clone() },
        MODULE_PATH.clone(),
    );
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance?.run(lucet_wasi::START_SYMBOL, &[])?;
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    let ret = CURRENT_RESULT.with(|current_result| {
        let r = match &*current_result.borrow() {
            ModuleResult::ByteArray(b) => JPEGResponder { bytes: b.clone() },
            _ => {
                panic!("Unexpected response from module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });
    Ok(ret)
}

#[get("/msghash_check_c?<msg>&<hash>&<protection>")]
fn msghash_check_c(msg: &RawStr, hash: &RawStr, protection: &RawStr) -> Result<String, Error> {
    let input: Vec<String> = vec!["".to_string(), msg.to_string(), hash.to_string()];
    let instance = wasm_module::create_wasm_instance(
        "msghash_check_c".to_string(),
        protection.to_string(),
        input,
        unsafe { ASLR.clone() },
        MODULE_PATH.clone(),
    );
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance?.run(lucet_wasi::START_SYMBOL, &[])?;
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    let ret = CURRENT_RESULT.with(|current_result| {
        let r = match &*current_result.borrow() {
            ModuleResult::String(s) => s.clone(),
            _ => {
                panic!("Unexpected response from module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });
    Ok(ret)
}

#[get("/fib_c?<num>&<protection>")]
fn fib_c(num: &RawStr, protection: &RawStr) -> Result<String, Error> {
    let input: Vec<String> = vec!["".to_string(), num.to_string()];
    let instance = wasm_module::create_wasm_instance(
        "fib_c".to_string(),
        protection.to_string(),
        input,
        unsafe { ASLR.clone() },
        MODULE_PATH.clone(),
    );
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance?.run(lucet_wasi::START_SYMBOL, &[])?;
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    let ret = CURRENT_RESULT.with(|current_result| {
        let r = match &*current_result.borrow() {
            ModuleResult::String(s) => s.clone(),
            _ => {
                panic!("Unexpected response from module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });
    Ok(ret)
}

#[get("/html_template?<protection>")]
fn html_template(protection: &RawStr) -> Result<String, Error> {
    let input: Vec<String> = vec!["".to_string()];
    let instance = wasm_module::create_wasm_instance(
        "html_template".to_string(),
        protection.to_string(),
        input,
        unsafe { ASLR.clone() },
        MODULE_PATH.clone(),
    );
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance?.run(lucet_wasi::START_SYMBOL, &[])?;
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    let ret = CURRENT_RESULT.with(|current_result| {
        let r = match &*current_result.borrow() {
            ModuleResult::String(s) => s.clone(),
            _ => {
                panic!("Unexpected response from jpeg module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });
    Ok(ret)
}

pub struct StringCopy(pub String);

const LIMIT: u64 = 256 * 1024;
impl FromDataSimple for StringCopy {
    type Error = String;

    fn from_data(_: &Request<'_>, data: Data) -> Outcome<Self, String> {
        let mut string = String::new();
        if let Err(e) = data.open().take(LIMIT).read_to_string(&mut string) {
            return Failure((Status::InternalServerError, format!("{:?}", e)));
        }
        Success(StringCopy(string))
    }
}

#[post("/xml_to_json?<protection>", data = "<xml>")]
fn xml_to_json(xml: StringCopy, protection: &RawStr) -> Result<String, Error> {
    let str_data: String = xml.0;
    let xml_decoded = percent_encoding::percent_decode_str(&str_data).decode_utf8()?;
    let input: Vec<String> = vec!["".to_string(), xml_decoded.to_string()];
    let instance = wasm_module::create_wasm_instance(
        "xml_to_json".to_string(),
        protection.to_string(),
        input,
        unsafe { ASLR.clone() },
        MODULE_PATH.clone(),
    );
    CURRENT_RESULT.with(|current_result| {
        *current_result.borrow_mut() = ModuleResult::None;
    });

    let result = instance?.run(lucet_wasi::START_SYMBOL, &[])?;
    if !result.is_returned() {
        panic!("wasm module yielded?");
    }

    let ret = CURRENT_RESULT.with(|current_result| {
        let r = match &*current_result.borrow() {
            ModuleResult::String(s) => s.clone(),
            _ => {
                panic!("Unexpected response from module");
            }
        };
        *current_result.borrow_mut() = ModuleResult::None;
        return r;
    });
    Ok(ret)
}

#[no_mangle]
pub extern "C" fn ensure_linked() {
    lucet_runtime::lucet_internal_ensure_linked();
    lucet_wasi::export_wasi_funcs();
}

static mut ASLR: bool = false;

lazy_static! {
    static ref MODULE_PATH: PathBuf = {
        let mut m = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        m.push("modules");
        m
    };
}

fn main() {
    ensure_linked();

    let matches = app_from_crate!()
    .arg(
        Arg::with_name("sandbox_cores")
            .long("--sandbox-cores")
            .takes_value(true)
            .help("No effect if app poisoning protections are disabled. Number of cores to dedicate for sandboxes if app poisoning protection is enabled. If not set, we use the defaults, which is to use half the cores for sandboxes."),
    )
    .arg(
        Arg::with_name("aslr")
            .long("--aslr")
            .takes_value(false)
            .help("Whether to use ASLR for wasm sandbox code pages."),
    )
    .get_matches();

    let sandbox_cores = matches
        .value_of("sandbox_cores")
        .map(|t| t.parse::<usize>().unwrap());
    cranelift_spectre::runtime::use_spectre_mitigation_core_partition(sandbox_cores);

    let aslr = matches.is_present("aslr");
    if aslr {
        println!("Enabling ASLR for wasm modules");
        unsafe {
            ASLR = true;
        }
        DlModule::aslr_dl_enable();
    }

    let module_path = MODULE_PATH.clone();
    println!("module directory: {:?}", module_path);

    service_directory::load_dir(module_path).unwrap();

    rocket::ignite()
        .mount(
            "/",
            routes![
                index,
                bytes,
                stream,
                jpeg_resize_c,
                msghash_check_c,
                fib_c,
                html_template,
                xml_to_json
            ],
        )
        .launch();
}
