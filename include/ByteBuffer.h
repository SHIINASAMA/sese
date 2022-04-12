/**
 * @file ByteBuffer.h
 * @brief 线程安全的字节缓冲类
 * @author kaoru
 * @date 2022年4月5日
 */
#pragma once
#include "AbstractByteBuffer.h"
#include <mutex>

#ifdef _WIN32
#pragma warning(disable : 4251)
#endif

namespace sese {

    /**
     * @brief 线程安全的字节缓冲类
     */
    class API ByteBuffer : AbstractByteBuffer {
    public:
        explicit ByteBuffer(size_t baseSize);
        void resetPos() override;
        [[nodiscard]] size_t getLength();
        [[nodiscard]] size_t getCapacity();
        size_t freeCapacity() override;
        int64_t read(void *buffer, size_t len) override;
        int64_t write(void *buffer, size_t needWrite) override;
        void close() override;

    private:
        std::mutex mutex;
    };
}// namespace sese