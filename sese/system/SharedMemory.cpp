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

#include "SharedMemory.h"

#include "sese/util/Util.h"

sese::Result<sese::system::SharedMemory::Ptr, sese::ErrorCode> sese::system::SharedMemory::createEx(const char *name, size_t size) noexcept {
    if (auto result = create(name, size)) {
        return Result<Ptr, ErrorCode>::success(std::move(result));
    }
    return Result<Ptr, ErrorCode>::error({
        getErrorCode(),
        getErrorString()
    });
}

sese::Result<sese::system::SharedMemory::Ptr, sese::ErrorCode> sese::system::SharedMemory::useEx(const char *name) noexcept {
    if (auto result = use(name)) {
        return Result<Ptr, ErrorCode>::success(std::move(result));
    }
    return Result<Ptr, ErrorCode>::error({
        getErrorCode(),
        getErrorString()
    });
}
