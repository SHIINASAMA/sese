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

/// \file BalanceLoader.h
/// \author kaoru
/// \date June 9, 2023
/// \brief Balanced Loader

#pragma once

#include "sese/Config.h"

#ifdef SESE_PLATFORM_LINUX

#include "sese/service/SystemBalanceLoader.h"

namespace sese::service {
using BalanceLoader = SystemBalanceLoader;
using Service = sese::event::EventLoop;
} // namespace sese::service

#else

#include "sese/service/UserBalanceLoader.h"

namespace sese::service {
using BalanceLoader = UserBalanceLoader;
using Service = sese::event::EventLoop;
} // namespace sese::service

#endif