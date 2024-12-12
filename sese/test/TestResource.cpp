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

#include <sese/res/Marco.h>
#include <gtest/gtest.h>
#include <set>

SESE_DEF_RES_MANAGER(TestResource)
SESE_ADD_RES("hello.txt", strlen("Hello"), "Hello")
SESE_ADD_RES("world.txt", strlen("World"), "World")
SESE_DEF_RES_MANAGER_END(TestResource)

TEST(TestResource, Manager) {
    // Get the Resource Manager instance
    auto instance = TestResourceInstance::getInstance();
    EXPECT_NE(instance, nullptr);

    // Obtain a resource instance
    auto res = instance->getResource("hello.txt");
    EXPECT_NE(res, nullptr);

    EXPECT_EQ(instance->getResource("hello"), nullptr);

    // Get the resource flow
    auto stream = res->getStream();
    EXPECT_NE(stream, nullptr);

    char buffer[16]{};
    EXPECT_EQ(stream->read(buffer, stream->getSize()), 5);
    EXPECT_EQ(std::string_view(buffer), "Hello");
}

TEST(TestResource, Foreach) {
    auto instance = TestResourceInstance ::getInstance();
    EXPECT_NE(instance, nullptr);

    std::set<std::string> names{"hello.txt", "world.txt"};

    for (decltype(auto) item: *instance) {
        EXPECT_TRUE(names.contains(item.first));
    }

    for (decltype(auto) item = instance->begin(); item != instance->end(); item++) {
        EXPECT_TRUE(names.contains(item->first));
    }
}

TEST(TestResource, Read) {
    auto instance = TestResourceInstance::getInstance();
    auto res = instance->getResource("hello.txt");
    auto stream = res->getStream();
    char buffer[32]{};
    EXPECT_EQ(stream->read(buffer, sizeof(buffer)), stream->getSize());
    EXPECT_EQ(stream->read(buffer, sizeof(buffer)), 0);
}

TEST(TestResource, Peek) {
    auto instance = TestResourceInstance::getInstance();
    auto res = instance->getResource("hello.txt");
    auto stream = res->getStream();
    char buffer[32]{};
    EXPECT_EQ(stream->peek(buffer, sizeof(buffer)), stream->getSize());
    EXPECT_EQ(stream->trunc(stream->getSize()), stream->getSize());
    EXPECT_EQ(stream->peek(buffer, sizeof(buffer)), 0);
    EXPECT_EQ(stream->trunc(stream->getSize()), 0);
}

TEST(TestResource, Seek) {
    auto instance = TestResourceInstance::getInstance();
    auto res = instance->getResource("hello.txt");
    auto stream = res->getStream();
    char buffer[32]{};

    EXPECT_EQ(stream->setSeek(1, sese::io::Seek::CUR), 0);
    EXPECT_EQ(stream->peek(buffer, sizeof(buffer)), stream->getSize() - 1);
    EXPECT_EQ(stream->getSeek(), 1);

    EXPECT_EQ(stream->setSeek(2, sese::io::Seek::BEGIN), 0);
    EXPECT_EQ(stream->peek(buffer, sizeof(buffer)), stream->getSize() - 2);
    EXPECT_EQ(stream->getSeek(), 2);

    EXPECT_EQ(stream->setSeek(-3, sese::io::Seek::END), 0);
    EXPECT_EQ(stream->peek(buffer, sizeof(buffer)), 3);
    EXPECT_EQ(stream->getSeek(), stream->getSize() - 3);

    EXPECT_EQ(stream->setSeek(-1, sese::io::Seek::BEGIN), -1);

    EXPECT_EQ(stream->setSeek(0, 114514), -1);
}