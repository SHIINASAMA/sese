#include <sese/convert/SHA1Util.h>
#include <sese/util/Endian.h>
#include <sese/io/ByteBuilder.h>

using sese::SHA1Context;
using sese::SHA1Util;
using sese::io::ByteBuilder;

// 声明：
// 此文件实现参考 https://github.com/vog/sha1

static const size_t BLOCK_INTS = 16; /* number of 32bit integers per SHA1 block */
static const size_t BLOCK_BYTES = BLOCK_INTS * 4;

inline static uint32_t rol(const uint32_t value, const size_t bits) {
    return (value << bits) | (value >> (32 - bits));
}

inline static uint32_t blk(const uint32_t block[BLOCK_INTS], const size_t i) {
    return rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i], 1);
}

/*
 * (R0+R1), R2, R3, R4 are the different operations used in SHA1
 */
inline static void R0(const uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i) {
    z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
    w = rol(w, 30);
}

inline static void R1(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i) {
    block[i] = blk(block, i);
    z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5);
    w = rol(w, 30);
}

inline static void R2(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i) {
    block[i] = blk(block, i);
    z += (w ^ x ^ y) + block[i] + 0x6ed9eba1 + rol(v, 5);
    w = rol(w, 30);
}

inline static void R3(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i) {
    block[i] = blk(block, i);
    z += (((w | x) & y) | (w & x)) + block[i] + 0x8f1bbcdc + rol(v, 5);
    w = rol(w, 30);
}

inline static void R4(uint32_t block[BLOCK_INTS], const uint32_t v, uint32_t &w, const uint32_t x, const uint32_t y, uint32_t &z, const size_t i) {
    block[i] = blk(block, i);
    z += (w ^ x ^ y) + block[i] + 0xca62c1d6 + rol(v, 5);
    w = rol(w, 30);
}

inline static void transform(SHA1Context *ctx, uint32_t *block) {
    /* Copy digest[] to working vars */
    uint32_t a = ctx->h[0];
    uint32_t b = ctx->h[1];
    uint32_t c = ctx->h[2];
    uint32_t d = ctx->h[3];
    uint32_t e = ctx->h[4];

    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(block, a, b, c, d, e, 0);
    R0(block, e, a, b, c, d, 1);
    R0(block, d, e, a, b, c, 2);
    R0(block, c, d, e, a, b, 3);
    R0(block, b, c, d, e, a, 4);
    R0(block, a, b, c, d, e, 5);
    R0(block, e, a, b, c, d, 6);
    R0(block, d, e, a, b, c, 7);
    R0(block, c, d, e, a, b, 8);
    R0(block, b, c, d, e, a, 9);
    R0(block, a, b, c, d, e, 10);
    R0(block, e, a, b, c, d, 11);
    R0(block, d, e, a, b, c, 12);
    R0(block, c, d, e, a, b, 13);
    R0(block, b, c, d, e, a, 14);
    R0(block, a, b, c, d, e, 15);
    R1(block, e, a, b, c, d, 0);
    R1(block, d, e, a, b, c, 1);
    R1(block, c, d, e, a, b, 2);
    R1(block, b, c, d, e, a, 3);
    R2(block, a, b, c, d, e, 4);
    R2(block, e, a, b, c, d, 5);
    R2(block, d, e, a, b, c, 6);
    R2(block, c, d, e, a, b, 7);
    R2(block, b, c, d, e, a, 8);
    R2(block, a, b, c, d, e, 9);
    R2(block, e, a, b, c, d, 10);
    R2(block, d, e, a, b, c, 11);
    R2(block, c, d, e, a, b, 12);
    R2(block, b, c, d, e, a, 13);
    R2(block, a, b, c, d, e, 14);
    R2(block, e, a, b, c, d, 15);
    R2(block, d, e, a, b, c, 0);
    R2(block, c, d, e, a, b, 1);
    R2(block, b, c, d, e, a, 2);
    R2(block, a, b, c, d, e, 3);
    R2(block, e, a, b, c, d, 4);
    R2(block, d, e, a, b, c, 5);
    R2(block, c, d, e, a, b, 6);
    R2(block, b, c, d, e, a, 7);
    R3(block, a, b, c, d, e, 8);
    R3(block, e, a, b, c, d, 9);
    R3(block, d, e, a, b, c, 10);
    R3(block, c, d, e, a, b, 11);
    R3(block, b, c, d, e, a, 12);
    R3(block, a, b, c, d, e, 13);
    R3(block, e, a, b, c, d, 14);
    R3(block, d, e, a, b, c, 15);
    R3(block, c, d, e, a, b, 0);
    R3(block, b, c, d, e, a, 1);
    R3(block, a, b, c, d, e, 2);
    R3(block, e, a, b, c, d, 3);
    R3(block, d, e, a, b, c, 4);
    R3(block, c, d, e, a, b, 5);
    R3(block, b, c, d, e, a, 6);
    R3(block, a, b, c, d, e, 7);
    R3(block, e, a, b, c, d, 8);
    R3(block, d, e, a, b, c, 9);
    R3(block, c, d, e, a, b, 10);
    R3(block, b, c, d, e, a, 11);
    R4(block, a, b, c, d, e, 12);
    R4(block, e, a, b, c, d, 13);
    R4(block, d, e, a, b, c, 14);
    R4(block, c, d, e, a, b, 15);
    R4(block, b, c, d, e, a, 0);
    R4(block, a, b, c, d, e, 1);
    R4(block, e, a, b, c, d, 2);
    R4(block, d, e, a, b, c, 3);
    R4(block, c, d, e, a, b, 4);
    R4(block, b, c, d, e, a, 5);
    R4(block, a, b, c, d, e, 6);
    R4(block, e, a, b, c, d, 7);
    R4(block, d, e, a, b, c, 8);
    R4(block, c, d, e, a, b, 9);
    R4(block, b, c, d, e, a, 10);
    R4(block, a, b, c, d, e, 11);
    R4(block, e, a, b, c, d, 12);
    R4(block, d, e, a, b, c, 13);
    R4(block, c, d, e, a, b, 14);
    R4(block, b, c, d, e, a, 15);

    /* Add the working vars back into digest[] */
    ctx->h[0] += a;
    ctx->h[1] += b;
    ctx->h[2] += c;
    ctx->h[3] += d;
    ctx->h[4] += e;
}

