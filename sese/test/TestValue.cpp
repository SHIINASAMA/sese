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

#include <sese/util/Value.h>
#include <sese/Log.h>

#include <gtest/gtest.h>

using sese::Value;

TEST(TestValue, Contruct) {
    EXPECT_TRUE(Value(Value::Type::NONE).isNull());
    EXPECT_TRUE(Value(Value::Type::BOOL).isBool());
    EXPECT_TRUE(Value(Value::Type::INT).isInt());
    EXPECT_TRUE(Value(Value::Type::DOUBLE).isDouble());
    EXPECT_TRUE(Value(Value::Type::STRING).isString());
    EXPECT_TRUE(Value(Value::Type::BLOB).isBlob());
    EXPECT_TRUE(Value(Value::Type::LIST).isList());
    EXPECT_TRUE(Value(Value::Type::DICT).isDict());
}

TEST(TestValue, IdentityNull) {
    Value value;
    EXPECT_TRUE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());
}

TEST(TestValue, IdentityBool) {
    Value value(true);
    EXPECT_FALSE(value.isNull());
    EXPECT_TRUE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());

    EXPECT_EQ(value.getIfBool().value(), true);
    EXPECT_FALSE(value.getIfInt().has_value());

    EXPECT_EQ(value.getBool(), true);
}

TEST(TestValue, IdentityInt) {
    Value value(INT64_C(1));
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_TRUE(value.isInt());
    EXPECT_FALSE(value.isDouble());

    EXPECT_EQ(value.getIfInt().value(), 1);

    EXPECT_EQ(value.getInt(), 1);
}

TEST(TestValue, IdentityDouble) {
    Value value(1.0);
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_TRUE(value.isDouble());

    EXPECT_EQ(value.getIfDouble().value(), 1.0);

    EXPECT_EQ(value.getDouble(), 1.0);
}

TEST(TestValue, IdentityString) {
    Value value("test");
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());
    EXPECT_TRUE(value.isString());

    EXPECT_EQ(*value.getIfString(), "test");
    EXPECT_FALSE(value.getIfBool().has_value());
    EXPECT_FALSE(value.getIfDouble().has_value());
    EXPECT_EQ(value.getIfList(), nullptr);
    EXPECT_EQ(value.getIfDict(), nullptr);

    EXPECT_FALSE(value.getString().empty());

    value.getString().append("test");
    EXPECT_EQ(*value.getIfString(), "testtest");
}

TEST(TestValue, IdentityBlob) {
    Value value("test", 4);
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());
    EXPECT_TRUE(value.isBlob());

    EXPECT_TRUE(value.getIfBlob());

    auto data = value.getBlob();
    EXPECT_EQ(memcmp(data.data(), "test", data.size()), 0);

    Value::Blob blob;
    blob.emplace_back(1);
    blob.emplace_back(2);
    EXPECT_TRUE(Value(std::move(blob)).isBlob());

    value.getBlob().clear();
}

TEST(TestValue, IdentityList) {
    auto value = Value::list();
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());
    EXPECT_TRUE(value.isList());

    EXPECT_TRUE(value.getList().empty());
    value.getList().clear();
}

TEST(TestValue, IdentityDict) {
    auto value = Value::dict();
    EXPECT_FALSE(value.isNull());
    EXPECT_FALSE(value.isBool());
    EXPECT_FALSE(value.isInt());
    EXPECT_FALSE(value.isDouble());
    EXPECT_TRUE(value.isDict());

    EXPECT_TRUE(value.getDict().empty());
    value.getDict().clear();
}

TEST(TestValue, ConvertInt) {
    Value value(INT64_C(1));
    EXPECT_EQ(value.getIfInt().value(), 1);
    EXPECT_EQ(value.getIfBool().value(), true);
    EXPECT_EQ(value.getIfDouble().value(), 1.0);

    value = Value(INT64_C(0));
    EXPECT_EQ(value.getIfInt().value(), 0);
    EXPECT_EQ(value.getIfBool().value(), false);
    EXPECT_EQ(value.getIfDouble().value(), 0.0);
}

TEST(TestValue, ConvertDouble) {
    Value value(1.0);
    EXPECT_EQ(value.getIfInt().value(), 1);
    EXPECT_EQ(value.getIfBool().value(), true);
    EXPECT_EQ(value.getIfDouble().value(), 1.0);

    value = Value(0.0);
    EXPECT_EQ(value.getIfInt().value(), 0);
    EXPECT_EQ(value.getIfBool().value(), false);
    EXPECT_EQ(value.getIfDouble().value(), 0.0);
}

TEST(TestValue, CompareBool) {
    Value value1(true);
    Value value2(true);
    EXPECT_EQ(value1, value2);

    Value value3(false);
    EXPECT_NE(value1, value3);
}

TEST(TestValue, CompareInt) {
    Value value1(INT64_C(1));
    Value value2(INT64_C(1));
    EXPECT_EQ(value1, value2);

    Value value3(INT64_C(2));
    EXPECT_NE(value1, value3);
}

TEST(TestValue, CompareString) {
    Value value1("test");
    Value value2("test");
    EXPECT_EQ(value1, value2);

    Value value3("test2");
    EXPECT_NE(value1, value3);
}

TEST(TestValue, CompareBlob) {
    Value value1("test", 4);
    Value value2("test", 4);
    EXPECT_NE(value1, value2);
}

TEST(TestValue, CompareList) {
    // clang-format off
    Value value1(Value::List()
        .append(INT64_C(1))
        .append(INT64_C(2))
    );
    Value value2(Value::List()
        .append(INT64_C(1))
        .append(INT64_C(2))
    );
    // clang-format on
    EXPECT_NE(value1, value2);
}

TEST(TestValue, CompareDict) {
    // clang-format off
    Value value1(Value::Dict()
        .set("a", INT64_C(1))
        .set("b", INT64_C(2))
    );
    Value value2(Value::Dict()
        .set("a", INT64_C(1))
        .set("b", INT64_C(2))
    );
    EXPECT_NE(value1, value2);
    // clang-format on
}

TEST(TestValue, ToString) {
    // clang-format off
    Value value(Value::Dict()
        .set("list", Value::List()
            .append("Hello").append(Value())
            .append(true)
            .append(false)
            .append(INT64_C(114514))
            .append("Blob", 4)
            .append(Value::Dict())
            .append(Value::List()))
        .set("int", INT64_C(1919810))
        .set("double", 3.14)
        .set("dict", Value::Dict()
            .set("string", "World")
            .set("int", INT64_C(123456)))
    );
    // clang-format on
    SESE_INFO("\n{}", value);
}