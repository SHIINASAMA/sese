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
 * @file Base64Converter.h
 * @author kaoru
 * @date 2022年4月18日
 * @brief BASE64 转换器
 */
#pragma once

#include "sese/io/Stream.h"
#include "sese/util/NotInstantiable.h"

#ifdef _WIN32
#pragma warning(disable : 4624)
#endif

namespace sese {

/**
 * @brief BASE64 转换器
 */
class  Base64Converter final : public NotInstantiable {
public:
    using OutputStream = sese::io::OutputStream;
    using InputStream = sese::io::InputStream;

    Base64Converter() = delete;

    static void encode(const InputStream::Ptr &src, const OutputStream::Ptr &dest);
    static void encode(InputStream *src, OutputStream *dest);

    static void decode(const InputStream::Ptr &src, const OutputStream::Ptr &dest);
    static void decode(InputStream *src, OutputStream *dest);

public:
    using CodePage = const unsigned char *;

    /// 按照 Base62 码表编码
    /// \warning 注意此函数的编码方式类似于 Base64，并不是标准的 Base62编码
    /// \param input 输入流
    /// \param output 输出流
    /// \return 编码结果
    static bool encodeBase62(InputStream *input, OutputStream *output) noexcept;
    /// 按照 Base62 码表解码
    /// \warning 注意此函数的解码方式类似于 Base64，并不是标准的 Base62解码
    /// \param input 输入流
    /// \param output 输出流
    /// \return 编码结果
    static bool decodeBase62(InputStream *input, OutputStream *output) noexcept;

public:
    static bool encodeInteger(size_t num, OutputStream *output) noexcept;
    static int64_t decodeBuffer(const unsigned char *buffer, size_t size) noexcept;
};
} // namespace sese