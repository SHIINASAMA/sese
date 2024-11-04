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

#include <sese/text/Number.h>

#include <bitset>

using sese::text::Number;

std::string Number::toHex(uint64_t number, bool upper_case) noexcept {
    StringBuilder builder(number2StringLength(number));
    toString(builder, number, 16, upper_case);
    return builder.toString();
}

std::string Number::toHex(int64_t number, bool upper_case) noexcept {
    StringBuilder builder(number2StringLength(number));
    toString(builder, number, 16, upper_case);
    return builder.toString();
}

std::string Number::toOct(uint64_t number) noexcept {
    StringBuilder builder(number2StringLength(number));
    toString(builder, number, 8, true);
    return builder.toString();
}

std::string Number::toOct(int64_t number) noexcept {
    StringBuilder builder(number2StringLength(number));
    toString(builder, number, 8, true);
    return builder.toString();
}

std::string Number::toBin(uint64_t number) noexcept {
    std::bitset<64> bits(number);
    return toStringWithNoLeadingZeros(bits.to_string());
}

std::string Number::toBin(int64_t number) noexcept {
    std::bitset<64> bits(number);
    return toStringWithNoLeadingZeros(bits.to_string());
}

std::string Number::toString(double number, uint16_t placeholder) noexcept {
    StringBuilder builder(floating2StringLength(number, placeholder));
    toString(builder, number, placeholder);
    return builder.toString();
}

std::string Number::toStringWithNoLeadingZeros(const std::string &number) noexcept {
    size_t found = number.find_first_not_of('0');
    if(found != std::string::npos) {
        return number.substr(found);
    }
    return "0";
}