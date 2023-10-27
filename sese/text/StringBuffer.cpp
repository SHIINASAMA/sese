#include "sese/text/StringBuffer.h"
#include "sese/text/AbstractStringBuffer.h"
#include "sese/thread/Locker.h"

// GCOVR_EXCL_START

sese::text::StringBuffer::StringBuffer(size_t cap) noexcept : AbstractStringBuffer(cap) {
}

sese::text::StringBuffer::StringBuffer(const char *str) noexcept : AbstractStringBuffer(str) {
}

void sese::text::StringBuffer::append(char ch) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(ch);
}

void sese::text::StringBuffer::append(const char *str) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(str);
}

void sese::text::StringBuffer::append(const std::string &str) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(str);
}

void sese::text::StringBuffer::append(const std::string_view &str) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(str);
}

void sese::text::StringBuffer::append(const String &str) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(str);
}

void sese::text::StringBuffer::append(const StringView &str) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(str);
}

void sese::text::StringBuffer::append(const char *data, size_t length) noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::append(data, length);
}

void sese::text::StringBuffer::clear() noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::clear();
}

void sese::text::StringBuffer::reverse() noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::reverse();
}

size_t sese::text::StringBuffer::length() noexcept {
    Locker locker(mutex);
    return AbstractStringBuffer::length();
}

size_t sese::text::StringBuffer::size() noexcept {
    Locker locker(mutex);
    return AbstractStringBuffer::size();
}

bool sese::text::StringBuffer::empty() noexcept {
    Locker locker(mutex);
    return AbstractStringBuffer::empty();
}

char sese::text::StringBuffer::getCharAt(int index) {
    Locker locker(mutex);
    return AbstractStringBuffer::getCharAt(index);
}

bool sese::text::StringBuffer::setChatAt(int index, char ch) {
    Locker locker(mutex);
    return AbstractStringBuffer::setChatAt(index, ch);
}

bool sese::text::StringBuffer::delCharAt(int index) {
    Locker locker(mutex);
    return AbstractStringBuffer::delCharAt(index);
}

bool sese::text::StringBuffer::del(int start, int len) {
    Locker locker(mutex);
    return AbstractStringBuffer::del(start, len);
}

bool sese::text::StringBuffer::insertAt(int index, const char *str) {
    Locker locker(mutex);
    return AbstractStringBuffer::insertAt(index, str);
}

bool sese::text::StringBuffer::insertAt(int index, const std::string &str) {
    Locker locker(mutex);
    return AbstractStringBuffer::insertAt(index, str);
}

bool sese::text::StringBuffer::insertAt(int index, const std::string_view &str) {
    Locker locker(mutex);
    return AbstractStringBuffer::insertAt(index, str);
}

bool sese::text::StringBuffer::insertAt(int index, const String &str) {
    Locker locker(mutex);
    return AbstractStringBuffer::insertAt(index, str);
}

bool sese::text::StringBuffer::insertAt(int index, const StringView &str) {
    Locker locker(mutex);
    return AbstractStringBuffer::insertAt(index, str);
}


void sese::text::StringBuffer::trim() noexcept {
    Locker locker(mutex);
    AbstractStringBuffer::trim();
}

std::vector<std::string> sese::text::StringBuffer::split(const std::string &str) noexcept {
    Locker locker(mutex);
    return AbstractStringBuffer::split(str);
}

std::string sese::text::StringBuffer::toString() {
    Locker locker(mutex);
    return AbstractStringBuffer::toString();
}

sese::text::String sese::text::StringBuffer::toSString() {
    Locker locker(mutex);
    return AbstractStringBuffer::toSString();
}

// GCOVR_EXCL_STOP
