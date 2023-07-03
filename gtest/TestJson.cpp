#include "gtest/gtest.h"

#include "sese/config/json/JsonUtil.h"
#include "sese/util/ConsoleOutputStream.h"
#include "sese/util/FileStream.h"
#include "sese/util/InputBufferWrapper.h"
#include "sese/net/rpc/Marco.h"


/// 从文件解析 Json 格式
TEST(TestJson, Json) {
    auto fileStream = sese::FileStream::create(PROJECT_PATH "/test/TestJsonUtil/data.json", TEXT_READ_EXISTED);
    auto object = sese::json::JsonUtil::deserialize(fileStream, 3);
    ASSERT_TRUE(object != nullptr);

    EXPECT_EQ(object->getDataAs<sese::json::ObjectData>("A"), nullptr);
    EXPECT_EQ(object->getDataAs<sese::json::ArrayData>("A"), nullptr);
    EXPECT_EQ(object->getDataAs<sese::json::BasicData>("A"), nullptr);

    EXPECT_NE(object->getDataAs<sese::json::ObjectData>("object"), nullptr);
    EXPECT_NE(object->getDataAs<sese::json::ArrayData>("mixin_array"), nullptr);
    EXPECT_NE(object->getDataAs<sese::json::BasicData>("value"), nullptr);

    EXPECT_EQ(object->getDataAs<sese::json::ObjectData>("mixin_array"), nullptr);
    EXPECT_EQ(object->getDataAs<sese::json::ArrayData>("object"), nullptr);

    auto booleanValue = object->getDataAs<sese::json::BasicData>("boolean");
    ASSERT_TRUE(booleanValue != nullptr);
    ASSERT_TRUE(booleanValue->getDataAs<bool>(false));
    booleanValue->setDataAs<bool>(false);

    auto output = std::make_shared<sese::ConsoleOutputStream>();
    sese::json::JsonUtil::serialize(object, output);
    output->write("\n", 1);
}

TEST(TestJson, Getter) {
    const char str[] = "{\n"
                       "  \"str1\": \"\\b\\f\\t\\n\\r/\",\n"
                       "  \"str2\": \"Hello\",\n"
                       "  \"bool1\": true,\n"
                       "  \"bool2\": false,\n"
                       "  \"int\": 1,\n"
                       "  \"double\": 3.14\n"
                       "}";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_NE(object, nullptr);

    SESE_JSON_GET_STRING(str1, object, "str1", "undef");
    EXPECT_EQ(str1, "\b\f\t\n\r/");
    SESE_JSON_GET_STRING(str2, object, "str2", "undef");
    EXPECT_EQ(str2, "Hello");
    SESE_JSON_GET_BOOLEAN(bool1, object, "bool1", false);
    EXPECT_EQ(bool1, true);
    SESE_JSON_GET_BOOLEAN(bool2, object, "bool2", false);
    EXPECT_EQ(bool2, false);
    SESE_JSON_GET_INTEGER(int1, object, "int", 0);
    EXPECT_EQ(int1, 1);
    SESE_JSON_GET_DOUBLE(double1, object, "double", 0.0);
    EXPECT_EQ(double1, 3.14);

    SESE_JSON_GET_STRING(str0, object,"A", "undef");
    EXPECT_EQ(str0, "undef");
    SESE_JSON_GET_BOOLEAN(bool0, object, "A",  true);
    EXPECT_EQ(bool0, true);
    SESE_JSON_GET_DOUBLE(double0, object, "A", 0.0);
    EXPECT_EQ(double0, 0);
    SESE_JSON_GET_INTEGER(int0, object, "A", 0);
    EXPECT_EQ(int0, 0);

    EXPECT_EQ(object->getDataAs<sese::json::ObjectData>("A"), nullptr);
    EXPECT_EQ(object->getDataAs<sese::json::ArrayData>("A"), nullptr);
    EXPECT_EQ(object->getDataAs<sese::json::BasicData>("A"), nullptr);
}

TEST(TestJson, Setter) {
    auto object = std::make_shared<sese::json::ObjectData>();
    SESE_JSON_SET_STRING(object, "str1", "\b\f\t\n\r/");
    SESE_JSON_SET_INTEGER(object, "int", 10);
    SESE_JSON_SET_DOUBLE(object, "pi", 3.14);
    SESE_JSON_SET_BOOLEAN(object, "t1", true);
    SESE_JSON_SET_BOOLEAN(object, "t2", false);
    SESE_JSON_SET_NULL(object, "nullable");

    sese::ConsoleOutputStream console;
    sese::json::JsonUtil::serialize(object.get(), &console);
}

/// 键值对分割错误
TEST(TestJson, Error_0) {
    const char str[] = "{\n"
                       "  \"Hello\", \"World\"\n"
                       "}";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_EQ(object, nullptr);
}

/// 对象起始符错误
TEST(TestJson, Error_1) {
    const char str[] = "  \"Hello\": \"World\"\n"
                       "}";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_EQ(object, nullptr);
}

/// 逗号后不存在下一个对象
TEST(TestJson, Error_2) {
    const char str[] = "{\n"
                       "  \"Hello\": \"World\",\n"
                       "}";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_EQ(object, nullptr);
}

/// 字符串转义不完整
TEST(TestJson, Error_3) {
    const char str[] = "{\n"
                       "  \"Hello\": \"\\";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_EQ(object, nullptr);
}

/// tokens 为空
TEST(TestJson, Error_4) {
    const char str[] = "";
    auto input = sese::InputBufferWrapper(str, sizeof(str) - 1);
    auto object = sese::json::JsonUtil::deserialize(&input, 5);
    ASSERT_EQ(object, nullptr);
}