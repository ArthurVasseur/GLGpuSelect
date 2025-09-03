import xml.etree.ElementTree as ET
import urllib.request
import os

FEATURES = ["GL_VERSION_1_0", "GL_VERSION_1_1"]
FEATURE_COMMANDS = []

def parse_gl_xml(xml_bytes):
    """Parse the XML and return its root element."""
    return ET.fromstring(xml_bytes)


def collect_commands_for_versions(root: ET.Element, features=FEATURES):
    """
    Collect all command names required by the specified OpenGL feature versions.
    """
    cmds = set()
    for feat in root.findall('feature'):
        name = feat.get('name')
        for req in feat.findall('require'):
            for cmd in req.findall('command'):
                fname = cmd.get('name')
                if fname:
                    if name in features:
                        FEATURE_COMMANDS.append(fname)
                    cmds.add(fname)
    return cmds


def generate_dispatch_table(cmds, h_path: str, cpp_path: str):
    """
    Generate C++ header and source files for a dispatch table.
    """

    with open(h_path, 'w') as h:
        h.write("// Auto-generated dispatch table for OpenGL\n")
        h.write("#pragma once\n")
        h.write("#include \"OpenGl32/Defines.hpp\"\n")
        h.write("#ifdef CCT_PLATFORM_WINDOWS\n")
        h.write('#include "OpenGl32/WglDispatchTable.hpp"\n')
        h.write("#endif // CCT_PLATFORM_WINDOWS\n")
        h.write("namespace glgpus {\n")
        h.write("void* GetFromDispatchTable(const char* name);\n")

        h.write("struct OpenGlDispatchTable\n{\n")
        for name, (param_name, param_type, ret_type) in cmds.items():
            parameters = []
            for ptype, pname in zip(param_type, param_name):
                parameters.append(f"{ptype} {pname}")
            h.write(f"    {ret_type} (*{name})({', '.join(parameters)});\n")
        h.write("};\n")

        h.write("#ifdef CCT_PLATFORM_WINDOWS\n")
        h.write("OpenGlDispatchTable WglDispatchTableToOpenGlDispatchTable(const WglDispatchTable& wglDispatchTable);\n")
        h.write("#endif // CCT_PLATFORM_WINDOWS\n")
        h.write("\n} // namespace glgpus\n\n")

    with open(cpp_path, 'w') as cpp:
        cpp.write("// Auto-generated dispatch table for OpenGL\n")
        cpp.write('#include "OpenGl32/OpenGlFunctions.hpp"\n')
        cpp.write('#include "OpenGl32/IcdLoader/IcdLoader.hpp"\n')
        cpp.write('#include "OpenGl32/DeviceContext/DeviceContext.hpp"\n')
        cpp.write('#ifdef CCT_PLATFORM_WINDOWS\n')
        cpp.write('#include "OpenGl32/IcdLibrary/Wgl/WglIcdLibrary.hpp"\n')
        cpp.write('#endif // CCT_PLATFORM_WINDOWS\n')
        for name, (param_name, param_type, ret_type) in cmds.items():
            parameters = []
            for ptype, pname in zip(param_type, param_name):
                parameters.append(f"{ptype} {pname}")
            default_ret_type = "" if ret_type == "void" else "{}"
            cpp.write("""
extern "C" GLGPUS_API {ret_type} CCT_CALL {name}({parameters})
{{
    GLGPUS_AUTO_PROFILER_SCOPE();
    auto* instance = glgpus::IcdLoader::Instance();
    if (!instance)
    {{
        CCT_ASSERT_FALSE("IcdLoader::Instance() returned nullptr");
        return {default_ret_type};
    }}
    if (!instance->GetIcd().IsLoaded())
    {{
        CCT_ASSERT_FALSE("Invalid ICD library");
        return {default_ret_type};
    }}

    auto* deviceContext = instance->GetCurrentDeviceContextForCurrentThread();
    if (!deviceContext)
    {{
        CCT_ASSERT_FALSE("Invalid device context");
        return {default_ret_type};
    }}
    if (deviceContext->DeviceContext)
        return deviceContext->DeviceContext->GetGlDispatchTable().{name}({param_name});
    CCT_ASSERT_FALSE("Invalid device context");
    return {default_ret_type};
}}
""".format(
                ret_type=ret_type,
                name=name,
                parameters=", ".join(parameters),
                param_name=", ".join(param_name),
                default_ret_type=default_ret_type,
            ))
        cpp.write("void* glgpus::GetFromDispatchTable(const char* name) {\n")
        cpp.write("\n")
        cpp.write(r"""
    auto* instance = glgpus::IcdLoader::Instance();
    if (!instance)
    {
        CCT_ASSERT_FALSE("IcdLoader::Instance() returned nullptr");
        return nullptr;
    }
    if (!instance->GetIcd().IsLoaded())
    {
        CCT_ASSERT_FALSE("Invalid ICD library");
        return nullptr;
    }

    auto* deviceContext = instance->GetCurrentDeviceContextForCurrentThread();
    if (!deviceContext)
    {
        CCT_ASSERT_FALSE("Invalid device context");
        return nullptr;
    }
""")
        for name, (param_name, param_type, ret_type) in cmds.items():
            cpp.write(
                f"    if (strcmp(name, \"{name}\") == 0)\n"
                f"        return (void*){name};\n"
            )
        cpp.write("    return nullptr;\n")
        cpp.write("}\n")

        cpp.write("#ifdef CCT_PLATFORM_WINDOWS\n")
        cpp.write("glgpus::OpenGlDispatchTable glgpus::WglDispatchTableToOpenGlDispatchTable(const glgpus::WglDispatchTable& wglDispatchTable)\n{\n")
    
        cpp.write("    OpenGlDispatchTable dispatchTable;\n")
        cpp.write("""    auto* glgpusInstance = glgpus::IcdLoader::Instance();
    if (glgpusInstance == nullptr)
    {
        CCT_ASSERT_FALSE("glgpusInstance is null");
        return {};
    }
""")
        for name, (param_name, param_type, ret_type) in cmds.items():
            parameters = []
            for ptype, pname in zip(param_type, param_name):
                parameters.append(f"{ptype} {pname}")
            if name in FEATURE_COMMANDS:
                cpp.write(f"    dispatchTable.{name} = wglDispatchTable.{name};\n")
            else:
                cpp.write(f"    dispatchTable.{name} = (decltype(dispatchTable.{name}))glgpusInstance->GetPlatformIcd<glgpus::WglIcdLibrary>().DrvGetProcAddress(\"{name}\");\n")
        cpp.write("    return dispatchTable;\n")
        cpp.write("}\n")

        cpp.write("#endif // CCT_PLATFORM_WINDOWS\n")
  

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Generate C++.")
    parser.add_argument('xml', help='Path to input xml file')
    parser.add_argument('hpp', help='Path to output hpp file')
    parser.add_argument('cpp', help='Path to output cpp file')
    args = parser.parse_args()

    if not os.path.exists(args.xml):  
        print(f"XML file {args.xml} does not exist.")
        return
    xml_data = None
    with open(args.xml, 'rb') as f:
        xml_data = f.read()
    root = parse_gl_xml(xml_data)
    cmds = collect_commands_for_versions(root)

    full_cmds = {}
    for cmd in root.findall('commands/command'):
        proto = cmd.find('proto')
        if proto is None:
            continue
        name = proto.find('name')
        if name is None:
            continue
        if name.text not in cmds:
            continue

        ptype = proto.find('ptype')
        const = proto.text if proto.text else ""
        ptr = (ptype.tail if ptype.tail else "") if ptype is not None else ""

        ret_type = (const + (ptype.text if ptype is not None else "") + ptr).strip()

        func_param_name = []
        func_param_type = []
        
        for param in cmd.findall('param'):
            ptype = param.find('ptype')
            pname = param.find('name')

            const = param.text if param.text else ""
            ptr = (ptype.tail if ptype.tail else "") if ptype is not None else ""

            if pname is not None:
                func_param_name.append(pname.text)
            func_param_type.append(const + (ptype.text if ptype is not None else "") + ptr)


        full_cmds[name.text] = (
                       func_param_name,
                       func_param_type,
                       ret_type)

    if not os.path.exists(args.cpp):
        os.makedirs(os.path.dirname(args.cpp), exist_ok=True)

    generate_dispatch_table(full_cmds, args.hpp, args.cpp)
    print(f"Generated dispatch table with {len(full_cmds)} functions.")


if __name__ == "__main__":
    main()
