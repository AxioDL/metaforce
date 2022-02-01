fn main() {
    let zeus_include = std::path::PathBuf::from(format!(
        "{}/../extern/zeus/include",
        std::env::var("CARGO_MANIFEST_DIR").unwrap()
    ))
    .canonicalize()
    .unwrap();
    let imgui_include = std::path::PathBuf::from(format!(
        "{}/../extern/imgui",
        std::env::var("CARGO_MANIFEST_DIR").unwrap()
    ))
    .canonicalize()
    .unwrap();
    let imgui_engine_include = std::path::PathBuf::from(format!(
        "{}/../imgui",
        std::env::var("CARGO_MANIFEST_DIR").unwrap()
    ))
    .canonicalize()
    .unwrap();
    // let include_dir = include_path.to_string_lossy();
    // let bindings = bindgen::Builder::default()
    //     .header(format!("{}/zeus/CVector3f.hpp", include_dir))
    //     .clang_arg(format!("-I{}", include_dir))
    //     .parse_callbacks(Box::new(bindgen::CargoCallbacks))
    //     .generate()
    //     .expect("Unable to generate bindings");
    // let out_path = std::path::PathBuf::from(std::env::var("OUT_DIR").unwrap());
    // bindings.write_to_file(out_path.join("zeus.rs")).expect("Couldn't write bindings!");

    cxx_build::bridge("src/lib.rs")
        .include("include")
        .include(zeus_include.clone())
        .compile("aurora");
    println!("cargo:rerun-if-changed=src/lib.rs");
    cxx_build::bridge("src/shaders/mod.rs")
        .include("include")
        .include(zeus_include.clone())
        .compile("aurora_shaders");
    println!("cargo:rerun-if-changed=src/shaders/mod.rs");
    cxx_build::bridge("src/imgui.rs")
        .include("include")
        .include(zeus_include.clone())
        .include(imgui_include.clone())
        .include(imgui_engine_include.clone())
        .compile("aurora_imgui");
    println!("cargo:rerun-if-changed=src/imgui.rs");
}
