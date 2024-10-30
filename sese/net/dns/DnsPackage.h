/// @file DnsPackage.h
/// @brief DNS 包对象
/// @author kaoru
/// @date 2024年10月30日

// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace sese::net::dns {

/// \brief DNS 包对象
class DnsPackage {
public:
    /// \brief DNS Flags 工具
    struct Flags {
        bool qr = false;
        uint8_t opcode = 0;
        bool aa = false;
        bool tc = false;
        bool rd = false;
        bool ra = false;
        uint8_t z = false;
        uint8_t rcode = false;

        [[nodiscard]] uint16_t encode() const;

        void decode(uint16_t flags);
    };

    class Index;

    /// \brief DNS 请求
    struct Question {
        std::string name;
        uint16_t type;
        uint16_t class_;
    };

    /// \brief DNS 应答
    struct Answer {
        std::string name;
        uint16_t type;
        uint16_t class_;
        uint32_t ttl;
        uint16_t data_length;
        std::unique_ptr<uint8_t[]> data{};
    };

    /// \brief DNS 权威应答
    using Authority = Answer;
    /// \brief DNS 附加应答
    using Additional = Answer;

private:
    struct DnsHeader {
        uint16_t id;
        uint16_t flags;
    } header{};

    std::vector<Question> questions;
    std::vector<Answer> answers;
    std::vector<Authority> authorities;
    std::vector<Additional> additionals;

    DnsPackage() = default;

    /// 解码 DNS 名称
    /// @warning 注意此函数会进行递归调用解码多级指针
    /// @param buffer 完整 buffer
    /// @param length buffer 大小
    /// @param offset 需要解码的偏移位置，解码完成后停留在 00 位置
    /// @return 解码后字符串
    static std::string decodeWords(const uint8_t *buffer, size_t length, size_t &offset); // NOLINT

    static bool decodeAnswers(std::vector<Answer> &answers, size_t expect_size, const uint8_t *buffer, size_t length, size_t &pos);

    static std::string encodeWords(const std::string &fullname);

    static bool encodeQuestions(const std::vector<Question> &questions, void *buffer, size_t &length, Index &index, size_t offset);

    static bool encodeAnswers(const std::vector<Answer> &answers, void *buffer, size_t &length, Index &index, size_t offset);

public:
    /// \brief DNS 打包压缩索引
    class Index {
        friend class DnsPackage;
        using CompressMapping = std::set<uint16_t>;
        struct CompressIndex {
            uint16_t index;
            std::string name;
            uint16_t pos;
        };

        std::vector<CompressIndex> compress_index;
        std::map<std::string, CompressMapping> compress_mapping;

        Index(
            std::vector<Question> &questions,
            std::vector<Answer> &answers,
            std::vector<Authority> &authorities,
            std::vector<Additional> &additionals
        );

        [[nodiscard]] std::vector<CompressIndex *> getSortedIndexes(const std::set<uint16_t> &indexes);

        static void updatePos(std::vector<CompressIndex *> &indexes, uint16_t offset);

        void clearPos() const;

        std::string encodeWords(const std::string &fullname, const std::set<uint16_t> &indexes, uint16_t base_offset);

    public:
        Index() = default;
    };

    using Ptr = std::shared_ptr<DnsPackage>;

    /// @return 空 DNS 打包压缩索引
    static Ptr new_();

    /// 解析 DNS 原始包
    /// @param buffer DNS 包缓存
    /// @param length 缓存大小
    /// @return 成功返回对象，反之返回nullptr
    static Ptr decode(const uint8_t *buffer, size_t length);

    /// 获取 DNS 打包压缩索引
    /// @return 索引
    Index buildIndex();

    /// 将 DNS 包编码至缓存
    /// @param buffer 输出缓存
    /// @param length 输出缓存大小，执行成功将返回实际所用大小；若输入0则计算所需缓存大小，不执行实际打包操作
    /// @param index DNS 打包压缩索引
    /// @return 是否执行成功
    bool encode(void *buffer, size_t &length, Index &index) const;

    [[nodiscard]] std::vector<Question> &getQuestions() { return questions; }
    [[nodiscard]] std::vector<Answer> &getAnswers() { return answers; }
    [[nodiscard]] std::vector<Authority> &getAuthorities() { return authorities; }
    [[nodiscard]] std::vector<Additional> &getAdditionals() { return additionals; }
    [[nodiscard]] auto getId() const { return header.id; }
    [[nodiscard]] auto getFlags() const { return header.flags; }

    void setId(uint16_t id) { header.id = id; }
    void setFlags(uint16_t flags) { header.flags = flags; }
};

} // namespace sese::net::dns