inline static void buffer_to_block(const uint8_t *buffer, uint32_t block[BLOCK_INTS]) {
    /* Convert the std::string (byte buffer) to an uint32_t array (MSB) */
    for (size_t i = 0; i < BLOCK_INTS; i++) {
        block[i] = (buffer[4 * i + 3] & 0xff) | (buffer[4 * i + 2] & 0xff) << 8 | (buffer[4 * i + 1] & 0xff) << 16 | (buffer[4 * i + 0] & 0xff) << 24;
    }
}

// GCOVR_EXCL_START
void SHA1Util::encode(const InputStream::Ptr &input, const OutputStream::Ptr &output) noexcept {
    encode(input.get(), output.get());
}
// GCOVR_EXCL_STOP

void SHA1Util::encode(InputStream *input, OutputStream *output) noexcept {
    SHA1Context ctx;
    uint8_t buffer[64];
    uint32_t block[64];

    int64_t size;
    while ((size = input->read(buffer, 64)) == 64) {
        ctx.total += size;
        buffer_to_block(buffer, block);
        transform(&ctx, block);
    }

    // 处理尾部
    if (size > 55) {
        ctx.total += size;
        buffer[size] = 0x80;
        memset(buffer + size + 1, 0, 63 - size);

        buffer_to_block(buffer, block);
        transform(&ctx, block);

        // 最后填充一次
        uint64_t total = ToBigEndian64(ctx.total * 8);
        memset(buffer, 0, 56);
        memcpy(buffer + 56, &total, 8);

        buffer_to_block(buffer, block);
        transform(&ctx, block);
    } else {
        ctx.total += size;
        uint64_t total = ToBigEndian64(ctx.total * 8);
        buffer[size] = 0x80;
        memset(buffer + size + 1, 0, 55 - size);
        memcpy(buffer + 56, &total, 8);

        buffer_to_block(buffer, block);
        transform(&ctx, block);
    }

    for (unsigned int &i: ctx.h) {
        i = FromBigEndian32(i);
    }
    output->write(ctx.h, sizeof(ctx.h));
}

// GCOVR_EXCL_START
inline char toChar(unsigned char ch, bool isCap) {
    if (ch >= 0 && ch <= 9) {
        return (char) (ch + 48);
    } else {
        if (isCap) {
            return (char) (ch + 55);
        } else {
            return (char) (ch + 87);
        }
    }
}
// GCOVR_EXCL_STOP

std::unique_ptr<char[]> SHA1Util::encode(const InputStream::Ptr &input, bool isCap) noexcept {
    return encode(input.get(), isCap);
}

std::unique_ptr<char[]> SHA1Util::encode(InputStream *input, bool isCap) noexcept {
    ByteBuilder dest(64);
    encode(input, &dest);
    unsigned char buffer[40];
    auto rt = std::unique_ptr<char[]>(new char[41]);
    dest.read(buffer, 20);
    for (auto i = 0; i < 20; ++i) {
        rt[i * 2 + 1] = toChar(buffer[i] % 0x10, isCap);
        rt[i * 2 + 0] = toChar(buffer[i] / 0x10, isCap);
    }
    rt[40] = 0;
    return rt;
}