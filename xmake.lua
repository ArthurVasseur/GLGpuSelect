add_rules("mode.debug", "mode.release")
includes("deps/ConcertoFramework/xmake.lua")

option("examples", { description = "Build examples", default = false })
option("profiling", { description = "Build with tracy profiler", default = false })
option("tests", { description = "Build tests", default = false })
option("asserts", { description = "Enable asserts", default = false })
option("logging", { description = "Enable logging", default = false })

if has_config("profiling") then
    add_requires("tracy")
end

rule("gen_gl_functions")
    set_extensions(".xml")

    on_config(function(target)
        local out_folder = target:autogendir()
        local out_hpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.hpp")
        local out_cpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.cpp")
        local gl_xml_file = path.join(out_folder, "gl.xml")

        target:add("files", out_cpp_file, gl_xml_file, {public = true, always_added = true})
        target:add("headerfiles", out_hpp_file, {public = true, install = false})
        target:add("includedirs", out_folder, {public = true})

        if not os.isfile(gl_xml_file) then
            import("net.http")
            http.download("https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/refs/heads/main/xml/gl.xml", gl_xml_file)
        end
    end)

    before_buildcmd_file(function(target, batchcmds, gl_file, opt)
        local out_folder = target:autogendir()
        local out_hpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.hpp")
        local out_cpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.cpp")
        
        batchcmds:show_progress(opt.progress, "${color.build.object}generate.gl %s", gl_file)
        batchcmds:vrunv("python.exe", { "./gen_gl_functions.py", path.absolute(gl_file), path.absolute(out_hpp_file), path.absolute(out_cpp_file)})

        batchcmds:set_depmtime(os.mtime(gl_file))
        batchcmds:add_depfiles(gl_file)
        --batchcmds:set_depcache(gl_file)
    end)

rule_end()

rule("gen_gl_dispatch_table")
    set_extensions(".xml")

    on_config(function(target)
        local out_folder = target:autogendir()
        local out_hpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.hpp")
        local gl_xml_file = path.join(out_folder, "gl.xml")

        target:add("files", gl_xml_file, {always_added = true})
        target:add("headerfiles", out_hpp_file, {public = true, install = false})
        target:add("includedirs", out_folder, {public = true})

        if not os.isfile(gl_xml_file) then
            import("net.http")
            http.download("https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/refs/heads/main/xml/gl.xml", gl_xml_file)
        end
    end)

    before_buildcmd_file(function(target, batchcmds, gl_file, opt)
        local out_folder = target:autogendir()
        local out_hpp_file = path.join(out_folder, "GlLoader", "OpenGlFunctions.hpp")

        batchcmds:show_progress(opt.progress, "${color.build.object}generate.gl.hpp %s", gl_file)
        batchcmds:vrunv("python.exe", {"./gen_gl_functions.py", path.absolute(gl_file), path.absolute(out_hpp_file)})

        batchcmds:set_depmtime(os.mtime(gl_file))
        batchcmds:add_depfiles(gl_file)
    end)

rule_end()

-- Interface GlLayer (header-only)
target("gl-layer")
    set_kind("headeronly")
    add_includedirs("Src", {public = true})
    add_headerfiles("Src/(GlLayer/**.hpp)")
    add_deps("concerto-core", {public = true})
    if is_plat("windows") then
        add_defines("NOGDI", {public = true})
    end
target_end()

-- Interface GlDriver (header-only, extends gl-layer)
target("gl-driver")
    set_kind("headeronly")
    add_includedirs("Src", {public = true})
    add_headerfiles("Src/(GlDriver/**.hpp)")
    add_deps("gl-layer", {public = true})
target_end()

-- WGL ICD driver implementation (gl-icd-driver.dll)
target("gl-icd-driver")
    set_kind("shared")
    set_languages("cxx20")
    add_files("Src/GlIcdDriver/**.cpp")
    add_includedirs("Src")
    add_deps("gl-driver")
    add_deps("concerto-core", {public = true})
    add_defines("GL_ICD_DRIVER_BUILD")
    add_rules("gen_gl_dispatch_table")
    if is_plat("windows") then
        add_syslinks("Gdi32")
        add_defines("NOGDI")
    end
    if has_config("asserts") then
        add_defines("GLGPUS_ASSERTS")
    end
    if has_config("logging") then
        add_defines("GLGPUS_LOGGING")
    end
target_end()

-- Validation layer (gl-validation.dll)
target("gl-validation")
    set_kind("shared")
    set_languages("cxx20")
    add_files("Src/GlValidationLayer/**.cpp")
    add_includedirs("Src")
    add_deps("gl-layer")
    add_deps("concerto-core", {public = true})
    add_defines("GL_VALIDATION_BUILD")
    add_rules("gen_gl_dispatch_table")
    if is_plat("windows") then
        add_defines("NOGDI")
    end
    if has_config("asserts") then
        add_defines("GLGPUS_ASSERTS")
    end
target_end()

target("opengl32")
    set_languages("cxx20")
    set_kind("shared")
    add_files("Src/GlLoader/**.cpp")
    add_includedirs("Src", { public = true })
    add_headerfiles("Src/(GlLoader/**.hpp)", { install = false })
    add_headerfiles("Src/(GlLoader/GLGpuSelect.h)")
    add_installfiles("Src/GlLoader/GLGpuSelect.h", { prefixdir = "include" })
    set_warnings("all")
    add_deps("concerto-core", {public = true})
    add_deps("gl-driver", { public = false })
    if is_plat("windows") then
        add_syslinks("Gdi32")
        add_defines("NOGDI")
    end
    if has_config("profiling") then
        add_packages("tracy")
        add_defines("GLGPUS_PROFILING")
    end
    if has_config("asserts") then
        add_defines("GLGPUS_ASSERTS")
    end
    if has_config("logging") then
        add_defines("GLGPUS_LOGGING")
    end
    add_defines("GLGPUS_BUILD")
    add_rules("gen_gl_functions")
target_end()

if has_config("examples") then
    add_requires("libsdl3", { configs = {debug = true, with_symbols = true, shared = true} })

    target("sdl-renderer-example")
        set_languages("c99")
        set_kind("binary")
        add_files("Src/SdlRendererExample/**.cpp")
        add_deps("concerto-core", {public = true})
        if is_plat("windows") then
            add_syslinks("Gdi32")
        end
        add_deps("opengl32")
        add_packages("libsdl3")
    target_end()

    target("c-api-example")
        set_languages("c89")
        set_kind("binary")
        add_files("Src/CApiExample/**.c")
        add_deps("concerto-core", {public = true})
        add_deps("opengl32")
    target_end()
end

if has_config("tests") then
    add_requires("catch2")

    target("opengl32-tests")
        set_languages("cxx20")
        set_kind("binary")
        add_files("Src/Tests/*.cpp")
        add_packages("catch2")
        add_deps("opengl32")
        if is_plat("windows") then
            add_syslinks("Gdi32", "User32")
        end
    target_end()
end