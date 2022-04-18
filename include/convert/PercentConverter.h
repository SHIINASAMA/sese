/**
 * @file PercentConverter.h
 * @author kaoru
 * @date 2022年4月18日
 * @brief 百分号编码转换器
 */
#pragma once
#include "Stream.h"

namespace sese {

    /**
     * @brief 百分号编码转换器
     */
    class API PercentConverter {
    public:

        static void encode(const char *src, const Stream::Ptr &dest);

        static void decode(const char *src, const Stream::Ptr &dest);
    };
}