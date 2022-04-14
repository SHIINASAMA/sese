#include "FileStream.h"
#include "BaseStreamReader.h"
#include "Util.h"

#define FILTER_TEST_STREAM_READER "fSTREAM_READER"

using sese::FileStream;
using sese::WStreamReader;
using sese::StreamReader;

int main() {
    auto fileStream = std::make_shared<FileStream>(
            PROJECT_PATH "/test/TestStreamReader/data.txt",
            TEXT_READ_EXISTED);
    if (!fileStream->good()) {
        ROOT_INFO(FILTER_TEST_STREAM_READER, "%s", sese::getErrorString().c_str())
        return 0;
    }

    auto reader = std::make_shared<StreamReader>(fileStream);
    while (true) {
        auto line = reader->readLine();
        if (line.empty()) {
            break;
        } else {
            ROOT_INFO(FILTER_TEST_STREAM_READER, "%s", line.c_str())
        }
    }
    return 0;
}