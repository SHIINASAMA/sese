/**
 * HPACK 的键值对处理规则
 *      0.索引的 HEADER 字段
 *          1 + index -> indexedName + indexedValue
 *      1.带索引的字面 HEADER 字段
 *          01 + index -> indexedName + 编码 value
 *          01 + 0     -> 编码 key + 编码 value
 *      2.不带索引的字面 HEADER 字段
 *          0000 + index -> indexedName + 编码 value
 *          0000 + 0     -> 编码 key + 编码 value
 *      3.从不索引的字面 HEADER 字段
 *          0001 + index -> indexedName + 编码 value
 *          0001 + 0     -> 编码 key + 编码 value
 *      4.更新动态表大小
 *          001 + size
 */

#include "sese/net/http/HttpUtil.h"
#include "sese/net/http/HPackUtil.h"
#include "sese/net/http/HPACK.h"
#include "sese/text/DateTimeFormatter.h"
#include "sese/util/DateTime.h"
#include "sese/text/StringBuilder.h"

#include <cmath>
#include <algorithm>

#ifdef _WIN32
#pragma warning(disable : 4996)
#endif

using namespace sese::net::http;

HuffmanDecoder HPackUtil::decoder{};
HuffmanEncoder HPackUtil::encoder{};

bool HPackUtil::decode(InputStream *src, size_t content_length, DynamicTable &table, Header &header) noexcept {
    uint8_t buf;
    size_t len = 0;
    while (len < content_length) {
        if (1 != src->read(&buf, 1)) {
            return false;
        }
        len += 1;
        /// 对应第 0 种情况
        if (buf & 0b1000'0000) {
            uint32_t index = 0;
            auto l = decodeInteger(buf, src, index, 7);
            if (-1 == l) {
                return false;
            }
            len += l;
            if (index == 0) {
                return false;
            }

            auto pair = table.get(index);
            if (pair == std::nullopt) {
                return false;
            }
            if (strcasecmp(pair->first.c_str(), "Cookie") == 0) {
                auto cookies = HttpUtil::parseFromCookie(pair->second);
                header.setCookies(cookies);
            } else {
                header.set(pair->first, pair->second);
            }
        }
        // 对应第4种情况
        else if (buf & 0b0010'0000) {
            uint32_t new_size;
            auto l = decodeInteger(buf, src, new_size, 5);
            if (-1 == l) {
                return false;
            }
            len += l;
            table.resize(new_size);
        } else {
            uint32_t index = 0;
            bool is_store;
            /// 对应第 1 种情况
            if (0b0100'0000 == (buf & 0b1100'0000)) {
                // 添加至动态表
                auto l = decodeInteger(buf, src, index, 6);
                if (-1 == l) {
                    return false;
                }
                len += l;
                is_store = true;
            }
            /// 对应第 2 种和第 3 种情况
            else {
                // 不添加至动态表
                auto l = decodeInteger(buf, src, index, 4);
                if (-1 == l) {
                    return false;
                }
                len += l;
                is_store = false;
            }

            std::string key;
            if (0 != index) {
                auto ret = table.get(index);
                if (ret == std::nullopt) {
                    return false;
                }
                key = ret.value().first;
            } else {
                auto l = decodeString(src, key);
                if (-1 == l) {
                    return false;
                } else {
                    len += l;
                }
            }

            std::string value;
            auto l = decodeString(src, value);
            if (-1 == l) {
                return false;
            } else {
                len += l;
            }

            if (is_store) {
                table.set(key, value);
            }

            if (strcasecmp(key.c_str(), "Cookie") == 0) {
                auto cookies = HttpUtil::parseFromCookie(value);
                header.setCookies(cookies);
            } else {
                header.set(key, value);
            }
        }
    }
    return true;
}

