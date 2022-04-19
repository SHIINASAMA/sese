#include "Util.h"
#include "record/FileAppender.h"
#include "record/LogHelper.h"
#include "record/SimpleFormatter.h"

sese::LogHelper helper("fLOGGER"); // NOLINT

int main() {
    auto logger = sese::getLogger();
    auto formatter = std::make_shared<sese::SimpleFormatter>();
    auto fileAppender = std::make_shared<sese::FileAppender>("hello.log", formatter);
    logger->addAppender(fileAppender);

    helper.debug("Hello 你好 こんにちは");
    helper.info("Hello 你好 こんにちは");
    helper.warn("Hello 你好 こんにちは");
    helper.error("Hello 你好 こんにちは");
    return 0;
}