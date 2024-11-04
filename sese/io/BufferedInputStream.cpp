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

#include "sese/io/BufferedInputStream.h"

using sese::io::BufferedInputStream;

BufferedInputStream::BufferedInputStream(const InputStream::Ptr &source, size_t buffer_size) {
    this->source = source;
    this->pos = 0;
    this->len = 0;
    this->cap = buffer_size;
    this->buffer = malloc(cap);
}

BufferedInputStream::~BufferedInputStream() noexcept {
    free(buffer);
}

inline int64_t BufferedInputStream::preRead() noexcept {
    // 尝试使用目标流填充缓存
    auto read = source->read(buffer, cap);
    // 此处用于修正一些输入源可能读取返回负值的情况
    read = read < 0 ? 0 : read; // GCOVR_EXCL_LINE
    pos = 0;
    len = read;
    return read;
}

int64_t BufferedInputStream::read(void *buf, size_t length) {
    /*
     * 如果读取所需字节数需要缓存两次以下，
     * 需要进行预读操作，避免频繁的IO操作；
     * 如果读取所需字节数需要缓存两次及两次以上，
     * 则处理原先的缓存后，直接操作裸流，减少拷贝次数
     */
    if (length <= this->cap) {
        if (this->len - this->pos >= length) {
            // 字节数足够 - 不需要预读取
            memcpy(buf, static_cast<char *>(this->buffer) + this->pos, length);
            pos += length;
            return static_cast<int64_t>(length);
        } else {
            // 字节数不足 - 需要预读取
            size_t total = this->len - this->pos;
            memcpy(buf, static_cast<char *>(this->buffer) + this->pos, total);
            pos += total;
            if (0 != preRead()) {
                if (this->len - this->pos >= length - total) {
                    // 字节数足够
                    memcpy(static_cast<char *>(buf) + total, this->buffer, length - total);
                    pos = length - total;
                    total = length;
                } else {
                    // 字节数不足，且无法继续读取
                    memcpy(static_cast<char *>(buf) + total, this->buffer, this->len - this->pos);
                    pos = this->len - this->pos;
                    total += this->len - this->pos;
                }
            }
            return static_cast<int64_t>(total);
        }
    } else {
        // 先处理已有缓存
        size_t total = this->len - this->pos;
        memcpy(buf, this->buffer, total);
        this->len = 0;
        this->pos = 0;
        // 操作裸流
        while (true) {
            size_t read = source->read(static_cast<char *>(buf) + total, (length - total) >= 1024 ? 1024 : length - total);
            total += static_cast<int64_t>(read);
            // 无可再读
            if (read <= 0) break;
            // 完成目标
            if (total == length) break;
        }
        return static_cast<int64_t>(total);
    }
}