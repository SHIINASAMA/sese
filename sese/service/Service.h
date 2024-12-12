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

/// \file Service.h
/// \brief Service Interface
/// \author kaoru
/// \date March 4, 2024

#pragma once

#include <memory>
#include <string>

#include <sese/util/ErrorCode.h>

namespace sese::service {

/// @brief Service Interface
class Service {
public:
    using Ptr = std::unique_ptr<Service>;

    Service() = default;

    virtual ~Service() = default;

    virtual bool startup() = 0;

    virtual bool shutdown() = 0;

    virtual int getLastError() = 0;

    virtual std::string getLastErrorMessage() = 0;

    ErrorCode getErrorCode();
};
} // namespace sese::service