use anyhow::Error;
use lazy_static::lazy_static;
use lucet_runtime::DlModule;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use std::sync::Arc;
use std::sync::RwLock;

lazy_static! {
    static ref MODULE_MAP : RwLock<HashMap<String, Arc<DlModule>>> = RwLock::new(HashMap::new());
}


/// Load services from a directory in the filesystem.
/// Lucet compiles Wasm modules to native code in `.so` files. Load all `.so` files in a
/// directory. Refer to them by the base of their filenames (i.e. drop the `.so` extension).
pub fn load_dir<P: AsRef<Path>>(dir: P) -> Result<(), Error> {
    let mut mapg = MODULE_MAP.write().unwrap();
    for entry in fs::read_dir(dir.as_ref())? {
        let file = entry?.path();
        if let Some(ext) = file.extension() {
            if ext == "so" {
                let dl_module_load = DlModule::load(&file);
                if dl_module_load.is_ok() {
                    let dl_module = dl_module_load?;

                    let basename = file
                        .file_stem()
                        .expect("file has stem")
                        .to_str()
                        .expect("filename is utf-8")
                        .to_owned();
                    println!("Found module {}", basename);
                    mapg.insert(basename, dl_module);
                } else {
                    println!("Could not load module: {}", file.file_name().unwrap().to_str().unwrap());
                }
            }
        }
    }
    Ok(())
}

/// Lookup a Wasm module by its name.
pub fn get_module(name: &str) -> Option<Arc<DlModule>> {
    let mapg = MODULE_MAP.read().unwrap();
    mapg.get(name).cloned()
}