size_t HPackUtil::encode(OutputStream *dest, DynamicTable &table, Header &once_header,
                         Header &indexed_header) noexcept {
    size_t size = 0;
    // 处理索引的 HEADERS
    for (const auto &item: indexed_header) {
        // 动态表查询
        {
            auto iterator_key = table.end();
            auto iterator_all = table.end();
            for (auto header = table.begin(); header != table.end(); ++header) {
                if (header->first == item.first) {
                    iterator_key = header;
                    if (header->second == item.second) {
                        iterator_all = header;
                        break;
                    }
                }
            }

            // auto iterator = std::find_if(table.begin(), table.end(), isHitAll);
            if (iterator_all != table.end()) {
                /// 对应第 0 种情况
                size_t index = iterator_all - table.begin() + 62;
                size += encodeIndexCase0(dest, index);
                continue;
            }

            // iterator = std::find_if(table.begin(), table.end(), isHit);
            // 存在动态表中
            if (iterator_key != table.end()) {
                size_t index = iterator_key - table.begin() + 62;
                /// 对应第 1 种情况
                size += encodeIndexCase1(dest, index);
                size += encodeString(dest, item.second);
                /// 添加动态表
                table.set(item.first, item.second);
                continue;
            }
        }
        // 静态表查询
        {
            auto iterator_key = PREDEFINED_HEADERS.end();
            auto iterator_all = PREDEFINED_HEADERS.end();
            for (auto header = PREDEFINED_HEADERS.begin(); header != PREDEFINED_HEADERS.end(); ++header) {
                if (header->first == item.first) {
                    iterator_key = header;
                    if (header->second == item.second) {
                        iterator_all = header;
                        break;
                    }
                }
            }

            // auto iterator = std::find_if(predefined_headers.begin(), predefined_headers.end(), isHitAll);
            if (iterator_all != PREDEFINED_HEADERS.end()) {
                /// 对应第 0 种情况
                size_t index = iterator_all - PREDEFINED_HEADERS.begin();
                size += encodeIndexCase0(dest, index);
                continue;
            }

            // iterator = std::find_if(predefined_headers.begin(), predefined_headers.end(), isHit);
            if (iterator_key != PREDEFINED_HEADERS.end()) {
                size_t index = iterator_key - PREDEFINED_HEADERS.begin();
                /// 对应第 1 种情况
                size += encodeIndexCase1(dest, index);
                size += encodeString(dest, item.second);
                /// 添加动态表
                table.set(item.first, item.second);
                continue;
            }
        }

        /// 添加动态表
        {
            size += encodeIndexCase1(dest, 0);
            size += encodeString(dest, item.first);
            size += encodeString(dest, item.second);
            table.set(item.first, item.second);
            continue;
        }
    }

    // 处理一次性 HEADERS
    for (const auto &item: once_header) {
        // 动态表查询
        {
            auto iterator_key = table.end();
            auto iterator_all = table.end();
            for (auto header = table.begin(); header != table.end(); ++header) {
                if (header->first == item.first) {
                    iterator_key = header;
                    if (header->second == item.second) {
                        iterator_all = header;
                        break;
                    }
                }
            }

            // auto iterator = std::find_if(table.begin(), table.end(), isHit);
            if (iterator_all != table.end()) {
                /// 第 0 种情况
                size += encodeIndexCase0(dest, iterator_all - table.begin() + 62);
                continue;
            }

            if (iterator_key != table.end()) {
                /// 第 2 种情况
                size += encodeIndexCase2(dest, iterator_key - table.begin() + 62);
                size += encodeString(dest, item.second);
                continue;
            }
        }

        // 静态表查询
        {
            auto iterator_key = PREDEFINED_HEADERS.end();
            auto iterator_all = PREDEFINED_HEADERS.end();
            for (auto header = PREDEFINED_HEADERS.begin(); header != PREDEFINED_HEADERS.end(); ++header) {
                if (header->first == item.first) {
                    iterator_key = header;
                    if (header->second == item.second) {
                        iterator_all = header;
                        break;
                    }
                }
            }

            // auto iterator = std::find_if(predefined_headers.begin(), predefined_headers.end(), isHit);
            if (iterator_all != PREDEFINED_HEADERS.end()) {
                /// 第 0 种情况
                size += encodeIndexCase0(dest, iterator_all - PREDEFINED_HEADERS.begin());
                continue;
            }
            if (iterator_key != PREDEFINED_HEADERS.end()) {
                /// 第 2 种情况
                size += encodeIndexCase2(dest, iterator_key - PREDEFINED_HEADERS.begin());
                size += encodeString(dest, item.second);
                continue;
            }
        }

        // 无任何索引数据
        {
            size += encodeIndexCase3(dest, 0);
            size += encodeString(dest, item.first);
            size += encodeString(dest, item.second);
            continue;
        }
    }

    /// 此处未对 Cookies 进行压缩
    auto once_cookies = once_header.getCookies();
    auto indexed_cookies = indexed_header.getCookies();
    if (once_cookies) {
        for (const auto &cookie: *once_cookies) {
            auto str = buildCookieString(cookie.second);
            size += encodeIndexCase2(dest, 0);
            size += encodeString(dest, "Set-Cookie");
            size += encodeString(dest, str);
        }
    }
    if (indexed_cookies) {
        for (const auto &cookie: *indexed_cookies) {
            auto str = buildCookieString(cookie.second);
            size += encodeIndexCase2(dest, 0);
            size += encodeString(dest, "Set-Cookie");
            size += encodeString(dest, str);
        }
    }

    return size;
}

