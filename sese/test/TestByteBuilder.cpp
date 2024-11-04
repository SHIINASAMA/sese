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

#include "sese/io/ByteBuffer.h"
#include "sese/io/ByteBuilder.h"
#include <sese/record/LogHelper.h>

#include <gtest/gtest.h>

TEST(TestByteBuilder, Read) {
    auto builder = sese::io::ByteBuffer(16);
    ASSERT_EQ(builder.getCapacity(), 16);

    {
        // node1:
        //   "Hello World, sese"
        // node2:
        //   "\'s Byte Builder"
        const char STR[] = {"Hello World, sese's Byte Builder"};
        auto len = builder.write(STR, sizeof(STR) - 1);
        ASSERT_EQ(len, sizeof(STR) - 1);
    }
    {
        // read a part of the first node
        char str[32]{};
        auto len = builder.read(str, 10);
        ASSERT_EQ(len, 10);
        ASSERT_EQ(std::string_view(str), std::string_view("Hello Worl"));
    }
    {
        // read data between nodes
        // read latest of the first node and all the second node
        char str[32]{};
        auto len = builder.read(str, sizeof(str) - 1);
        ASSERT_EQ(len, 22);
        ASSERT_EQ(std::string_view(str), std::string_view("d, sese's Byte Builder"));
    }
    {
        // fill data
        char buffer1[1024];
        char buffer2[1024];
        memset(buffer1, 'A', 1024);
        memset(buffer2, 'B', 1024);
        auto len = builder.write(buffer1, 1024);
        ASSERT_EQ(len, 1024);
        len = builder.write(buffer2, 1024);
        ASSERT_EQ(len, 1024);
    }
    {
        // read data cross multi nodes
        char buffer[2048];
        auto len = builder.read(buffer, 4096);
        ASSERT_EQ(len, 2048);
    }

    builder.freeCapacity();
}

TEST(TestByteBuilder, Peek) {
    auto builder = sese::io::ByteBuffer(16);
    ASSERT_EQ(builder.getCapacity(), 16);

    {
        // node1:
        //   "Hello World, sese"
        // node2:
        //   "\'s Byte Builder"
        const char STR[] = {"Hello World, sese's Byte Builder"};
        auto len = builder.write(STR, sizeof(STR) - 1);
        ASSERT_EQ(len, sizeof(STR) - 1);
    }
    {
        // read a part of the first node
        char str[32]{};
        auto len = builder.peek(str, 10);
        ASSERT_EQ(len, 10);
        builder.trunc(len);
        ASSERT_EQ(std::string_view(str), std::string_view("Hello Worl"));
    }
    {
        // read data between nodes
        // read latest of the first node and all the second node
        char str[32]{};
        auto len = builder.peek(str, sizeof(str) - 1);
        ASSERT_EQ(len, 22);
        builder.trunc(len);
        ASSERT_EQ(std::string_view(str), std::string_view("d, sese's Byte Builder"));
    }
    {
        // fill data
        char buffer1[1024];
        char buffer2[1024];
        memset(buffer1, 'A', 1024);
        memset(buffer2, 'B', 1024);
        auto len = builder.write(buffer1, 1024);
        ASSERT_EQ(len, 1024);
        len = builder.write(buffer2, 1024);
        ASSERT_EQ(len, 1024);
    }
    {
        // read data cross multi nodes
        char buffer[2048];
        auto len = builder.peek(buffer, 4096);
        ASSERT_EQ(len, 2048);
        builder.trunc(len);
    }

    builder.freeCapacity();
}

TEST(TestByteBuilder, Misc_0) {
    auto builder = sese::io::ByteBuffer();
    builder.write("Hello, World", 12);

    ASSERT_EQ(builder.getCapacity(), 1024);
    // ASSERT_EQ(builder.getCurrentReadPos(), 0);
    // ASSERT_EQ(builder.getCurrentWritePos(), 12);
    ASSERT_EQ(builder.getLength(), 12);

    builder.trunc(12);
    ASSERT_EQ(builder.getCapacity(), 1024);
    // ASSERT_EQ(builder.getCurrentReadPos(), 12);
    // ASSERT_EQ(builder.getCurrentWritePos(), 12);
    ASSERT_EQ(builder.getLength(), 12);

    builder.resetPos();
    ASSERT_EQ(builder.getCapacity(), 1024);
    // ASSERT_EQ(builder.getCurrentReadPos(), 0);
    // ASSERT_EQ(builder.getCurrentWritePos(), 12);
    ASSERT_EQ(builder.getLength(), 12);
}

TEST(TestByteBuilder, Misc_1) {
    auto builder = sese::io::ByteBuilder(6);
    builder.write("Hello, World", 12);

    // deep copy
    auto i = builder;
    ASSERT_EQ(i.trunc(12), 12);
    // ASSERT_EQ(i.getCurrentReadPos(), 6);
    // ASSERT_EQ(builder.getCurrentReadPos(), 0);

    // move copy
    auto k = std::move(builder);
    // ASSERT_EQ(k.getCurrentReadPos(), 0);
    // builder will be unusable
    ASSERT_EQ(k.trunc(12), 12);
}

TEST(TestByteBuilder, Misc_2) {
    auto builder = sese::io::ByteBuilder(6);
    builder.write("Hello, World", 12);
    builder.trunc(7);

    auto i = builder;
    // ASSERT_EQ(i.getCurrentReadPos(), 1);
    ASSERT_EQ(i.trunc(32), 5);
}