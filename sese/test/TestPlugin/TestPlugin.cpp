#include "sese/plugin/Module.h"
#include "gtest/gtest.h"

class Printable : public sese::plugin::BaseClass {
public:
    virtual void run() = 0;
};

TEST(TestPlugin, SUCCESS) {
    auto m = sese::plugin::Module::open(PATH_TO_MODULE);
    ASSERT_NE(m, nullptr);
    printf("module info:\n"
           "\tname: %s\n"
           "\tversion: %s\n"
           "\tdescription: %s\n",
           m->getName(),
           m->getVersion(),
           m->getDescription());

    printf("register class:\n");
    printf("\tname%s & raw name\n", "");
    auto info_map = m->getRegisterClassInfo();
    for (const auto &info: info_map) {
        printf("\t%s %s\n", info.first.c_str(), info.second.info->name());
    }

    printf("execute plugin code:\n");
    auto p1 = m->createClassAs<Printable>("M::Bye");
    auto p2 = m->createClassAs<Printable>("M::Hello");
    auto p3 = m->createClassAs<Printable>("M::Undef");
    p1->run();
    p2->run();
    ASSERT_EQ(p3, nullptr);
}

// 文件不存在
TEST(TestPlugin, Error_0) {
    auto m = sese::plugin::Module::open("NullModule");
    ASSERT_EQ(m, nullptr);
}

// 文件存在但不包含模块信息
TEST(TestPlugin, Error_1) {
    auto m = sese::plugin::Module::open(PATH_TO_CORE);
    ASSERT_EQ(m, nullptr);
}