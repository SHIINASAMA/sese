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

#include <sese/net/http/Http2Frame.h>
#include <sese/util/Endian.h>

using sese::net::http::Http2Frame;

Http2Frame::Http2Frame(size_t frame_size)
    : length(0),
      type(0),
      flags(0),
      ident(0),
      frame(std::make_unique<char []>(frame_size + 9)) {
}

char *Http2Frame::getFrameBuffer() const {
    return frame.get();
}

size_t Http2Frame::getFrameLength() const {
    return length + 9;
}

char *Http2Frame::getFrameContentBuffer() const {
    return frame.get() + 9;
}

size_t Http2Frame::getFrameContentLength() const {
    return length;
}

void Http2Frame::buildFrameHeader() const {
    auto length = ToBigEndian32(this->length);
    auto ident = ToBigEndian32(this->ident);

    auto buffer = getFrameBuffer();
    memcpy(buffer + 0, reinterpret_cast<const char *>(&length) + 1, 3);
    memcpy(buffer + 3, &this->type, 1);
    memcpy(buffer + 4, &this->flags, 1);
    memcpy(buffer + 5, &ident, 4);
}

