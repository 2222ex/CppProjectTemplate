add_deps("common")
add_links("common")

target("test_target")
    set_default(true)
    set_kind("binary")

    -- add_files("entry/main.cpp")

    add_includedirs(".")

    add_includedirs("../../3rd/Microsoft.Web.WebView2.1.0.3351.48/build/native/include")
    add_links("3rd/Microsoft.Web.WebView2.1.0.3351.48/build/native/x86/WebView2LoaderStatic.lib")
    add_links("3rd\\Microsoft.Web.WebView2.1.0.3351.48\\build\\native\\x86/WebView2LoaderStatic")

    add_files("web/*.cpp")

    -- add_includedirs("../../3rd/OpenSSL-3.4.1/include")
    -- add_links("3rd/OpenSSL-3.4.1/lib/libcrypto_static")
    -- add_links("3rd/OpenSSL-3.4.1/lib/libssl_static")

    add_links("user32", "gdi32","advapi32.lib")