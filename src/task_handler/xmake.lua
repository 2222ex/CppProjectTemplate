add_deps("common")
add_links("common")

target("task_handler")
    set_default(true)

    set_kind("shared")
    add_files("entry/dllmain.cpp")

    add_files("*.cpp")
    add_files("utils/*.cpp")
    add_files("*.cpp")
    add_files("http/*.cpp")

    add_includedirs(".")
    
    add_links("user32", "gdi32","advapi32.lib")