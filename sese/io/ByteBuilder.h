/**
 * @file ByteBuilder.h
 * @brief 线程不安全的字节缓冲类
 * @author kaoru
 * @date 2022年3月28日
 */

#pragma once

#include "sese/Config.h"
#include "sese/io/AbstractByteBuffer.h"

namespace sese::io {

/**
 * @brief 线程不安全的字节缓冲类
 */
class  ByteBuilder final : public AbstractByteBuffer {
public:
    using Ptr = std::unique_ptr<ByteBuilder>;

    ByteBuilder(size_t baseSize = STREAM_BYTE_STREAM_SIZE_FACTOR, size_t factor = STREAM_BYTE_STREAM_SIZE_FACTOR) noexcept // NOLINT
        : AbstractByteBuffer(baseSize, factor) {
    }
};

} // namespace sese::io