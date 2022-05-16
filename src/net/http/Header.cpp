#include <sese/net/http/Header.h>

using sese::http::Header;

Header::Header(const std::initializer_list<std::pair<const std::string &, const std::string &>> &initializerList) noexcept {
    for (const auto &item: initializerList) {
        headers.insert(item);
    }
}


void Header::set(const std::string &key, const std::string &value) noexcept {
    headers[key] = value;
}

const std::string &Header::get(const std::string &key, const std::string &defaultValue) noexcept {
    auto res = headers.find(key);
    if (res == headers.end()) {
        return defaultValue;
    } else {
        return res->second;
    }
}

std::string_view Header::getView(const std::string &key, const std::string &defaultValue) noexcept {
    auto res = headers.find(key);
    if (res == headers.end()) {
        return defaultValue;
    } else {
        return res->second;
    }
}