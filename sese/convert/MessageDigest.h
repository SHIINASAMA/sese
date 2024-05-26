/**
 * \file MessageDigest.h
 * \brief 信息摘要工具类
 * \author kaoru
 * \version 0.1
 * \date 2022.12.26
 */
#pragma once

#include "sese/io/InputStream.h"
#include "sese/util/NotInstantiable.h"

namespace sese {
/// 信息摘要工具类
class API MessageDigest final : public NotInstantiable {
public:
    using InputStream = sese::io::InputStream;

    enum Type {
        MD5,
        SHA1,
        SHA256
    };

    MessageDigest() = delete;

    /// 进行摘要
    /// \param type 摘要算法类型
    /// \param source 信息来源
    /// \param is_cap 字母是否大写
    /// \retval nullptr 摘要失败
    /// \return 摘要结果
    std::unique_ptr<char[]> static digest(Type type, const InputStream::Ptr &source, bool is_cap = false) noexcept;
    std::unique_ptr<char[]> static digest(Type type, InputStream *source, bool is_cap = false) noexcept;
};
} // namespace sese