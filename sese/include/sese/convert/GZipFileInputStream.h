/// \file GZipFileInputStream.h
/// \author kaoru
/// \brief GZip 文件输入流（解压）
/// \version 0.1
/// \date 2023年3月15日

#pragma once

#include "sese/util/InputStream.h"

namespace sese {

    /// GZip 文件输入流（解压）
    class API GZipFileInputStream : public InputStream {
    public:
        using Ptr = std::unique_ptr<GZipFileInputStream>;

        /// 创建文件输入流对象
        /// \param file 文件的相对或者绝对路径
        /// \retval nullptr 打开文件失败
        static GZipFileInputStream::Ptr create(const char *file) noexcept;

        int64_t read(void *buffer, size_t length) override;

        void close() noexcept;

    private:
        GZipFileInputStream() = default;

        void *gzFile = nullptr;
    };

}// namespace sese