set_xmakever("2.9.7")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
if is_plat("windows") then
    add_rules("plugin.vsxmake.autoupdate")
end

add_packages("boost")

local packages = {
    "nlohmann_json v3.11.3",
    "spdlog v1.14.1",
    "minhook",
    "cpp-httplib"
}

for _, package in ipairs(packages) do
    add_requires(package)
    add_packages(package:match("^[^ ]+"))
end

set_languages("c++23")

add_includedirs("src/base")

target("task_manager")
    set_default(true)

    set_kind("binary")
    add_files("src/entry/main.cpp")

    add_includedirs("3rd/imgui-1.91.8/backends")
    add_includedirs("3rd/imgui-1.91.8")
    add_files("3rd/imgui-1.91.8/imgui.cpp")
    add_files("3rd/imgui-1.91.8/imgui_tables.cpp")
    add_files("3rd/imgui-1.91.8/imgui_draw.cpp")
    add_files("3rd/imgui-1.91.8/imgui_widgets.cpp")
    add_files("3rd/imgui-1.91.8/backends/imgui_impl_dx11.cpp")
    add_files("3rd/imgui-1.91.8/backends/imgui_impl_win32.cpp")

    set_pcxxheader("src/base/stdafx.h")
    add_files("src/base/*.cpp")
    add_files("src/task_manager/*.cpp")
    add_files("src/task_manager/window/*.cpp")
    add_files("src/task_manager/utils/*.cpp")
    add_files("src/task_manager/http/*.cpp")

    add_links("user32", "gdi32","advapi32.lib")

target("task_handler")
    set_default(true)

    set_kind("shared")
    add_files("src/entry/dllmain.cpp")

    set_pcxxheader("src/base/stdafx.h")
    add_files("src/base/*.cpp")
    add_files("src/task_handler/utils/*.cpp")
    add_files("src/task_handler/*.cpp")
    add_files("src/task_handler/http/*.cpp")
    

    add_links("user32", "gdi32","advapi32.lib")

