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

#include "sese/io/BufferedOutputStream.h"

using sese::io::BufferedOutputStream;

BufferedOutputStream::BufferedOutputStream(const OutputStream::Ptr &source, size_t buffer_size) {
    this->source = source;
    this->pos = 0;
    this->len = 0;
    this->cap = buffer_size;
    this->buffer = malloc(cap);
}

BufferedOutputStream::~BufferedOutputStream() noexcept {
    free(buffer);
}

int64_t BufferedOutputStream::write(const void *buf, size_t length) {
    /*
     * 如果写入所需字节数需要刷新缓存两次一下，
     * 则暂时写入缓存，避免频繁的IO操作；
     * 如果写入所需字节数需要刷新缓存两次及两次以上，
     * 则处理原先的缓存后，直接操作裸流，减少拷贝次数
     */
    if (length <= this->cap) {
        if (this->cap - this->len >= length) {
            // 字节数足够 - 不需要刷新
            memcpy(static_cast<char *>(this->buffer) + this->len, buf, length);
            this->len += length;
            return static_cast<int64_t>(length);
        } else {
            // 字节数不足 - 需要刷新
            size_t expect = len - pos;
            if (expect == flush()) {
                memcpy(this->buffer, (char *) buf, length);
                this->len = length;
                expect = length;
                return static_cast<int64_t>(expect);
            } else {
                // flush 失败
                return -1;
            }
        }
    } else {
        // 直接写入
        if (this->len != this->pos) {
            // 缓存区有剩余，需要刷新
            size_t expect = len - pos;
            if (expect != flush()) {
                // flush 失败
                return -1;
            }
        }

        int64_t wrote = 0;
        while (true) {
            auto rt = source->write(static_cast<const char *>(buf) + wrote, length - wrote >= cap ? cap : length - wrote);
            if (rt <= 0) return -1;
            wrote += rt;
            if (wrote == length) break;
        }

        return wrote;
    }
}

int64_t BufferedOutputStream::flush() noexcept {
    // 将已有未处理数据立即写入流
    auto wrote = source->write(static_cast<char *>(buffer) + pos, len - pos);
    pos = 0;
    len = 0;
    return wrote;
}