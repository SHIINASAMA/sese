#include <sese/net/http/Header.h>

#include <cctype>
#include <algorithm>

using namespace sese::net::http;

Header::Header(const std::initializer_list<KeyValueType> &initializerList) noexcept {
    for (const auto &item: initializerList) {
        headers.insert(item);
    }
}


#if defined(SESE_PLATFORM_WINDOWS) && !defined(__MINGW32__)
#define XX std::tolower
#else
#define XX ::tolower
#endif

Header *Header::set(const std::string &key, const std::string &value) noexcept {
    auto temp = key;
    std::transform(temp.begin(), temp.end(), temp.begin(), XX);
    headers[temp] = value;
    return this;
}

const std::string &Header::get(const std::string &key, const std::string &defaultValue) noexcept {
    auto temp = key;
    std::transform(temp.begin(), temp.end(), temp.begin(), XX);
    auto res = headers.find(temp);
    if (res == headers.end()) {
        return defaultValue;
    } else {
        return res->second;
    }
}

#undef XX

#if SESE_CXX_STANDARD > 201700L

std::string_view Header::getView(const std::string &key, const std::string &defaultValue) noexcept {
    auto res = headers.find(key);
    if (res == headers.end()) {
        return defaultValue;
    } else {
        return res->second;
    }
}

#endif

const CookieMap::Ptr &Header::getCookies() const {
    return cookies;
}

void Header::setCookies(const CookieMap::Ptr &cookies) {
    Header::cookies = cookies;
}

Cookie::Ptr Header::getCookie(const std::string &name) const {
    if (Header::cookies) {
        return Header::cookies->find(name);
    }
    return nullptr;
}

void Header::setCookie(const Cookie::Ptr &cookie) {
    if (!Header::cookies) {
        Header::cookies = std::make_shared<CookieMap>();
    }
    Header::cookies->add(cookie);
}