fn main() {
    cc::Build::new()
        .file("c_src/noloopcopy.c")
        .compile("noloopcopy");
}