int HPackUtil::decodeInteger(uint8_t &buf, InputStream *src, uint32_t &dest, uint8_t n) noexcept {
    const auto TWO_N = static_cast<uint16_t>(std::pow(2, n) - 1);
    dest = buf & TWO_N;
    if (dest == TWO_N) {
        uint64_t m = 0;
        int len = 0;
        while ((src->read(&buf, 1)) > 0) {
            dest += (buf & 0x7F) << m;
            m += 7;
            len += 1;

            if (!(buf & 0x80)) {
                return len;
            }
        }
        return -1;
    } else {
        return 0;
    }
}

int HPackUtil::decodeString(InputStream *src, std::string &dest) noexcept {
    uint8_t buf;
    auto i = src->read(&buf, 1);
    uint8_t len = (buf & 0x7F);
    bool is_huffman = (buf & 0x80) == 0x80;

    char buffer[UINT8_MAX]{};
    if (len != src->read(buffer, len)) {
        return -1;
    }

    if (is_huffman) {
        auto result = decoder.decode(buffer, len);
        if (result != std::nullopt) {
            dest = decoder.decode(buffer, len).value();
        }
    } else {
        dest = buffer;
    }
    return len + 1;
}

size_t HPackUtil::encodeIndexCase0(OutputStream *dest, size_t index) noexcept {
    const auto PREFIX = static_cast<uint8_t>(std::pow(2, 7) - 1);
    uint8_t buf;
    if (index < PREFIX) {
        buf = 0b1000'0000 | (static_cast<uint8_t>(index) & 0b0111'1111);
        dest->write(&buf, 1);
        return 1;
    } else {
        buf = 0b1000'0000 | 0b0111'1111;
        dest->write(&buf, 1);
        size_t size = 1;
        index -= PREFIX;
        while (index >= 128) {
            buf = index % 128 + 128;
            dest->write(&buf, 1);
            index = index / 128;
            size += 1;
        }
        return size;
    }
}

size_t HPackUtil::encodeIndexCase1(OutputStream *dest, size_t index) noexcept {
    const auto PREFIX = static_cast<uint8_t>(std::pow(2, 6) - 1);
    uint8_t buf;
    if (index < PREFIX) {
        buf = 0b0100'0000 | (static_cast<uint8_t>(index) & 0b0011'1111);
        dest->write(&buf, 1);
        return 1;
    } else {
        buf = 0b0100'0000 | 0b0011'1111;
        dest->write(&buf, 1);
        size_t size = 1;
        index -= PREFIX;
        while (index >= 128) {
            buf = index % 128 + 128;
            dest->write(&buf, 1);
            index = index / 128;
            size += 1;
        }
        buf = static_cast<uint8_t>(index);
        dest->write(&buf, 1);
        size += 1;
        return size;
    }
}

size_t HPackUtil::encodeIndexCase2(OutputStream *dest, size_t index) noexcept {
    const auto PREFIX = static_cast<uint8_t>(std::pow(2, 4) - 1);
    uint8_t buf;
    if (index < PREFIX) {
        buf = 0b0000'0000 | (static_cast<uint8_t>(index) & 0b0000'1111);
        dest->write(&buf, 1);
        return 1;
    } else {
        buf = 0b0000'0000 | 0b0000'1111;
        dest->write(&buf, 1);
        size_t size = 1;
        index -= PREFIX;
        while (index >= 128) {
            buf = index % 128 + 128;
            dest->write(&buf, 1);
            index = index / 128;
            size += 1;
        }
        buf = static_cast<uint8_t>(index);
        dest->write(&buf, 1);
        size += 1;
        return size;
    }
}

