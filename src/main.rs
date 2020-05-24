#![feature(proc_macro_hygiene, decl_macro)]

#[macro_use] extern crate rocket;

use rocket::response::Stream;
use rocket::response::content;

use std::io;
use std::io::Stdin;

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

fn main() {
    rocket::ignite()
    .mount("/", routes![index, bytes, stream, image])
    .launch();
}