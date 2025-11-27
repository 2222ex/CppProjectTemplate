set_xmakever("2.9.7")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
if is_plat("windows") then
    add_rules("plugin.vsxmake.autoupdate")
end

local packages = {
    "nlohmann_json v3.11.3",
    "spdlog v1.15.3",
    "safetyhook",
    "cpp-httplib"
}

for _, package in ipairs(packages) do
    add_requires(package)
    add_packages(package:match("^[^ ]+"))
end

set_languages("c++23")



if is_mode("debug") then
    set_runtimes("MDd")
    set_optimize("none")
    set_warnings("all", "extra")

else
    set_runtimes("MD")
    set_optimize("fastest")
end

includes("src/common")
includes("src/task_manager")
includes("src/task_handler")
includes("src/test_target")


