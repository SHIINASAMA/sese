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

#include "sese/util/TimeSpan.h"

const sese::TimeSpan sese::TimeSpan::ZERO = sese::TimeSpan(0, 0, 0, 0, 0, 0);

sese::TimeSpan::TimeSpan(
        int32_t days,
        int32_t hours,
        int32_t minutes,
        int32_t seconds,
        int32_t milliseconds,
        int32_t microseconds
) noexcept {
    timestamp += static_cast<uint64_t>(days) * 24 * 60 * 60 * 1000 * 1000;
    timestamp += static_cast<uint64_t>(hours) * 60 * 60 * 1000 * 1000;
    timestamp += static_cast<uint64_t>(minutes) * 60 * 1000 * 1000;
    timestamp += static_cast<uint64_t>(seconds) * 1000 * 1000;
    timestamp += static_cast<uint64_t>(milliseconds) * 1000;
    timestamp += static_cast<uint64_t>(microseconds);
}

sese::TimeSpan::TimeSpan(uint64_t timestamp) noexcept {
    this->timestamp = timestamp;
}

int32_t sese::TimeSpan::getDays() const noexcept {
    return static_cast<int32_t>(timestamp / 1000 / 1000 / 60 / 60 / 24);
}

int32_t sese::TimeSpan::getHours() const noexcept {
    return static_cast<int32_t>(timestamp / 1000 / 1000 / 60 / 60 % 24);
}

int32_t sese::TimeSpan::getMinutes() const noexcept {
    return static_cast<int32_t>(timestamp / 1000 / 1000 / 60 % 60);
}

int32_t sese::TimeSpan::getSeconds() const noexcept {
    return static_cast<int32_t>(timestamp / 1000 / 1000 % 60);
}

int32_t sese::TimeSpan::getMilliseconds() const noexcept {
    return static_cast<int32_t>(timestamp % (1000 * 1000) / 1000);
}

int32_t sese::TimeSpan::getMicroseconds() const noexcept {
    return static_cast<int32_t>(timestamp % (1000 * 1000) % 1000);
}

uint64_t sese::TimeSpan::getTotalMicroseconds() const noexcept {
    return timestamp;
}

double sese::TimeSpan::getTotalMilliseconds() const noexcept {
    return static_cast<double>(timestamp) / 1000;
}

double sese::TimeSpan::getTotalSeconds() const noexcept {
    return static_cast<double>(timestamp) / 1000 / 1000;
}

double sese::TimeSpan::getTotalMinutes() const noexcept {
    return static_cast<double>(timestamp) / 1000 / 1000 / 60;
}

double sese::TimeSpan::getTotalHours() const noexcept {
    return static_cast<double>(timestamp) / 1000 / 1000 / 60 / 60;
}

double sese::TimeSpan::getTotalDays() const noexcept {
    return static_cast<double>(timestamp) / 1000 / 1000 / 60 / 60 / 24;
}