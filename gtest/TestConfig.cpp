#include "sese/record/LogHelper.h"
#include "sese/config/ConfigUtil.h"
#include "sese/config/xml/XmlUtil.h"
#include "sese/config/json/JsonUtil.h"
#include "sese/util/ConsoleOutputStream.h"
#include "sese/util/FileStream.h"
#include "gtest/gtest.h"

TEST(TestConfig, Config) {
    sese::record::LogHelper log("Config");

    auto file = sese::FileStream::create(PROJECT_PATH "/gtest/Data/data.ini", TEXT_READ_EXISTED);
    ASSERT_TRUE(file != nullptr);

    auto config = sese::ConfigUtil::readFrom(file.get());
    ASSERT_TRUE(config != nullptr);

    auto defaultSection = config->getDefaultSection();
    ASSERT_TRUE(defaultSection != nullptr);
    auto defaultName = defaultSection->getValueByKey("name", "unknown");
    log.info("[default] name = %s", defaultName.c_str());

    auto serverSection = config->getSectionByName("server");
    ASSERT_TRUE(serverSection != nullptr);
    log.info("[server] address = %s", serverSection->getValueByKey("address", "192.168.1.1").c_str());
    log.info("[server] port = %s", serverSection->getValueByKey("port", "8080").c_str());

    auto clientSection = config->getSectionByName("client");
    ASSERT_TRUE(clientSection == nullptr);
}

TEST(TestConfig, Json) {
    auto file = sese::FileStream::create(PROJECT_PATH "/gtest/Data/data.json", TEXT_READ_EXISTED);
    ASSERT_TRUE(file != nullptr);
    auto object = sese::json::JsonUtil::deserialize(file, 3);
    ASSERT_TRUE(object != nullptr);

    auto booleanValue = object->getDataAs<sese::json::BasicData>("boolean");
    ASSERT_TRUE(booleanValue != nullptr);
    ASSERT_TRUE(booleanValue->getDataAs<bool>(false));
    booleanValue->setDataAs<bool>(false);

    auto out = std::make_shared<sese::ConsoleOutputStream>();
    sese::json::JsonUtil::serialize(object, out);
    out->write("\n", 1);
}

TEST(TestConfig, Xml) {
    auto file = sese::FileStream::create(PROJECT_PATH "/gtest/Data/data.xml", BINARY_READ_EXISTED);
    ASSERT_TRUE(file != nullptr);
    auto element = sese::xml::XmlUtil::deserialize(file, 5);
    ASSERT_TRUE(element != nullptr);

    auto subElement = std::make_shared<sese::xml::Element>("new-element");
    subElement->setAttribute("info", "from serialize");
    element->elements.push_back(subElement);

    // auto saveFile = sese::FileStream::create("out.xml", BINARY_WRITE_CREATE_TRUNC);
    // ASSERT_TRUE(saveFile != nullptr);
    auto out = std::make_shared<sese::ConsoleOutputStream>();
    sese::xml::XmlUtil::serialize(element, out);
    out->write("\n", 1);
}