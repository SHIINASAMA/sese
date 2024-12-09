// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sese/Enter.h>
#include <sese/Log.h>
#include <sese/system/CommandLine.h>
#include <sese/util/ArgParser.h>
#include <sese/util/Exception.h>
#include <sese/config/Json.h>
#include <sese/io/File.h>
#include <sese/text/StringBuilder.h>

#include <string>
#include <map>

class Bundler {
public:
    static int main() {
        try {
            Bundler bundler;
            bundler.parse_config();
            bundler.parse_resource_file();
            bundler.write_rc_file();
            bundler.write_rc_header_file();
        } catch (sese::Exception &e) {
            e.printStacktrace();
        }
        return 0;
    }

private:
    std::string base_path;
    std::string resource_file_path;
    std::string generate_code_path;

    std::string class_name;
    std::map<std::string, std::string> strings;
    std::map<std::string, std::string> binaries;

    void parse_config() {
        sese::ArgParser parser;
        if (!parser.parse(sese::system::CommandLine::getArgc(), sese::system::CommandLine::getArgv())) {
            throw sese::Exception("Failed to parse command line arguments");
        }

        if (!parser.exist("--base_path")) {
            throw sese::Exception("`base_path` not found in command line arguments");
        }
        base_path = parser.getValueByKey("--base_path");
        SESE_DEBUG("base_path = {}", base_path);

        if (!parser.exist("--resource_file_path")) {
            throw sese::Exception("`resource_file_path` not found in command line arguments");
        }
        resource_file_path = parser.getValueByKey("--resource_file_path");
        SESE_DEBUG("resource_file_path = {}", resource_file_path);

        if (!parser.exist("--generate_code_path")) {
            throw sese::Exception("`generate_code_path` not found in command line arguments");
        }
        generate_code_path = parser.getValueByKey("--generate_code_path");
        SESE_DEBUG("generate_code_path = {}", generate_code_path);
    }

    void parse_resource_file() {
        auto result = sese::io::File::createEx(resource_file_path, "rt");
        if (result) {
            throw sese::Exception("Failed to open resource file");
        }
        auto value = sese::Json::parse(result.get().get(), 4);
        auto &root = value.getDict();
        class_name = root.find("class")->getString();
        auto &strings = root.find("strings")->getDict();
        auto &binaries = root.find("binaries")->getDict();
        for (auto &[k, v]: strings) {
            auto str = v->getString();
            this->strings[k] = str;
            SESE_DEBUG("strings[{}] = {}", k, str);
        }
        for (auto &[k, v]: binaries) {
            auto bin = v->getString();
            this->binaries[k] = bin;
            SESE_DEBUG("binaries[{}] = {}", k, bin);
        }
    }

    void write_rc_file() {
        auto result = sese::io::File::createEx(generate_code_path + "/" + class_name + ".rc", "wt");
        if (result) {
            throw sese::Exception("Failed to open RC file to write");
        }
        auto file = result.get();
        sese::text::StringBuilder builder(1024);
        int index = 0;
        for (auto &[k, v]: binaries) {
            index += 1;
            builder << std::to_string(index) << " ";
            builder << k << " ";
            builder << "\"" << v << "\"";
            builder << "\n";
            auto string = builder.toString();
            builder.clear();
            file->write(string.data(), string.length());
        }
        file->write("\n", 1);

        file->write("STRINGTABLE\nBEGIN\n", 18);
        for (auto &[k, v]: strings) {
            index += 1;
            builder << "\t" << std::to_string(index) << " ";
            builder << k << " ";
            builder << "\"" << v << "\"";
            builder << "\n";
            auto string = builder.toString();
            builder.clear();
            file->write(string.data(), string.length());
        }
        file->write("END", 3);

        file->close();
    }

    void write_rc_header_file() {
        auto result = sese::io::File::createEx(generate_code_path + "/" + class_name + ".h", "wt");
        if (result) {
            throw sese::Exception("Failed to open RC file to write");
        }
        auto file = result.get();

        std::string str1 =
            "// Please do not modify this file manually\n"
            "// This file is generated by Bundler\n"
            "#include <map>\n"
            "#include <string>\n\n"
            "class ";
        std::string str2 =
            " {\n"
            "public: \n"
            "\tstatic std::map<std::string, int> strings;\n"
            "\tstatic std::map<std::string, int> binaries;\n"
            "};\n";
        std::string str3 = "std::map<std::string, int> Resource::binaries {\n";
        std::string str4 = "std::map<std::string, int> Resource::strings {\n";

        file->write(str1.data(), str1.length());
        file->write(class_name.data(), class_name.length());
        file->write(str2.data(), str2.length());

        sese::text::StringBuilder builder(1024);
        int index = 0;
        file->write(str3.data(), str3.length());
        bool first = true;
        for (auto &[k, _]: binaries) {
            index += 1;
            if (!first) {
                builder << ", ";
            }
            first = false;
            builder << "{\"" << k << "\", " << std::to_string(index) << "}\n";
            auto string = builder.toString();
            builder.clear();
            file->write(string.data(), string.length());
        }
        file->write("};\n", 3);

        file->write(str4.data(), str4.length());
        first = true;
        for (auto &[k, _]: strings) {
            index += 1;
            if (!first) {
                builder << ", ";
            }
            first = false;
            builder << "{\"" << k << "\", " << std::to_string(index) << "}\n";
            auto string = builder.toString();
            builder.clear();
            file->write(string.data(), string.length());
        }
        file->write("};\n", 3);
    }
};
SESE_ENTER(Bundler::main)
