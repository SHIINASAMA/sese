// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file AbstractByteBuffer.h
 * @brief 字节缓冲区类
 * @author kaoru
 * @date 2022年3月28日
 */

#pragma once

#include "sese/io/Stream.h"
#include "sese/io/PeekableStream.h"

namespace sese::io {

/**
 * @brief 字节缓冲区类
 */
class  AbstractByteBuffer : public Stream, public PeekableStream {
private:
    /**
     * @brief 缓冲节点
     */
    struct Node {
        /// 缓存内存
        void *buffer = nullptr;
        /// 下一个节点
        Node *next = nullptr;
        /// 节点已用内存
        size_t length = 0;
        /// 节点容量
        size_t cap = 0;
        /**
         * 初始化节点
         * @param buffer_size 节点分配内存大小
         */
        explicit Node(size_t buffer_size);
        /// 析构
        ~Node();
    };

public:
    /**
     * @param base_size 初始节点内存大小
     * @param factor 追加内存节点大小
     */
    explicit AbstractByteBuffer(size_t base_size = STREAM_BYTE_STREAM_SIZE_FACTOR, size_t factor = STREAM_BYTE_STREAM_SIZE_FACTOR);

    /// 拷贝
    AbstractByteBuffer(const AbstractByteBuffer &abstract_byte_buffer) noexcept;
    /// 移动语义
    AbstractByteBuffer(AbstractByteBuffer &&abstract_byte_buffer) noexcept;

    /// 析构
    ~AbstractByteBuffer() override;
    /// 重置读取位置
    virtual void resetPos();
    /// 是否有可读数据
    virtual bool eof();

    /**
     * @return 所有节点已用内存总数
     */
    [[nodiscard]] virtual size_t getLength() const;
    /**
     *
     * @return 所有节点容量总数
     */
    [[nodiscard]] virtual size_t getCapacity() const;
    /**
     *
     * @return 可读字节总数
     */
    [[nodiscard]] virtual size_t getReadableSize() const;

    /**
     * 释放 CurrentReadNode 前的所有节点
     * @return 实际释放空间，单位 “字节”
     */
    virtual size_t freeCapacity();

    virtual void swap(AbstractByteBuffer &other) noexcept;

    // [[nodiscard]] size_t getCurrentWritePos() const { return currentWritePos; }
    // [[nodiscard]] size_t getCurrentReadPos() const { return currentReadPos; }

public:
    int64_t read(void *buffer, size_t len) override;
    int64_t write(const void *buffer, size_t len) override;

    int64_t peek(void *buffer, size_t len) override;
    int64_t trunc(size_t need_read) override;

private:
    size_t factor = 0;

    Node *root = nullptr;
    Node *currentWriteNode = nullptr;
    size_t currentWritePos = 0;
    Node *currentReadNode = nullptr;
    size_t currentReadPos = 0;

    /// length 不计算最后一个 Node 的，真实长度应为
    /// realLength = length + currentWriteNode->length
    size_t length = 0;
    size_t cap = 0;
};

} // namespace sese::io