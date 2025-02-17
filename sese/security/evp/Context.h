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
 * @file Context.h
 * @author kaoru
 * @version 0.1
 * @brief Base class for EVP context
 * @date September 13, 2023
 */

#pragma once

#include <sese/Config.h>

namespace sese::security::evp {

/// Base class for EVP context
class  Context {
public:
    using Ptr = std::unique_ptr<Context>;

    Context() = default;

    virtual ~Context() noexcept = default;

    virtual void update(const void *buffer, size_t len) noexcept = 0;

    virtual void final() noexcept = 0;

    virtual void *getResult() noexcept = 0;

    virtual size_t getLength() noexcept = 0;
};

} // namespace sese::security::evp