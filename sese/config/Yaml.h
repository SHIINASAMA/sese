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

/// \file Yaml.h
/// \brief YAML parser
/// \author Kaoru
/// \date November 4, 2023

#pragma once

#include <sese/io/InputStream.h>
#include <sese/io/OutputStream.h>
#include <sese/util/Value.h>

#include <queue>
#include <stack>

namespace sese {
/// YAML parser
class Yaml final : public NotInstantiable {
    using Tokens = std::vector<std::string>;
    using TokensQueue = std::queue<std::tuple<int, Tokens>>;
    using InputStream = io::InputStream;
    using OutputStream = io::OutputStream;
    using Line = std::tuple<int, std::string>;

    static Value parseBasic(const std::string &value);

    static bool parseObject(TokensQueue &tokens_queue, std::stack<std::pair<Value *, int>> &stack);

    static bool parseArray(TokensQueue &tokens_queue, std::stack<std::pair<Value *, int>> &stack);

    static void streamifyObject(io::OutputStream *output, const Value::Dict &dict, size_t level);

    static void streamifyArray(io::OutputStream *output, const Value::List &list, size_t level);

    static int getSpaceCount(const std::string &line) noexcept;

    static Line getLine(InputStream *input) noexcept;

    static Tokens tokenizer(const std::string &line) noexcept;

    static void writeSpace(size_t count, OutputStream *output) noexcept;

public:
    Yaml() = delete;

    /// Deserialize yaml object from stream
    /// \param input Input stream
    /// \return If parsing fails, Value::isNull is true
    static Value parse(io::InputStream *input);

    /// Serialize yaml object to stream
    /// \param output Output stream
    /// \param value yaml object
    static void streamify(io::OutputStream *output, const Value &value);
};
} // namespace sese