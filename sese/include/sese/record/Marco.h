#pragma once

#include <sese/record/Logger.h>
#include <sese/thread/Thread.h>

#ifdef WIN32
#define sprintf sprintf_s
#endif

#ifdef __APPLE__
#define sprintf(buf, format, ...) snprintf(buf, sizeof(buf), format, ##__VA_ARGS__)
#endif

#define __SESE_LOG(point_to_logger, level, format, ...)              \
    {                                                                \
        char sese_tmp_buf[RECORD_OUTPUT_BUFFER]{0};                  \
        sprintf(sese_tmp_buf, format, ##__VA_ARGS__);                \
        auto sese_tmp_time = sese::DateTime::now();                  \
        auto sese_tmp_event = std::make_shared<sese::record::Event>( \
                sese_tmp_time,                                       \
                level,                                               \
                sese::Thread::getCurrentThreadName(),                \
                sese::Thread::getCurrentThreadId(),                  \
                SESE_FILENAME,                                       \
                __LINE__,                                            \
                sese_tmp_buf                                         \
        );                                                           \
        point_to_logger->log(sese_tmp_event);                        \
    }

#define __SESE_DEBUG(sese_tmp_logger, format, ...)                                 \
    __SESE_LOG(sese_tmp_logger, sese::record::Level::DEBUG, format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define __SESE_INFO(sese_tmp_logger, format, ...)                                 \
    __SESE_LOG(sese_tmp_logger, sese::record::Level::INFO, format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define __SESE_WARN(sese_tmp_logger, format, ...)                                 \
    __SESE_LOG(sese_tmp_logger, sese::record::Level::WARN, format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define __SESE_ERROR(sese_tmp_logger, format, ...)                               \
    __SESE_LOG(sese_tmp_logger, sese::record::Level::ERR, format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define SESE_DEBUG(sese_tmp_format, ...)                                                              \
    __SESE_LOG(sese::record::getLogger(), sese::record::Level::DEBUG, sese_tmp_format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define SESE_INFO(sese_tmp_format, ...)                                                              \
    __SESE_LOG(sese::record::getLogger(), sese::record::Level::INFO, sese_tmp_format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define SESE_WARN(sese_tmp_format, ...)                                                              \
    __SESE_LOG(sese::record::getLogger(), sese::record::Level::WARN, sese_tmp_format, ##__VA_ARGS__) \
    SESE_MARCO_END

#define SESE_ERROR(sese_tmp_format, ...)                                                            \
    __SESE_LOG(sese::record::getLogger(), sese::record::Level::ERR, sese_tmp_format, ##__VA_ARGS__) \
    SESE_MARCO_END
