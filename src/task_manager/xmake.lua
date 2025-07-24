add_deps("common")
add_links("common")

target("task_manager")
    set_default(true)

    set_kind("binary")
    add_files("entry/main.cpp")

    add_includedirs("../../3rd/imgui-1.91.8/backends")
    add_includedirs("../../3rd/imgui-1.91.8")
    add_files("../../3rd/imgui-1.91.8/imgui.cpp")
    add_files("../../3rd/imgui-1.91.8/imgui_tables.cpp")
    add_files("../../3rd/imgui-1.91.8/imgui_draw.cpp")
    add_files("../../3rd/imgui-1.91.8/imgui_widgets.cpp")
    add_files("../../3rd/imgui-1.91.8/backends/imgui_impl_dx11.cpp")
    add_files("../../3rd/imgui-1.91.8/backends/imgui_impl_win32.cpp")

    
    add_files("*.cpp")
    add_files("window/*.cpp")
    add_files("utils/*.cpp")
    add_files("http/*.cpp")

    add_links("user32", "gdi32","advapi32.lib")