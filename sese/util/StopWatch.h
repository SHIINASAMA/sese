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
 * @file StopWatch.h
 * @brief 停表类
 * @author kaoru
 * @version 0.1
 */

#pragma once

#include <sese/Config.h>
#include <sese/util/DateTime.h>

namespace sese {

/// 停表类
class  StopWatch {
public:
    StopWatch();

    /**
     * 按下停表并返回距离上一次停止的时间间隔
     * @return 时间间隔
     */
    TimeSpan stop();

    /**
     * 重置停表
     */
    void reset();

protected:
    DateTime lastStopTime;
};
} // namespace sese