size_t HPackUtil::encodeIndexCase3(OutputStream *dest, size_t index) noexcept {
    const auto PREFIX = static_cast<uint8_t>(std::pow(2, 4) - 1);
    uint8_t buf;
    if (index < PREFIX) {
        buf = 0b0001'0000 | (static_cast<uint8_t>(index) & 0b0000'1111);
        dest->write(&buf, 1);
        return 1;
    } else {
        buf = 0b0001'0000 | 0b0000'1111;
        dest->write(&buf, 1);
        size_t size = 1;
        index -= PREFIX;
        while (index >= 128) {
            buf = index % 128 + 128;
            dest->write(&buf, 1);
            index = index / 128;
            size += 1;
        }
        buf = static_cast<uint8_t>(index);
        dest->write(&buf, 1);
        size += 1;
        return size;
    }
}

size_t HPackUtil::encodeString(OutputStream *dest, const std::string &str) noexcept {
    const auto PREFIX = static_cast<uint8_t>(std::pow(2, 7) - 1);
    auto str_len = str.length();
    if (str_len > 8) {
        /// 需要 Huffman 压缩
        uint8_t buf;
        auto code = encoder.encode(str);
        if (code.size() < PREFIX) {
            buf = 0b1000'0000 | static_cast<uint8_t>(code.size());
            dest->write(&buf, 1);
            dest->write(code.data(), code.size());
            return 1 + code.size();
        } else {
            buf = 0b1000'0000 | 0b0111'1111;
            dest->write(&buf, 1);
            size_t size = 1;
            size_t i = code.size();
            i -= PREFIX;
            while (i >= 128) {
                buf = i % 128 + 128;
                dest->write(&buf, 1);
                i = i / 128;
                size += 1;
            }
            buf = (uint8_t) i;
            dest->write(&buf, 1);
            size += 1;
            dest->write(code.data(), code.size());
            return size + code.size();
        }
    } else {
        /// 不需要 Huffman 压缩
        uint8_t buf;
        if (str.size() < PREFIX) {
            buf = 0b0000'0000 | static_cast<uint8_t>(str.size());
            dest->write(&buf, 1);
            dest->write(str.data(), str.size());
            return 1 + str.size();
        } else {
            buf = 0b0000'0000 | 0b0111'1111;
            dest->write(&buf, 1);
            size_t size = 1;
            size_t i = str.size();
            i -= PREFIX;
            while (i >= 128) {
                buf = i % 128 + 128;
                dest->write(&buf, 1);
                i = i / 128;
                size += 1;
            }
            buf = static_cast<uint8_t>(i);
            dest->write(&buf, 1);
            size += 1;
            dest->write(str.data(), str.size());
            return size + str.size();
        }
    }
}

std::string HPackUtil::buildCookieString(const Cookie::Ptr &cookie) noexcept {
    text::StringBuilder stream;
    const std::string &name = cookie->getName();
    const std::string &value = cookie->getValue();
    stream << name << "=" << value;

    const std::string &path = cookie->getPath();
    if (!path.empty()) {
        stream << "; " << path;
    }

    const std::string &domain = cookie->getDomain();
    if (!domain.empty()) {
        stream << "; " << domain;
    }

    uint64_t max_age = cookie->getMaxAge();
    if (max_age > 0) {
        stream << "; Max-Age=" << std::to_string(max_age);
    } else {
        uint64_t expires = cookie->getExpires();
        if (expires > 0) {
            auto date = DateTime(expires, 0);
            auto date_string = sese::text::DateTimeFormatter::format(date, TIME_GREENWICH_MEAN_PATTERN);
            stream << "; Expires=" << date_string;
        }
    }

    bool secure = cookie->isSecure();
    if (secure) {
        stream << "; Secure";
    }

    bool http_only = cookie->isHttpOnly();
    if (http_only) {
        stream << "; HttpOnly";
    }

    return stream.toString();
}
