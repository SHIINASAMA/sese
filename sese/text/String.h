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
* @file String.h
* @author kaoru
* @version 0.1
* @brief UTF-8 字符串
* @date 2023年9月13日
*/

#pragma once

#include <sese/text/SString.h>
#include <sese/text/SStringBuilder.h>

namespace sese::text {
using Char = sstr::SChar;
using String = sstr::SString;
using StringView = sstr::SStringView;
} // namespace sese::text