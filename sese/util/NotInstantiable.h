/**
 * @file NotInstantiable.h
 * @brief 不可实例化类
 * @author kaoru
 * @date 2022年3月28日
 */
#pragma once
#include "sese/Config.h"

namespace sese {

/**
 * @brief 不可实例化类
 */
class API NotInstantiable {
public:
    NotInstantiable() = delete;
    virtual ~NotInstantiable() = default;
    NotInstantiable(const NotInstantiable &) = delete;
    NotInstantiable &operator=(const NotInstantiable &) = delete;
};
} // namespace sese