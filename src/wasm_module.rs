use anyhow::Error;
use lucet_runtime::{self, Limits, MmapRegion, Module, Region};
use lucet_runtime_internals::instance::InstanceHandle;
use std::vec::Vec;

use crate::service_directory;

pub fn create_wasm_instance(endpoint:String, protection: String, input: Vec<String>) -> Result<InstanceHandle, Error> {
    let module_name = endpoint + "_" + &protection;
    let module = service_directory::get_module(&module_name).unwrap();
    let wasi_ctx = {
        let mut builder = lucet_wasi::WasiCtxBuilder::new();
        // The input to the computation is made available as arguments:
        builder.args(input);
        builder.build()?
    };
    let min_globals_size = module.initial_globals_size();
    let globals_size = ((min_globals_size + 4096 - 1) / 4096) * 4096;
    let region = MmapRegion::create(
        1,
        &Limits {
            heap_memory_size: 4 * 1024 * 1024 * 1024,
            heap_address_space_size: 8 * 1024 * 1024 * 1024,
            stack_size: 16 * 1024 * 1024,
            globals_size: globals_size,
            ..Limits::default()
        })?;

    let instance = region
        .new_instance_builder(module)
        .with_embed_ctx(wasi_ctx)
        .build()?;

    Ok(instance)
}