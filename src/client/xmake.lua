add_deps("common")
add_links("common")

target("client")
    set_default(true)

    set_kind("binary")
    add_files("entry/main.cpp")
    add_files("*.cpp")

    add_rules("protobuf.cpp")

    add_includedirs(".")
    

    add_links("user32", "gdi32","advapi32.lib")