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

#include "sese/record/SimpleFormatter.h"
#include "sese/text/DateTimeFormatter.h"
#include "sese/text/StringBuilder.h"
#include "sese/io/InputBufferWrapper.h"

#ifdef _WIN32
#pragma warning(disable : 4018)
#pragma warning(disable : 4996)
#endif

namespace sese {

extern "C"  const char *getLevelString(record::Level level) noexcept {
    switch (level) { // GCOVR_EXCL_LINE
        case record::Level::DEBUG:
            return "D";
        case record::Level::INFO:
            return "I";
        case record::Level::WARN:
            return "W";
        case record::Level::ERR:
            return "E";
        default:            // GCOVR_EXCL_LINE
            return "DEBUG"; // GCOVR_EXCL_LINE
    }
}

record::SimpleFormatter::SimpleFormatter(const std::string &text_pattern, const std::string &time_pattern) noexcept
    : AbstractFormatter() {
    this->textPattern = text_pattern;
    this->timePattern = time_pattern;
}

std::string record::SimpleFormatter::dump(const Event::Ptr &event) noexcept {
    sese::text::StringBuilder builder(1024);
    auto input = sese::io::InputBufferWrapper(textPattern.c_str(), textPattern.length());
    while (true) {
        char buf[2]{};
        auto len = input.peek(buf, 2); // GCOVR_EXCL_LINE
        if (len == 0) {
            break;
        } else {
            if (buf[0] == 'c') {
                builder.append(sese::text::DateTimeFormatter::format(event->getTime(), this->timePattern)); // GCOVR_EXCL_LINE
                input.trunc(1);                                                                             // GCOVR_EXCL_LINE
                continue;
            } else if (buf[0] == 'm') {
                builder.append(event->getMessage()); // GCOVR_EXCL_LINE
                input.trunc(1);                      // GCOVR_EXCL_LINE
                continue;
            } else if (len != 2) {
                builder.append(buf[0]); // GCOVR_EXCL_LINE
                input.trunc(1);         // GCOVR_EXCL_LINE
                continue;
            }

            if (buf[0] == 'l') {
                if (buf[1] == 'i') {
                    builder.append(std::to_string(event->getLine())); // GCOVR_EXCL_LINE
                    input.trunc(2);                                   // GCOVR_EXCL_LINE
                } else if (buf[1] == 'v') {
                    builder.append(getLevelString(event->getLevel())); // GCOVR_EXCL_LINE
                    input.trunc(2);                                    // GCOVR_EXCL_LINE
                } else {
                    builder.append(buf[0]); // GCOVR_EXCL_LINE
                    input.trunc(1);         // GCOVR_EXCL_LINE
                }
            } else if (buf[0] == 'f') {
                if (buf[1] == 'n') {
                    builder.append(event->getFileName()); // GCOVR_EXCL_LINE
                    input.trunc(2);                       // GCOVR_EXCL_LINE
                } else {
                    builder.append(buf[0]); // GCOVR_EXCL_LINE
                    input.trunc(1);         // GCOVR_EXCL_LINE
                }
            } else if (buf[0] == 't') {
                if (buf[1] == 'h') {
                    builder.append(std::to_string(event->getThreadId())); // GCOVR_EXCL_LINE
                    input.trunc(2);                                       // GCOVR_EXCL_LINE
                } else if (buf[1] == 'n') {
                    builder.append(event->getThreadName()); // GCOVR_EXCL_LINE
                    input.trunc(2);                         // GCOVR_EXCL_LINE
                } else {
                    builder.append(buf[0]); // GCOVR_EXCL_LINE
                    input.trunc(1);         // GCOVR_EXCL_LINE
                }
            } else if (buf[0] == '%') {
                builder.append(buf[1]); // GCOVR_EXCL_LINE
                input.trunc(2);         // GCOVR_EXCL_LINE
            } else {
                builder.append(buf[0]); // GCOVR_EXCL_LINE
                input.trunc(1);         // GCOVR_EXCL_LINE
            }
        }
    }
    return builder.toString();
}

} // namespace sese