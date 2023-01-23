/// \file BlockAppender.h
/// \author kaoru
/// \date 2022年8月6日
/// \brief 分块文件日志输出源类
/// \version 0.1

#pragma once
#include <sese/record/AbstractAppender.h>
#include "sese/util/FileStream.h"
#include "sese/util/BufferedStream.h"
#include <mutex>

namespace sese::record {

    /// 分块文件日志输出源类
    class API BlockAppender : public AbstractAppender {
    public:
        /// 初始化分块参数
        /// \param blockMaxSize 分块的预定大小，此参数在非 DEBUG 模式下具有最小值限制，至少为 1000 * 1024，即 1MB
        /// \param level 等级阈值
        explicit BlockAppender(size_t blockMaxSize, Level level = Level::DEBUG);

        ~BlockAppender() noexcept override;

        void dump(const char *buffer, size_t size) noexcept override;

    private:
        size_t size = 0;
        std::mutex mutex;
        size_t maxSize = 0;
        BufferedStream::Ptr bufferedStream;
        FileStream::Ptr fileStream;
    };

}// namespace sese::record