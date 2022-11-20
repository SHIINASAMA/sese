/**
* @file XmlUtil.h
* @brief Xml 序列化工具类
* @author kaoru
* @date 2022年10月23日
*/
#pragma once
#include <sese/config/xml/XmlTypes.h>
#include <sese/Stream.h>
#include <sese/NotInstantiable.h>
#include <queue>

namespace sese::xml {

    /// Xml 序列化工具类
    class API XmlUtil : public NotInstantiable {
    public:
        using Tokens = std::queue<std::string>;

        /// 从流中反序列化一个 Xml 元素对象
        /// \param inputStream 输入流
        /// \param level 反序列化深度
        /// \retval nullptr 反序列化失败，否则为成功
        static Element::Ptr deserialize(const InputStream::Ptr &inputStream, size_t level) noexcept;

        /// 向流中序列化一个 Xml 元素对象
        /// \param object 待序列化对象
        /// \param outputStream 待输出流
        static void serialize(const Element::Ptr &object, const OutputStream::Ptr &outputStream) noexcept;

    private:
        static void tokenizer(const InputStream::Ptr &inputStream, Tokens &tokens) noexcept;

        static void removeComment(Tokens &tokens) noexcept;

        static Element::Ptr createElement(Tokens &tokens, size_t level, bool isSubElement) noexcept;

    };
}// namespace sese::xml