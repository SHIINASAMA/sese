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

#include <gtest/gtest.h>

#include <sese/text/Format.h>
#include <sese/Log.h>

#include <map>

using namespace sese::text;

using MyMap = std::map<std::string, int>;
using MyMapPair = std::pair<const std::string, int>;

struct Point {
    int x;
    int y;
};

namespace sese::text::overload {
template<>
struct Formatter<Point> {
    static void parse(const std::string &args) {
        SESE_INFO("args {}", args);
    }
    static std::string format(const Point &p) {
        return fmt("({},{})", p.x, p.y);
    }
};

template<>
struct Formatter<MyMapPair> {
    void parse(const std::string &) {}
    static std::string format(MyMapPair &pair) {
        return "{" + pair.first + "," + std::to_string(pair.second) + "}";
    }
};
} // namespace sese::text::overload

TEST(TestFormat, Simple) {
    int a = 1, b = 2;
    std::string c = "Hello";
    double pi = 3.14159265;
    EXPECT_EQ("1, Hello, 2, World", fmt("{}, Hello, {}, World", a, b));
    EXPECT_EQ("{} Hello", fmt("\\{} {}", c));
    EXPECT_EQ("Hello", fmt("Hello"));
    EXPECT_EQ("World", fmt("{ Hello }", "World"));
    EXPECT_EQ("World", fmt("{ Hello \\}}", "World"));
    EXPECT_EQ("0x1 + 02 = b11", fmt("0x{h} + 0{o} = b{b}", a, b, a + b));
    EXPECT_EQ("Pi 3.14", fmt("Pi {2}", pi));
}

TEST(TestFormat, Log) {
    SESE_DEBUG("Hello {} {}", 123, "World");
    SESE_INFO("Hello {}", "World");
    SESE_WARN("Hello {}");
    SESE_ERROR("Hello \\}\\{", 3.1415);
    SESE_RAW("Hello\n", 6);
}

TEST(TestFormat, Formatter) {
    Point point{1, 2};
    SESE_INFO(R"(\{{\}123\}ABC\}})", point);
}

#include <sese/util/DateTime.h>

TEST(TestFormat, Parse) {
    auto datetime = sese::DateTime::now();
    SESE_INFO("{} | {HH:mm:ss}", datetime, datetime);
}

TEST(TestFormat, Constexpr) {
    constexpr auto PATTERN = "{} {}";
    EXPECT_EQ(2, FormatParameterCounter(PATTERN));
}

TEST(TestFormat, Iterable) {
    auto array = std::array<int, 3>({1, 2, 3});
    EXPECT_EQ("[1, 2, 3]",fmt("{}", array));

    MyMap map;
    map["abc"] = 114;
    map["efg"] = 514;
    EXPECT_EQ("[{abc,114}, {efg,514}]" ,fmt("{}", map));

    SESE_INFO(fmt("{AB}|{CD}", map, array));
}