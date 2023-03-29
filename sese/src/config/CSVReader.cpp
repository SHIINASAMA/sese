#include "sese/config/CSVReader.h"

#include <sstream>

sese::CSVReader::CSVReader(sese::InputStream *source, char splitChar) noexcept {
    CSVReader::source = source;
    CSVReader::splitChar = splitChar;
}

sese::CSVReader::Row sese::CSVReader::read() noexcept {
    Row row;
    std::stringstream builder;

    char ch;
    size_t quot = 0;
    while (source->read(&ch, 1) != 0) {
        // 是否处于字符串内部
        if (quot % 2 == 0) {
            if (ch != '\"') {
                builder << ch;
            } else {
                // 退出字符串
                quot += 1;
            }
        } else {
            if (ch == '\"') {
                // 进入字符串
                quot += 1;
            } else if (ch == CSVReader::splitChar) {
                // 切割元素
                row.emplace_back(builder.str());
                builder.clear();
            } else if (ch == '\r') {
                // 换行 - \r\n
                if (source->read(&ch, 1) != 0) {
                    return row;
                }
            } else if (ch == '\n') {
                // 换行 - \n
                return row;
            } else {
                builder << ch;
            }
        }
    }
    if (builder.rdbuf()->in_avail() != 0) {
        row.emplace_back(builder.str());
        builder.clear();
    }
    return row;
}