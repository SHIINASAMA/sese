#include "Bundler.h"
#include "sese/util/Exception.h"
#include "sese/io/File.h"

#include <queue>

// ref: https://gareus.org/wiki/embedding_resources_in_executables
//      https://discourse.cmake.org/t/how-to-embed-info-plist-in-a-simple-mac-binary-file/512
void Bundler::write_mach_o_header_file() {
    auto result = sese::io::File::createEx(generate_code_path + "/" + class_name + ".h", "wt");
    if (result) {
        throw sese::Exception("Failed to open Mach-O header file to write");
    }
    auto file = result.get();

    std::string header =
        "// Please do not modify this file manually\n"
        "// This file is generated by Bundler\n\n"
        "#pragma once\n\n";
    std::string str1 = "class ";
    std::string str2 =
        " {\n"
        "public: \n"
        "enum class Binaries : int {\n";
    std::string str3 =
        "};\n"
        "static const char *syms[";
    std::string str4 =
        "];\n"
        "};\n"
        "const char *";
    std::string str5 =
        "::syms[";
    std::string str6 =
        "] {\n";
    std::string str7 =
        "};";

    int index = 0;
    sese::text::StringBuilder builder(1024);

    file->write(header.data(), header.length());
    file->write(str1.data(), str1.length());
    file->write(class_name.data(), class_name.length());
    file->write(str2.data(), str2.length());
    for (auto &[k, _] : binaries) {
        builder << k << " = " << std::to_string(index) << ",\n";
        index += 1;
        auto str = builder.toString();
        builder.clear();
        file->write(str.data(), str.length());
    }
    file->write(str3.data(), str3.length());
    auto count = std::to_string(binaries.size());
    file->write(count.data(), count.length());
    file->write(str4.data(), str4.length());
    file->write(class_name.data(), class_name.length());
    file->write(str5.data(), str5.length());
    file->write(count.data(), count.length());
    file->write(str6.data(), str6.length());
    for (auto &[k, _] : binaries) {
        builder << "\"" << k << "\"\n,";
        auto str = builder.toString();
        builder.clear();
        file->write(str.data(), str.length());
    }
    file->write(str7.data(), str7.length());
}
