#include "Util.h"
#include "Config.h"

namespace sese {

    [[maybe_unused]] static struct InitStruct {
        InitStruct() {
            // 初始化 Logger
            auto logger = sese::Singleton<Logger>::getInstance();
            auto formatter = std::make_shared<SimpleFormatter>();
            auto appender = std::make_shared<ConsoleAppender>(formatter);
            logger->addAppender(appender);
        }
        ~InitStruct() {
            // Logger 不需要手动释放
            // delete sese::Singleton<Logger>::getInstance();
        }
    } initStruct; /* NOLINT */

    Logger *getLogger() noexcept {
        return sese::Singleton<Logger>::getInstance();
    }

    bool isSpace(char ch) noexcept {
        auto p = SPACE_CHARS;
        while (*p != '\0') {
            if (*p == ch) {
                return true;
            }
            p++;
        }
        return false;
    }

}// namespace sese
