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
    build.define("TEST_ENVIRONMENT", None);
    build.file("src/scheduler.c");
    build.file("src/data_recorder.c");
    build.compile("pm_sensor_data");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    let bindings_scheduler = bindgen::Builder::default()
        .header("src/scheduler.h")
        .whitelist_function("scheduler_init")
        .whitelist_function("scheduler_set_hourly")
        .whitelist_function("scheduler_set_minutely")
        .whitelist_function("scheduler_tick")
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

    bindings_scheduler
        .write_to_file(out_path.join("scheduler_bindings.rs"))
        .expect("Couldn't write bindings!");

    let bindings_data_recorder = bindgen::Builder::default()
        .header("src/data_recorder.h")
        .whitelist_function("data_recorder_init")
        .whitelist_function("data_recorder_add_sample")
        .derive_default(true)
        .clang_args(
            build
                .get_compiler()
                .args()
                .into_iter()
                .map(|s| s.to_string_lossy()),
        )
        .generate()
        .expect("Unable to generate bindings for data_recorder.h");

    bindings_data_recorder
        .write_to_file(out_path.join("data_recorder_bindings.rs"))
        .expect("Couldn't write bindings!");

}
