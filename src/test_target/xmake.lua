add_deps("common")
add_links("common")

target("test_target")
    set_default(true)
    set_kind("binary")

    add_files("entry/main.cpp")
    add_files("*.cpp")