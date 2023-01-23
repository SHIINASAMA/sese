/**
 * @file PercentConverter.h
 * @author kaoru
 * @date 2022年4月18日
 * @brief 百分号编码转换器
 */
#pragma once
#include "sese/util/OutputStream.h"
#include "sese/util/NotInstantiable.h"

#ifdef _WIN32
#pragma warning(disable : 4624)
#endif

namespace sese {

    /**
     * @brief 百分号编码转换器
     */
    class API PercentConverter : public NotInstantiable{
    public:

        static void encode(const char *src, const OutputStream::Ptr &dest);

        static void decode(const char *src, const OutputStream::Ptr &dest);
    };
}