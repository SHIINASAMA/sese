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

#define SESE_C_LIKE_FORMAT

#include <sese/service/BalanceLoader.h>
#include <sese/system/Process.h>
#include <sese/record/Marco.h>
#include <sese/net/ws/WebsocketAuthenticator.h>

#include <random>

#include <gtest/gtest.h>

#define printf SESE_INFO

static sese::net::IPv4Address::Ptr createAddress() {
    auto port = sese::net::createRandomPort();
    printf("select port %d", (int) port);
    return sese::net::IPv4Address::create("127.0.0.1", port);
}

TEST(TestWebsocket, Auth_0) {
    auto key = "dGhlIHNhbXBsZSBub25jZQ==";
    auto result = sese::net::ws::WebsocketAuthenticator::toResult(key);
    EXPECT_EQ(strcmp("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", result.get()), 0);
}

TEST(TestWebsocket, Auth_1) {
    auto pair = sese::net::ws::WebsocketAuthenticator::generateKeyPair();
    EXPECT_TRUE(sese::net::ws::WebsocketAuthenticator::verify(pair.first.get(), pair.second.get()));
}