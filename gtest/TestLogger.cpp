#include "sese/record/LogHelper.h"
#include "sese/record/BlockAppender.h"
#include "sese/record/FileAppender.h"
#include "sese/record/SimpleFormatter.h"

#include "gtest/gtest.h"

using namespace std::chrono_literals;

TEST(TestLogger, BlockAppender) {
    sese::record::LogHelper log("BlockAppender");

    auto appender = std::make_shared<sese::record::BlockAppender>(1 * 1024 * 20, sese::record::Level::INFO);
    sese::record::Logger::addGlobalLoggerAppender(appender);

    log.debug("no display");

    for (auto i = 0; i < 640; i++) {
        log.info("No.%d log message", i);
    }

    sese::record::Logger::removeGlobalLoggerAppender(appender);
}

TEST(TestLogger, Logger) {
    sese::record::LogHelper log("Logger");

    log.debug("Hello");
    log.info("Hello");
    log.warn("Hello");
    log.error("Hello");
}

TEST(TestLogger, StaticMethod) {
    using sese::record::LogHelper;
    LogHelper::d("Hello");
    LogHelper::i("Hello");
    LogHelper::w("Hello");
    LogHelper::e("Hello");
}

TEST(TestLogger, FileAppender) {
    sese::record::LogHelper log("FileAppender");

    auto logger = sese::record::getLogger();
    auto fileStream = sese::FileStream::create("hello.log", TEXT_WRITE_CREATE_TRUNC);
    ASSERT_TRUE(fileStream != nullptr);
    auto fileAppender = std::make_shared<sese::record::FileAppender>(fileStream);
    logger->addAppender(fileAppender);

    log.debug("Hello");
    log.info("Hello");
    log.warn("Hello");
    log.error("Hello");
}

TEST(TestLogger, SimpleFormat) {
    auto event = std::make_shared<sese::record::Event>(
            sese::DateTime::now(),
            sese::record::Level::INFO,
            "ThreadName",
            0,
            __FILE__,
            __LINE__,
            "Hello"
    );


    auto format1 = sese::record::SimpleFormatter("c");
    sese::record::LogHelper::d(format1.dump(event).c_str());

    auto format2 = sese::record::SimpleFormatter("li lv la");
    sese::record::LogHelper::d(format2.dump(event).c_str());

    auto format3 = sese::record::SimpleFormatter("fn fi fa");
    sese::record::LogHelper::d(format3.dump(event).c_str());

    auto format4 = sese::record::SimpleFormatter("th tn ta");
    sese::record::LogHelper::d(format4.dump(event).c_str());

    auto format5 = sese::record::SimpleFormatter("%m");
    sese::record::LogHelper::d(format5.dump(event).c_str());
}