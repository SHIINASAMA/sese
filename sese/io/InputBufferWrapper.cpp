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

#include "sese/io/InputBufferWrapper.h"
#include "sese/io/OutputStream.h"

sese::io::InputBufferWrapper::InputBufferWrapper(const char *buffer, size_t cap) {
    this->buffer = buffer;
    this->cap = cap;
}

int64_t sese::io::InputBufferWrapper::read(void *buf, size_t length) {
    if ((cap - pos) >= length) {
        memcpy(buf, buffer + pos, length);
        pos += length;
        return static_cast<int64_t>(length);
    } else {
        auto remain = cap - pos;
        memcpy(buf, buffer + pos, remain);
        pos = cap;
        return static_cast<int64_t>(remain);
    }
}

int64_t sese::io::InputBufferWrapper::peek(void *buf, size_t length) {
    if ((cap - pos) >= length) {
        memcpy(buf, buffer + pos, length);
        return static_cast<int64_t>(length);
    } else {
        auto remain = cap - pos;
        memcpy(buf, buffer + pos, remain);
        return static_cast<int64_t>(remain);
    }
}

const char *sese::io::InputBufferWrapper::getBuffer() const {
    return buffer;
}

size_t sese::io::InputBufferWrapper::getLength() const {
    return pos;
}

size_t sese::io::InputBufferWrapper::getCapacity() const {
    return cap;
}

void sese::io::InputBufferWrapper::reset() noexcept {
    this->pos = 0;
}

int64_t sese::io::InputBufferWrapper::trunc(size_t length) {
    if ((cap - pos) >= length) {
        // memcpy(buf, buffer + pos, length);
        pos += length;
        return static_cast<int64_t>(length);
    } else {
        auto remain = cap - pos;
        // memcpy(buf, buffer + pos, remain);
        pos = cap;
        return static_cast<int64_t>(remain);
    }
}

int64_t operator<<(sese::io::OutputStream &out, sese::io::InputBufferWrapper &input) noexcept {
    auto len = out.write(input.getBuffer() + input.getLength(), input.getCapacity() - input.getLength());
    input.trunc(len);
    return len;
}

int64_t operator<<(sese::io::OutputStream *out, sese::io::InputBufferWrapper &input) noexcept {
    auto len = out->write(input.getBuffer() + input.getLength(), input.getCapacity() - input.getLength());
    input.trunc(len);
    return len;
}