/**
 * @file Base64Converter.h
 * @author kaoru
 * @date 2022年4月18日
 * @brief Base64 转换器
 */
#pragma once
#include "sese/Stream.h"
#include "sese/NotInstantiable.h"

#ifdef _WIN32
#pragma warning(disable : 4624)
#endif

namespace sese {

    /**
     * @brief Base64 转换器
     */
    class API Base64Converter : NotInstantiable {
    public:

        enum class CodePage {
            BASE64,
            BASE62
        };

        static void encode(const Stream::Ptr &src, const Stream::Ptr &dest, CodePage codePage = CodePage::BASE64);

        static void decode(const Stream::Ptr &src, const Stream::Ptr &dest);
    };
}