package("vk-gl-cts")
    set_homepage("https://github.com/wolfpld/tracy")
    set_description("C++ frame profiler")

    add_urls("https://github.com/KhronosGroup/VK-GL-CTS/archive/refs/tags/$(version).zip",
             "https://github.com/KhronosGroup/VK-GL-CTS.git")

    add_versions("vulkan-cts-1.4.2.1", "aa94839aeac38ae36a5b31e85168d13c2a80c1f83ba32be572b958816ae601fe")
    add_deps("python 3.x", "cmake")

    on_install(function (package)
        import("lib.detect.find_tool")
        local python = find_tool("python")
        assert(python, "python not found!")

        os.vrunv(python.program, {"external/fetch_sources.py"})

        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=Release")
        table.insert(configs, "-DDEQP_TARGET=default")
        table.insert(configs, "-DGLCTS_GTF_TARGET=gl")
        table.insert(configs, "-DSELECTED_BUILD_TARGETS=glcts")

        import("package.tools.cmake").install(package, configs)
        os.vcp(path.join(package:buildir(), "external/openglcts/modules/Release/*.*"), package:installdir("bin"))
    end)

    on_test(function (package)
        local bin_path = package:installdir("bin")
        local glcts = path.join(bin_path, "glcts.exe")
        if os.isfile(glcts) then
            os.execv(glcts, {"--help"})
        else
            return false
        end
    end)
package_end()


package("vk-gl-cts")
    set_homepage("https://piglit.freedesktop.org/")
    set_description("OpenGL and Vulkan Conformance Test Suite")

    add_urls("https://gitlab.freedesktop.org/mesa/piglit.git")

    add_versions("05.07.2025", "1767af745ed96f77b16c0c205015366d1fbbdb22")
    add_deps("python 3.x", "cmake")

    on_install(function (package)
        import("lib.detect.find_tool")
        local python = find_tool("python")
        assert(python, "python not found!")

        os.vrunv(python.program, {"external/fetch_sources.py"})

        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=Release")
        table.insert(configs, "-DDEQP_TARGET=default")
        table.insert(configs, "-DGLCTS_GTF_TARGET=gl")
        table.insert(configs, "-DSELECTED_BUILD_TARGETS=glcts")

        import("package.tools.cmake").install(package, configs)
        os.vcp(path.join(package:buildir(), "external/openglcts/modules/Release/*.*"), package:installdir("bin"))
    end)

    on_test(function (package)
        local bin_path = package:installdir("bin")
        local glcts = path.join(bin_path, "glcts.exe")
        if os.isfile(glcts) then
            os.execv(glcts, {"--help"})
        else
            return false
        end
    end)
package_end()




