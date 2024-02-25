#include <sese/text/Number.h>

#include <bitset>

using sese::text::Number;

std::string Number::toHex(uint64_t number, bool upper_case) noexcept {
    char buffer[32];
    size_t len;
    if (upper_case) {
        len = std::snprintf(buffer, 32, "%" PRIX64, number);
    } else {
        len = std::snprintf(buffer, 32, "%" PRIx64, number);
    }
    return std::string{buffer, len};
}

std::string Number::toHex(int64_t number, bool upper_case) noexcept {
    char buffer[32];
    size_t len;
    if (upper_case) {
        len = std::snprintf(buffer, 32, "%" PRIX64, number);
    } else {
        len = std::snprintf(buffer, 32, "%" PRIx64, number);
    }
    return std::string{buffer, len};
}

std::string Number::toOct(uint64_t number) noexcept {
    char buffer[32];
    size_t len = std::snprintf(buffer, 32, "%" PRIo64, number);
    return std::string{buffer, len};
}

std::string Number::toOct(int64_t number) noexcept {
    char buffer[32];
    size_t len = std::snprintf(buffer, 32, "%" PRIo64, number);
    return std::string{buffer, len};
}

std::string Number::toBin(uint64_t number) noexcept {
    std::bitset<64> bits(number);
    return toStringWithNoLeadingZeros(bits.to_string());
}

std::string Number::toBin(int64_t number) noexcept {
    std::bitset<64> bits(number);
    return toStringWithNoLeadingZeros(bits.to_string());
}

std::string Number::toStringWithNoLeadingZeros(const std::string &number) noexcept {
    size_t found = number.find_first_not_of('0');
    if(found != std::string::npos) {
        return number.substr(found);
    }
    return "0";
}