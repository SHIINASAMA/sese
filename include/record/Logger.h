/**
 * @file Logger.h
 * @author kaoru
 * @date 2022年3月28日
 * @brief 日志输出类
 */
#pragma once
#include "AbstractAppender.h"
#include "Config.h"
#include <memory>
#include <vector>

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

namespace sese {
    /**
     * @brief 日志输出类
     */
    class API Logger {
    public:
        /// 智能指针
        typedef std::shared_ptr<Logger> Ptr;

        /**
         * 初始化
         * @param name 日志器名称
         */
        explicit Logger(const char *name = RECORD_MASTER_LOGGER_NAME) noexcept {
            this->name = name;
        }

        /**
         * 添加日志输出源
         * @param appender 日志输出源
         */
        void addAppender(const AbstractAppender::Ptr &appender) noexcept;

        /**
         * 输出日志
         * @param event 日志事件
         */
        void log(const Event::Ptr &event) const noexcept;

    private:
        const char *name = nullptr;
        std::vector<AbstractAppender::Ptr> appenders;
    };
}// namespace sese