extern crate bindgen;

use std::env;
use std::path::{Path, PathBuf};

fn main() {
    let simba_path = Path::new("/Users/kolen/items/simba/");
    let include_paths = [
        "src",
        "src/kernel/ports/linux/gnu",
        "src/boards/linux",
        "src/mcus/linux",
        "src/filesystems",
        "3pp/compat",
        "src/drivers/ports/linux",
    ];

    let mut build = cc::Build::new();
    for include_path in &include_paths {
        build.include(simba_path.join(include_path));
    }

    build.flag_if_supported("-std=gnu99");
    build.file("src/scheduler.c");
    build.compile("scheduler");

    let bindings = bindgen::Builder::default()
        .header("src/scheduler.h")
        .whitelist_function("scheduler_init")
        .whitelist_function("scheduler_set_hourly")
        .whitelist_function("scheduler_set_minutely")
        .whitelist_function("scheduler_tick")
        .whitelist_function("sem_init")
        .derive_default(true)
        .clang_args(
            build
                .get_compiler()
                .args()
                .into_iter()
                .map(|s| s.to_string_lossy()),
        )
        .generate()
        .expect("Unable to generate bindings for scheduler.h");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("scheduler_bindings.rs"))
        .expect("Couldn't write bindings!");
}
